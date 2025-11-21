#pragma once
#include "common.h"

/// <summary>
/// Header in Packet
/// </summary>

struct Header {

	Header() {}
	template<SocketUtil::enum32_t T>
	explicit Header(T _enum) : Header(static_cast<uint32_t>(_enum)) {}
	explicit Header(uint32_t datatype) {
		Type = datatype;
	}

	template<SocketUtil::enum32_t T>
	T TypeAs() const {
		return static_cast<T>(Type);
	}

	template<SocketUtil::enum32_t T>
	bool Is(T type) const {
		return static_cast<T>(Type) == type;
	}

	template<class T>
	bool IsSameAs() const {
		return Type == type_hash_code<T>();
	}

	template <typename T>
	static constexpr std::string_view type_name() {
#if defined(__clang__) || defined(__GNUC__)
		constexpr std::string_view func = __PRETTY_FUNCTION__;
		constexpr std::string_view prefix = "T = ";
		constexpr std::string_view suffix = "]";
#elif defined(_MSC_VER)
		constexpr std::string_view func = __FUNCSIG__;
		constexpr std::string_view prefix = "type_name<";
		constexpr std::string_view suffix = ">(void)";
#else
#   error Unsupported compiler
#endif
		const auto start = func.find(prefix) + prefix.size();
		const auto end = func.rfind(suffix);
		return func.substr(start, end - start);
	}

	template <typename T>
	static constexpr std::uint32_t type_hash_code() {
		constexpr auto name = type_name<T>();
		// Fowler–Noll–Vo hash (FNV-1a) for deterministic results
		std::uint32_t hash = 2166136261U;
		for (char c : name) {
			hash = (hash ^ static_cast<unsigned char>(c)) * 16777619U;
		}
		return hash;
	}

	uint32_t Size{};
	uint32_t Type{};

};

/// <summary>
/// Packet 
/// </summary>

struct Packet {

	static constexpr size_t HeaderSize = sizeof(Header);

	using byte_t = uint8_t;
	using bytearray = std::vector<byte_t>;

	using byte_iterator_t = decltype(std::declval<const bytearray>().end());
	
	using byte_view = std::span<const byte_t>;
	using mut_byte_view = std::span<byte_t>;

	template<class T, class dummyT = std::nullptr_t>
	using to_byteable_d = std::enable_if_t<std::is_same<decltype(std::declval<const T>().ToBytes()), bytearray>::value, dummyT>;
	template<class T>
	using to_byteable = to_byteable_d<T, T>;
	
	template<class T, class dummyT = std::nullptr_t>
	using from_byteable_d = std::enable_if_t<std::is_same<decltype(std::declval<T>().FromBytes(std::declval<byte_view>())), byte_view>::value, dummyT>;
	template<class T>
	using from_byteable = from_byteable_d<T, T>;

	template<class T>
	using cross_convertible_d = from_byteable_d<to_byteable<T>>;
	template<class T>
	using cross_convertable = from_byteable<to_byteable<T>>;

	template<class, class = void>
	struct is_cross_convertable : std::false_type {};
	template<class T>
	struct is_cross_convertable<T, std::void_t<cross_convertible_d<T>>> : std::true_type {};

	template<class T, class dummyT = std::nullptr_t>
	using memcpy_able_d = std::enable_if_t<std::is_trivially_copyable_v<T> && !is_cross_convertable<T>::value, dummyT>;
	template<class T>
	using memcpy_able = memcpy_able_d<T, T>;

	Packet(const Packet&) = default;
	Packet(Packet&&) = default;

	Packet& operator=(const Packet&) = default;
	Packet& operator=(Packet&&) = default;

	Packet() {};
	Packet(const bytearray&) = delete;
	Packet(bytearray&&) = delete;
	Packet& operator=(const bytearray&) = delete;
	Packet& operator=(bytearray&&) = delete;

	Packet(uint32_t hash, const void* src, uint32_t size) {
		Header head(hash);
		head.Size = size;
		m_buffer.resize(HeaderSize + head.Size);
		std::memcpy(m_buffer.data(), std::addressof(head), HeaderSize);
		std::memcpy(m_buffer.data() + HeaderSize, src, head.Size);
	}

