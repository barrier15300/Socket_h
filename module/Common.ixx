export module Socket.Common;

export import std;
export import std.compat;

export {
	template<class T>
	concept enum32_t = std::is_enum_v<T> && (sizeof(T) == sizeof(uint32_t));

	using byte_t = std::uint8_t;
	using byte_view = std::span<const byte_t>;
	using byte_ref = std::span<byte_t>;

	using bytearray = std::vector<byte_t>;
}