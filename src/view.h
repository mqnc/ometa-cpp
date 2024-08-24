#pragma once

#include <ranges>
#include <string_view>
#include <algorithm>
#include <ostream>

namespace ometa {

using std::ranges::forward_range;

template <forward_range TData>
class View: public std::ranges::view_interface<View<TData>> {
public:
	using data_type = TData;
	using iterator_type = decltype(std::ranges::cbegin(std::declval<TData&>()));
	using sentinel_type = decltype(std::ranges::cend(std::declval<TData&>()));

	View() = default;

	View(const TData& src)
		: m_begin(std::ranges::cbegin(src)), m_end(std::ranges::cend(src)) {}

	View(iterator_type begin, sentinel_type end)
		: m_begin(begin), m_end(end) {}

	auto begin() const { return m_begin; }

	auto end() const { return m_end; }

	auto next() { return View(++m_begin, m_end); }

	template <typename T>
	void copyInto(T& buffer) const {
		for (const auto& item: *this) {
			buffer.push_back(item);
		}
	}

	template <typename T>
	T copyAs() const {
		T buffer;
		copyInto(buffer);
		return buffer;
	}

	bool operator==(const auto& other) const {
		return std::ranges::equal(*this, other);
	}

	bool operator==(const char* other) const {
		return operator==(std::string_view(other));
	}

private:
	iterator_type m_begin;
	sentinel_type m_end;
};

View(const char*)->View<std::string_view>;
View(const std::string&)->View<std::string_view>;

template <forward_range TData>
std::ostream& operator<<(std::ostream& os, const View<TData> src)
{
	for (const auto& item: src) {
		os << item;
	}
	return os;
}

}