	template<SocketUtil::enum32_t enumT>
	Packet(enumT datatype, const void* src, uint32_t size) : Packet(static_cast<uint32_t>(datatype), src, size) {}

	Packet(uint32_t id, const bytearray& data) : Packet(id, data.data(), data.size()) {}
	template<SocketUtil::enum32_t enumT>
	Packet(enumT type, const bytearray& data) : Packet(type, data.data(), data.size()) {}
	
	template<size_t len>
	Packet(size_t id, const char(&data)[len]) : Packet(id, std::addressof(data), len - 1) {}
	template<SocketUtil::enum32_t enumT, size_t len>
	Packet(enumT type, const char(&data)[len]) : Packet(static_cast<uint32_t>(type), std::addressof(data), len - 1) {}
	template<size_t len>
	Packet(const char(&data)[len]) : Packet(Header::type_hash_code<std::string>(), std::addressof(data), len - 1) {}

	Packet(uint32_t id, const std::string& data) : Packet(id, data.data(), data.size()) {}
	template<SocketUtil::enum32_t enumT>
	Packet(enumT type, const std::string& data) : Packet(type, data.data(), data.size()) {}
	Packet(const std::string& data) : Packet(Header::type_hash_code<std::string>(), data.data(), data.size()) {}
	
	template<class T>
	Packet(uint32_t id, const T& data, memcpy_able_d<T> dummy_0 = {}) : Packet(id, std::addressof(data), sizeof(T)) {}
	template<SocketUtil::enum32_t enumT, class T>
	Packet(enumT type, const T& data, memcpy_able_d<T> dummy_0 = {}) : Packet(static_cast<uint32_t>(type), std::addressof(data), sizeof(T)) {}
	template<class T>
	Packet(const T& data, memcpy_able_d<T> dummy_0 = {}) : Packet(Header::type_hash_code<T>(), std::addressof(data), sizeof(T)) {}

	template<class T>
	Packet(uint32_t id, const std::vector<T>& data, memcpy_able_d<T> dummy_0 = {}) : Packet(id, data.data(), data.size() * sizeof(T)) {}
	template<SocketUtil::enum32_t enumT, class T>
	Packet(enumT type, const std::vector<T>& data, memcpy_able_d<T> dummy_0 = {}) : Packet(static_cast<uint32_t>(type), data.data(), data.size() * sizeof(T)) {}
	template<class T>
	Packet(const std::vector<T>& data, memcpy_able_d<T> dummy_0) : Packet(Header::type_hash_code<std::vector<T>>(), data.data(), data.size() * sizeof(T)) {}

	template<class T>
	Packet(uint32_t id, const T& data, cross_convertible_d<T> dummy_0 = {}) {
		bytearray _data = Convert<T>(data);
		*this = Packet(id, _data.data(), _data.size());
	}
	template<SocketUtil::enum32_t enumT, class T>
	Packet(enumT type, const T& data, cross_convertible_d<T> dummy_0 = {}) : Packet(static_cast<uint32_t>(type), data) {}
	template<class T>
	Packet(const T& data, cross_convertible_d<T> dummy_0 = {}) : Packet(Header::type_hash_code<T>(), data) {}

	template<class T>
	Packet(uint32_t id, const std::vector<T>& data, cross_convertible_d<T> dummy_0 = {}) {
		bytearray b;
		b.reserve(data.size() * sizeof(T));
		for (auto&& elem : data) {
			bytearray temp = Convert<T>(elem);
			b.insert(b.end(), temp.begin(), temp.end());
		}
		*this = Packet(id, b.data(), b.size());
	}
	template<SocketUtil::enum32_t enumT, class T>
	Packet(enumT type, const std::vector<T>& data, cross_convertible_d<T> dummy_0 = {}) : Packet(static_cast<uint32_t>(type), data) {}
	template<class T>
	Packet(const std::vector<T>& data, cross_convertible_d<T> dummy_0 = {}) : Packet(Header::type_hash_code<std::vector<T>>(), data) {}

