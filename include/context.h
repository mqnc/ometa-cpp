#pragma once

#include <unordered_map>
#include <stack>
#include <vector>
#include <utility>
#include <type_traits>
#include <tuple>
#include "tag.h"
#include "empty.h"

// This macro requires that Context member types have an overloaded assignment
// operator that can distinguish between regular assignment and backup
// restoration so we can use std::tie instead of complicatedly unfolding a, b, c
// into
// a.restore(get<0>(backup));
// b.restore(get<1>(backup)); ...

// we can't use auto or decltype(this->backup()) in method parameters of a local
// class so we have to repeat the lambda
#define OMETA_MAP_BACKUP_METHOD \
[](const auto&... fields) { \
	return std::make_tuple(fields.backup()...); \
}

#define OMETA_BACKTRACKING_FIELDS(...) \
auto backup() const { \
	return OMETA_MAP_BACKUP_METHOD(__VA_ARGS__); \
} \
void restore(const decltype(OMETA_MAP_BACKUP_METHOD(__VA_ARGS__))& backup) { \
	std::tie(__VA_ARGS__) = backup; \
}

namespace ometa {

// wrapper so the assignment operator can distinguish between assign and restore
template <typename T>
struct ContextBackup{
	T value;
};

// PersistentContextValue does not backtrack.
// Use for invocation counting etc.
template <typename T>
class PersistentContextValue {
	T value {};
public:

	PersistentContextValue() = default;

	PersistentContextValue(const T& value): value(value) {}

	PersistentContextValue& operator=(const T& newValue) {
		value = newValue;
		return *this;
	}
	const T& operator*() const {
		return value;
	}

	ContextBackup<Empty> backup() const { return ContextBackup{empty}; }
	
	PersistentContextValue& operator=(ContextBackup<Empty>) {}
};

// Simple ContextValue that copies itself as backup, use for small types.
template <typename T>
class ContextValue {
	T value {};
public:

	ContextValue() = default;

	ContextValue(const T& value): value(value) {}

	// works both for assignment and for restore in this case
	ContextValue& operator=(const T& newValue) {
		value = newValue;
		return *this;
	}

	const T& operator*() const {
		return value;
	}

	T backup() const { return value; }

	void restore(T targetVersion) { value = targetVersion; }
};

// VersionedContextValue carries its history around, backup value is a version
// number. Use for large types.
template <typename T>
class VersionedContextValue {
	size_t version = 0;
	std::stack<T, std::vector<T>> value {};
public:

	VersionedContextValue() = default;

	VersionedContextValue(const T& value): value({value}) {}

	VersionedContextValue& operator=(const T& newValue) {
		value.push(newValue);
		version++;
		return *this;
	}
	const T& operator*() const {
		return value.top();
	}

	ContextBackup<size_t> backup() const { return {version}; }

	VersionedContextValue& operator=(ContextBackup<size_t> targetVersion) {
		while (version > targetVersion.value) {
			value.pop();
			version--;
		}
		return *this;
	}
};

// ContextTable is a map key->VersionedContextValue that keeps track of
// insertion operations across the whole map, backup is also a version number.
// Use for symbol tables. Entries can not be erased (apart from restoring).
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

	ContextTable& operator=(size_t targetVersion) {
		while (backup() > targetVersion) {
			const auto& key = order.top();
			entries[key].pop();
			if (entries[key].size() == 0) {
				entries.erase(key);
			}
			order.pop();
		}
		return *this;
	}

	const size_t size() const { return entries.size(); }

	class iterator {
		using MapIterator = typename std::unordered_map<K, std::stack<V, std::vector<V>>>::iterator;
		MapIterator current;
	public:
		iterator(MapIterator iter): current(iter) {}
		std::pair<const K&, const V&> operator*() const {return {current->first, current->second.top()};}
		iterator& operator++() { ++current; return *this;}
		bool operator==(const iterator& other) const { return current == other.current; }
		bool operator!=(const iterator& other) const { return current != other.current; }
	};

	iterator begin() {
		return iterator(entries.begin());
	}

	iterator end() {
		return iterator(entries.end());
	}

};

}
