#pragma once

#include <array>
#include <cstdint>
#include <cstring>
#include <exception>
#include <filesystem>
#include <fstream>
#include <future>
#include <iostream>
#include <memory>
#include <optional>
#include <span>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

namespace SocketDetail {

	using byte_t = uint8_t;
	using byte_view = std::span<const byte_t>;
	using byte_ref = std::span<byte_t>;

	using bytearray = std::vector<byte_t>;
	template<size_t S>
	using cbytearray = std::array<byte_t, S>;

	template<class T>
	concept enum32 = std::is_enum_v<T> && (sizeof(T) == sizeof(uint32_t));

	template<class T>
	concept memcpyable = std::is_trivially_copyable_v<T>;

	template<class T>
	concept to_byteable = requires(const T& x) {
		{ x.ToBytes() } -> std::convertible_to<bytearray>;
	};

	template<class T>
	concept from_byteable = requires(T& x) {
		{ x.FromBytes(byte_view()) } -> std::convertible_to<byte_view>;
	};

	template<class T>
	concept cross_convertible = to_byteable<T> && from_byteable<T>;
}