	Packet(uint32_t id, std::ifstream& ifs) {

		if (!ifs.is_open()) {
			return;
		}

		std::istreambuf_iterator<char> begin(ifs);
		std::istreambuf_iterator<char> end;

		std::string data(begin, end);

		*this = Packet(id, data);
	}
	template<SocketUtil::enum32_t enumT>
	Packet(enumT type, std::ifstream& ifs) : Packet(static_cast<uint32_t>(type), ifs) {}
	explicit Packet(std::ifstream& ifs) : Packet(Header::type_hash_code<FILE>(), ifs) {}

	/*
	
	explicit Packet(uint32_t id, const std::filesystem::path& path) {
		std::error_code ec;
		if (path.empty() || !std::filesystem::exists(path, ec) || ec) {
			return;
		}

		const auto size = std::filesystem::file_size(path, ec);
		if (ec) {
			return;
		}

		std::ifstream ifs(path, std::ios::binary);

		if (!ifs.is_open()) {
			return;
		}

		buf_t data(size);
		ifs.read(reinterpret_cast<char*>(data.data()), size);

		ifs.close();

		*this = Packet(id, data);
	}
	template<class enumT>
	explicit Packet(enumT type, const std::filesystem::path& path, Header::enum32_t<enumT> dummy_0 = {}) : Packet(static_cast<uint32_t>(type), path) {}
	explicit Packet(const std::filesystem::path& path) : Packet(Header::type_hash_code<FILE>(), path) {}
	
	*/
	
	size_t Size() const { return m_buffer.size(); }

	const bytearray& GetBuffer() const { return m_buffer; }
	Packet& SetBuffer(bytearray&& src) {
		m_buffer = std::move(src);
		return *this;
	}

	std::optional<Header> GetHeader() const {
		if (CheckHeader(0)) {
			return std::nullopt;
		}
		Header ret;
		std::memcpy(&ret, m_buffer.data(), HeaderSize);
		return ret;
	}

	template<class T>
	std::optional<memcpy_able<T>> Get() const {
		if (CheckHeader(sizeof(T))) {
			return std::nullopt;
		}
		T ret{};
		std::memcpy(&ret, m_buffer.data() + HeaderSize, sizeof(T));
		return ret;
	}

	template<class T>
	std::optional<from_byteable<T>> Get() const {
		if (CheckHeader()) {
			return std::nullopt;
		}
		auto&& [ret, _] = Convert<T>(byte_view(m_buffer).subspan(HeaderSize));
		return ret;
	}
	
	template<class T>
	std::optional<std::enable_if_t<std::is_same<T, std::string>::value, std::string>> Get() const {
		if (CheckHeader()) {
			return std::nullopt;
		}
		size_t size = m_buffer.size() - HeaderSize;
		std::string ret(size, '\0');
		std::memcpy(ret.data(), m_buffer.data() + HeaderSize, size);
		return ret;
	}

	template<class T>
	std::optional<std::vector<memcpy_able<T>>> GetArray() const {
		if (CheckHeader()) {
			return std::nullopt;
		}
		size_t dataSize = (m_buffer.size() - HeaderSize) / sizeof(T);
		std::vector<T> data(dataSize);
		std::memcpy(data.data(), m_buffer.data() + HeaderSize, m_buffer.size() - HeaderSize);
		return data;
	}

	template<class T>
	std::optional<std::vector<from_byteable<T>>> GetArray() const {
		if (CheckHeader()) {
			return std::nullopt;
		}
		std::vector<T> ret;
		byte_view view = byte_view(m_buffer.begin(), HeaderSize);
		while (view.begin() < view.end()) {
			auto&& [elem, last] = Convert<T>(view);
			ret.push_back(std::move(elem));
			view = last;
		}
		return ret;
	}

	template<class T>
	static bytearray Convert(const to_byteable<T> &from) {
		return from.ToBytes();
	}

