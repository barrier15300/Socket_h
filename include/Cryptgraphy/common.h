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
#include <execution>
#include <iterator>
#include <memory>
#include <random>
#include <ranges>
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

template<class From>
	requires
		std::is_trivially_copyable_v<From> &&
		(!std::ranges::input_range<From>)
static constexpr Cryptgraphy::bytearray constexpr_bytes_cast(const From& v) {
	constexpr size_t bytes = sizeof(From);
	using arr_t = Cryptgraphy::cbytearray<bytes>;

	arr_t temp{};
	temp = std::bit_cast<arr_t>(v);
	
	Cryptgraphy::bytearray ret(bytes);

	for (size_t i = 0; i < bytes; ++i) {
		ret[i] = temp[i];
	}
	
	return ret;
}

template<std::ranges::range From>
	requires
		std::is_trivially_copyable_v<std::ranges::range_value_t<From>>
static constexpr Cryptgraphy::bytearray constexpr_bytes_cast(const From& R) {
	constexpr size_t bytes = sizeof(std::ranges::range_value_t<From>);

	size_t len = std::ranges::size(R);

	Cryptgraphy::bytearray ret;
	ret.reserve(bytes * len);

	for (size_t i = 0; i < len; ++i) {
		auto&& temp = constexpr_bytes_cast(R[i]);
		ret.insert(ret.end(), temp.begin(), temp.end());
	}

	return ret;
}

template<class T, std::ranges::range From>
	requires
		std::is_trivially_constructible_v<T> &&
		std::is_trivially_copyable_v<T> &&
		std::is_same_v<std::ranges::range_value_t<From>, Cryptgraphy::byte_t>
static constexpr T constexpr_bytes_cast(From&& byteR) {
	constexpr size_t bytes = sizeof(T);
	using arr_t = Cryptgraphy::cbytearray<bytes>;

	T ret{};
	arr_t temp{};

	size_t len = std::min(temp.size(), std::ranges::size(byteR));

	for (size_t i = 0; i < len; ++i) {
		temp[i] = byteR[i];
	}

	ret = std::bit_cast<T>(temp);

	return ret;
}