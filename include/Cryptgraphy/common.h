#pragma once
#include <algorithm>
#include <exception>
#include <cstdint>
#include <bitset>
#include <iterator>
#include <array>
#include <vector>
#include <cmath>
#include <string>
#include <memory>
#include <charconv>
#include <deque>
#include <stdfloat>
#include <bit>
#include <span>
#include <utility>
#include <random>

#include "NumberSet.h"

namespace Cryptgraphy {

	using byte_t = uint8_t;
	using bytearray = std::vector<byte_t>;
	template<size_t size>
	using cbytearray = std::array<byte_t, size>;

	using byte_view = std::span<const byte_t>;
	using byte_ref = std::span<byte_t>;

}