	template<class T>
	static std::pair<from_byteable<T>, byte_view> Convert(byte_view from) {
		T ret;
		byte_view view = ret.FromBytes(from);
		return {ret, view};
	}
	
	static void StoreBytes(bytearray& dest, const void* src, uint32_t size) {
		dest.insert(dest.end(), static_cast<const uint8_t*>(src), static_cast<const uint8_t*>(src) + size);
	}
	static void LoadBytes(byte_view& view, void* dest, uint32_t size) {
		std::copy(view.begin(), view.begin() + size, static_cast<uint8_t*>(dest));
		view = view.subspan(size);
	}

	template<class T>
	static void StoreBytes(bytearray& dest, const T& src, memcpy_able_d<T> dummy_0 = {}) {
		StoreBytes(dest, &src, sizeof(T));
	}
	template<class T>
	static void LoadBytes(byte_view& view, T& dest, memcpy_able_d<T> dummy_0 = {}) {
		LoadBytes(view, &dest, sizeof(T));
	}

	template<class T>
	static void StoreBytes(bytearray& dest, const T& src, cross_convertible_d<T> dummy_0 = {}) {
		bytearray data = Convert<T>(src);
		StoreBytes(dest, data.data(), data.size());
	}
	template<class T>
	static void LoadBytes(byte_view& view, T& dest, cross_convertible_d<T> dummy_0 = {}) {
		auto&& [ret, last] = Convert<T>(view);
		dest = std::move(ret);
		view = last;
	}

	template<class T>
	static void StoreBytes(bytearray& dest, const std::vector<T>& src, memcpy_able_d<T> dummy_0 = {}) {
		uint32_t size = src.size();
		StoreBytes(dest, size);
		StoreBytes(dest, src.data(), sizeof(T) * size);
	}
	template<class T>
	static void LoadBytes(byte_view& view, std::vector<T>& dest, memcpy_able_d<T> dummy_0 = {}) {
		uint32_t size = 0;
		LoadBytes(view, size);
		dest.resize(size);
		LoadBytes(view, dest.data(), sizeof(T) * size);
	}

	template<class T>
	static void StoreBytes(bytearray& dest, const std::vector<T>& src, cross_convertible_d<T> dummy_0 = {}) {
		uint32_t size = src.size();
		StoreBytes(dest, size);
		for (auto&& elem : src) {
			bytearray data = Convert<T>(elem);
			StoreBytes(dest, data.data(), data.size());
		}
	}
	template<class T>
	static void LoadBytes(byte_view& view, std::vector<T>& dest, cross_convertible_d<T> dummy_0 = {}) {
		uint32_t size = 0;
		LoadBytes(view, size);
		dest.clear();
		dest.reserve(size);
		for (size_t i = 0; i < size; ++i) {
			auto&& [ret, last] = Convert<T>(view);
			dest.push_back(std::move(ret));
			view = last;
		}
	}

	static void StoreBytes(bytearray& dest, const std::string& src) {
		uint32_t size = src.size();
		StoreBytes(dest, size);
		StoreBytes(dest, src.data(), src.size());
	}
	static void LoadBytes(byte_view& view, std::string& dest) {
		uint32_t size = 0;
		LoadBytes(view, size);
		dest.resize(size);
		LoadBytes(view, dest.data(), size);
	}

	static void StoreBytes(bytearray& dest, const std::vector<std::string>& src) {
		uint32_t size = src.size();
		StoreBytes(dest, size);
		for (auto&& elem : src) {
			StoreBytes(dest, elem);
		}
	}
	static void LoadBytes(byte_view& view, std::vector<std::string>& dest) {
		uint32_t size = 0;
		LoadBytes(view, size);
		dest.clear();
		dest.reserve(size);
		for (size_t i = 0; i < size; ++i) {
			std::string ret;
			LoadBytes(view, ret);
			dest.push_back(std::move(ret));
		}
	}

	bool CheckHeader(size_t option = 1) const {
		return m_buffer.size() < HeaderSize + option;
	}

private:

	bytearray m_buffer{};

};
