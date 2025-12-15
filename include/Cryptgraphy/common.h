#pragma once
#include <algorithm>
#include <array>
#include <bit>
#include <bitset>
#include <cassert>
#include <charconv>
#include <cmath>
#include <cstdint>
#include <deque>
#include <exception>
#include <iterator>
#include <memory>
#include <random>
#include <span>
#include <string>
#include <utility>
#include <vector>
#include <unordered_map>

#include "NumberSet.h"

namespace Cryptgraphy {

	using byte_t = uint8_t;
	using bytearray = std::vector<byte_t>;
	template<size_t size>
	using cbytearray = std::array<byte_t, size>;

	using byte_view = std::span<const byte_t>;
	using byte_ref = std::span<byte_t>;

}
