#pragma once

#include <ranges>

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

	void shift() { ++m_begin; }

	template<typename T>
	T copyInto() {
		T result;
		for (const auto& item : *this) {
			result.push_back(item);
		}
		return result;
	}
	
private:
	iterator_type m_begin;
	sentinel_type m_end;
};