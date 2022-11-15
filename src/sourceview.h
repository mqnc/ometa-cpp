#pragma once

#include <ranges>
#include <string_view>
#include <algorithm>

using std::ranges::forward_range;

template <forward_range TSource>
class SourceView : public std::ranges::view_interface<SourceView<TSource>> {
public:
	using source_type = TSource;
	using iterator_type = decltype(std::ranges::cbegin(std::declval<TSource&>()));
	using sentinel_type = decltype(std::ranges::cend(std::declval<TSource&>()));

	SourceView() = default;

	SourceView(const TSource& src)
		:m_begin(std::ranges::cbegin(src)), m_end(std::ranges::cend(src)) {}

	SourceView(iterator_type begin, sentinel_type end)
		:m_begin(begin), m_end(end) {}

	auto begin() const { return m_begin; }

	auto end() const { return m_end; }

	auto next() { return SourceView(++m_begin, m_end); }

	template<typename T>
	T copyInto() const {
		T result;
		for (const auto& item : *this) {
			result.push_back(item);
		}
		return result;
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

SourceView(const char*) -> SourceView<std::string_view>;
