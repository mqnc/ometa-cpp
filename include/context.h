#pragma once

#include <unordered_map>
#include <stack>
#include <vector>
#include <utility>
#include <type_traits>
#include "taggedtuple.h"

namespace ometa {

template <typename T>
class ContextValue {
	T value;
public:
	void operator=(const T& newValue) { value = newValue; }
	const T& operator*() const { return value; }
	size_t getVersion() const { return value; }
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
	size_t getVersion() const { return version; }
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
		const V& bucket = entries.at(key);
		return bucket.back();
	}

	size_t getVersion() const { return order.size(); }

	void backtrack(size_t targetVersion) {
		while (getVersion() > targetVersion) {
			const auto& key = order.pop();
			entries[key].pop();
			if (entries[key].size() == 0) {
				entries.erase(key);
			}
		}
	}

};

template <typename... TaggedValues>
class Context {
	std::tuple<TaggedValues...> data;

    template <size_t Index, Tag T, typename TupleElement>
    auto& getTaggedHelper(TupleElement& elem) {
        if constexpr (T == TupleElement::getTag()) {
            return elem.value;
        } else {
            return getTaggedHelper<Index + 1, T>(std::get<Index + 1>(data));
        }
    }

	template <size_t Index = 0, typename Tuple, typename VersionTuple>
	void getVersionHelper(const Tuple& data, VersionTuple& versions) const {
		if constexpr (Index < std::tuple_size_v<Tuple>) {
			std::get<Index>(versions) = std::get<Index>(data)->getVersion();
			getVersionHelper<Index + 1>(data, versions);
		}
	}

	template <size_t Index = 0, typename Tuple, typename VersionTuple>
	void backtrackHelper(Tuple& data, const VersionTuple& versions) {
		if constexpr (Index < std::tuple_size_v<Tuple>) {
			std::get<Index>(data).backtrack(std::get<Index>(versions));
			backtrackHelper<Index + 1>(data, versions);
		}
	}

public:
	Context(TaggedValues... values): data(values...) {}

	using VersionType = std::tuple<decltype(std::declval<TaggedValues>()->getVersion())...>;

    template <Tag T>
    auto& get() {
        return getTaggedHelper<0, T>(std::get<0>(data));
    }

	auto getVersion() const {
		VersionType versions;
		getVersionHelper(data, versions);
		return versions;
	}

	void backtrack(const VersionType& versions) {
		backtrackHelper(data, versions);
	}
};

}
