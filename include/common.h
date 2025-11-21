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

namespace SocketUtil {

	template<class T>
	concept enum32_t = std::is_enum_v<T> && (sizeof(T) == sizeof(uint32_t));

	using byte_t = uint8_t;
	using byte_view = std::span<const byte_t>;
	using byte_ref = std::span<byte_t>;

	using bytearray = std::vector<byte_t>;

	
}
