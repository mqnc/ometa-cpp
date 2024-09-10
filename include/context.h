#pragma once

#include <unordered_map>
#include <stack>
#include <vector>
#include <utility>
#include <type_traits>
#include <tuple>
#include "tag.h"
#include "empty.h"

namespace ometa {

template <typename T>
class UnsafeContextValue {
	T value;
public:
	void operator=(const T& newValue) { value = newValue; }
	const T& operator*() const { return value; }
	Empty backup() const {}
	void backtrack(Empty) {}
};

template <typename T>
class ContextValue {
	T value;
public:
	void operator=(const T& newValue) { value = newValue; }
	const T& operator*() const { return value; }
	size_t backup() const { return value; }
	void backtrack(T targetVersion) { value = targetVersion; }
};

template <typename T>
class LoggingContextValue {
	size_t version = 0;
	std::stack<T, std::vector<T>> value;
public:
	void operator=(const T& newValue) {
		value.push(newValue);
		version++;
	}
	const T& operator*() const { return value.top(); }
	size_t backup() const { return version; }
	void backtrack(size_t targetVersion) {
		while (version > targetVersion) {
			value.pop();
			version--;
		}
	}
};

template <typename K, typename V>
class ContextTable {
	std::unordered_map<K, std::stack<V, std::vector<V>>> entries;
	std::stack<K> order;

public:

	void insert(const std::pair<K, V>& entry) {
		const auto& [key, value] = entry;
		auto [bucket, _] = entries.insert({key, {}}); // does not overwrite an existing entry
		bucket->second.push(value);
		order.push(key);
	}

	const V& at(const K& key) const {
		const auto& bucket = entries.at(key);
		return bucket.top();
	}

	size_t backup() const { return order.size(); }

	void backtrack(size_t targetVersion) {
		while (backup() > targetVersion) {
			const auto& key = order.top();
			order.pop();
			entries[key].pop();
			if (entries[key].size() == 0) {
				entries.erase(key);
			}
		}
	}

};

template <typename... Members>
class Context;

namespace detail{

template <typename F, typename... Members>
constexpr decltype(auto) apply(
	F f, Context<Members...>& ctx
) {
	return std::make_tuple(
		f(static_cast<Members&>(ctx))...
	);
}

template <typename F, typename... Members, typename... Ts, std::size_t... Is>
constexpr void invoke_impl(
    F f, Context<Members...>& ctx, std::tuple<Ts...>& args, std::index_sequence<Is...>
) {
    (f(static_cast<Members&>(ctx), std::get<Is>(args)), ...);
}

template <typename F, typename... Members, typename... Ts>
constexpr void invoke(
    F f, Context<Members...>& ctx, std::tuple<Ts...>& args
) {
    invoke_impl(f, ctx, args, std::index_sequence_for<Ts...>{});
}

auto backup(auto& ctx) {
	return apply(
		[](const auto& field) {
			return field->backup();
		},
		ctx
	);
}

void backtrack(auto& ctx, auto& version) {
	invoke(
		[](auto& field, auto targetVersion) {
			field->backtrack(targetVersion);
		}, 
		ctx,
		version
	);
}

}

template <typename... Members>
class Context: public Members... {
public:
	Context(Members... members): Members(members)... {}

	auto backup() {
		return detail::backup(*this);
	}

	auto backtrack(auto& version) {
		detail::backtrack(*this, version);
	}
};

template <Tag tag, typename T>
decltype(auto) get(Tagged<tag, T>& m) {
	return (m.value);
}

}
