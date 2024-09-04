#pragma once

template <typename Until, auto value>
constexpr decltype(value) defer = value;
