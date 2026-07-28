#pragma once

// value is only created when the type of Until is known
template <typename Until, auto value>
constexpr decltype(value) defer = value;
