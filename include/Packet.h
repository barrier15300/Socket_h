#pragma once
#include "common.h"

/// <summary>
/// Header in Packet
/// </summary>

struct Header {

	Header() {}
	template<SocketDetail::enum32 T>
	explicit Header(T _enum) : Header(static_cast<uint32_t>(_enum)) {}
	explicit Header(uint32_t datatype) {
		Type = datatype;
	}

	template<SocketDetail::enum32 T>
	T TypeAs() const {
		return static_cast<T>(Type);
	}

	template<SocketDetail::enum32 T>
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
		// Fowler–Noll–Vo id (FNV-1a) for deterministic results
		std::uint32_t hash = 2166136261U;
		for (char c : name) {
			hash = (hash ^ static_cast<unsigned char>(c)) * 16777619U;
		}
		return hash;
	}

	uint32_t Size{};
	uint32_t Type{};
	uint32_t _reserved_field[2]{};

	template<typename T>
	static uint32_t type_id() {
		static constexpr std::string_view name = type_name<T>();
		static std::unordered_map<std::string_view, uint32_t> idstorage{};
		static uint32_t newid = 0;
		static std::mutex mtx;

		std::lock_guard<std::mutex> lock(mtx);

		uint32_t ret = 0;

		auto it = idstorage.find(name);
		if (it == idstorage.end()) {
			ret = newid;
			idstorage[name] = newid;
			++newid;
		}
		else {
			ret = it->second;
		}

		return ret;
	}

};

/// <summary>
/// Packet 
/// </summary>

struct Packet {

	static constexpr size_t HeaderSize = sizeof(Header);

	using byte_t = SocketDetail::byte_t;
	using bytearray = SocketDetail::bytearray;

	using byte_view = SocketDetail::byte_view;
	using byte_ref = SocketDetail::byte_ref;

	using byte_iterator_t = decltype(std::declval<const bytearray>().end());

	template<class T>
	static constexpr bool is_enum32 = SocketDetail::enum32<T>;

	template<class T>
	static constexpr bool memcpyable = SocketDetail::memcpyable<T>;
	
	template<class T>
	static constexpr bool to_byteable = SocketDetail::to_byteable<T>;
	
	template<class T>
	static constexpr bool from_byteable = SocketDetail::from_byteable<T>;

	template<class T>
	static constexpr bool cross_convertible = SocketDetail::cross_convertible<T>;

	Packet(const Packet&) = default;
	Packet(Packet&&) = default;

	Packet& operator=(const Packet&) = default;
	Packet& operator=(Packet&&) = default;

	Packet() {};
	Packet(const bytearray&) = delete;
	Packet(bytearray&&) = delete;
	Packet& operator=(const bytearray&) = delete;
	Packet& operator=(bytearray&&) = delete;

	Packet(uint32_t id, const void* src, uint32_t size) {
		Header head(id);
		head.Size = size;
		m_buffer.resize(HeaderSize + head.Size);
		std::memcpy(m_buffer.data(), std::addressof(head), HeaderSize);
		std::memcpy(m_buffer.data() + HeaderSize, src, head.Size);
	}
	template<class enumT>
	Packet(enumT datatype, const void* src, uint32_t size) requires (is_enum32<enumT>) : Packet(static_cast<uint32_t>(datatype), src, size) {}

	Packet(uint32_t id, const bytearray& data) : Packet(id, data.data(), data.size()) {}
	template<class enumT>
	Packet(enumT type, const bytearray& data) requires (is_enum32<enumT>) : Packet(type, data.data(), data.size()) {}
	
	template<size_t len>
	Packet(size_t id, const char(&data)[len]) : Packet(id, std::addressof(data), len - 1) {}
	template<class enumT, size_t len>
	Packet(enumT type, const char(&data)[len]) requires (is_enum32<enumT>) : Packet(static_cast<uint32_t>(type), std::addressof(data), len - 1) {}
	template<size_t len>
	Packet(const char(&data)[len]) : Packet(Header::type_hash_code<std::string>(), std::addressof(data), len - 1) {}

	Packet(uint32_t id, const std::string& data) : Packet(id, data.data(), data.size()) {}
	template<class enumT>
	Packet(enumT type, const std::string& data) requires (is_enum32<enumT>) : Packet(type, data.data(), data.size()) {}
	Packet(const std::string& data) : Packet(Header::type_hash_code<std::string>(), data.data(), data.size()) {}
	
	template<class T>
	Packet(uint32_t id, const T& data) requires (memcpyable<T> && !cross_convertible<T>) : Packet(id, std::addressof(data), sizeof(T)) {}
	template<class enumT, class T>
	Packet(enumT type, const T& data) requires (is_enum32<enumT> && memcpyable<T> && !cross_convertible<T>) : Packet(static_cast<uint32_t>(type), std::addressof(data), sizeof(T)) {}
	template<class T>
	Packet(const T& data) requires (memcpyable<T> && !cross_convertible<T>) : Packet(Header::type_hash_code<T>(), std::addressof(data), sizeof(T)) {}

	template<class T>
	Packet(uint32_t id, const std::vector<T>& data) requires (memcpyable<T> && !cross_convertible<T>) : Packet(id, data.data(), data.size() * sizeof(T)) {}
	template<class enumT, class T>
	Packet(enumT type, const std::vector<T>& data) requires (is_enum32<enumT> && memcpyable<T> && !cross_convertible<T>) : Packet(static_cast<uint32_t>(type), data.data(), data.size() * sizeof(T)) {}
	template<class T>
	Packet(const std::vector<T>& data) requires (memcpyable<T> && !cross_convertible<T>) : Packet(Header::type_hash_code<std::vector<T>>(), data.data(), data.size() * sizeof(T)) {}

	template<class T>
	Packet(uint32_t id, const T& data) requires (cross_convertible<T>) {
		bytearray _data = Convert<T>(data);
		*this = Packet(id, _data.data(), _data.size());
	}
	template<class enumT, class T>
	Packet(enumT type, const T& data) requires (is_enum32<enumT> && cross_convertible<T>) : Packet(static_cast<uint32_t>(type), data) {}
	template<class T>
	Packet(const T& data) requires (cross_convertible<T>) : Packet(Header::type_hash_code<T>(), data) {}

	template<class T>
	Packet(uint32_t id, const std::vector<T>& data) requires (cross_convertible<T>) {
		bytearray b;
		b.reserve(data.size() * sizeof(T));
		for (auto&& elem : data) {
			bytearray temp = Convert<T>(elem);
			b.insert(b.end(), temp.begin(), temp.end());
		}
		*this = Packet(id, b.data(), b.size());
	}
	template<class enumT, class T>
	Packet(enumT type, const std::vector<T>& data) requires (is_enum32<T> && cross_convertible<T>) : Packet(static_cast<uint32_t>(type), data) {}
	template<class T>
	Packet(const std::vector<T>& data) requires (cross_convertible<T>) : Packet(Header::type_hash_code<std::vector<T>>(), data) {}

	Packet(uint32_t id, std::ifstream& ifs) {

		if (!ifs.is_open()) {
			return;
		}

		std::istreambuf_iterator<char> begin(ifs);
		std::istreambuf_iterator<char> end;

		std::string data(begin, end);

		*this = Packet(id, data);
	}
	template<class enumT>
	Packet(enumT type, std::ifstream& ifs) requires (is_enum32<enumT>) : Packet(static_cast<uint32_t>(type), ifs) {}
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
	explicit Packet(enumT type, const std::filesystem::path& path, Header::enum32<enumT> dummy_0 = {}) : Packet(static_cast<uint32_t>(type), path) {}
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
	std::optional<T> Get() const requires (memcpyable<T> && !from_byteable<T>) {
		if (CheckHeader(sizeof(T))) {
			return std::nullopt;
		}
		T ret{};
		std::memcpy(&ret, m_buffer.data() + HeaderSize, sizeof(T));
		return ret;
	}

	template<class T>
	std::optional<T> Get() const requires (from_byteable<T>) {
		if (CheckHeader()) {
			return std::nullopt;
		}
		auto&& [ret, _] = Convert<T>(byte_view(m_buffer).subspan(HeaderSize));
		return ret;
	}
	
	template<class T>
	std::optional<T> Get() const requires (std::same_as<T, std::string>) {
		if (CheckHeader()) {
			return std::nullopt;
		}
		size_t size = m_buffer.size() - HeaderSize;
		std::string ret(size, '\0');
		std::memcpy(ret.data(), m_buffer.data() + HeaderSize, size);
		return ret;
	}

	template<class T>
	std::optional<std::vector<T>> GetArray() const requires (memcpyable<T> && !from_byteable<T>){
		if (CheckHeader()) {
			return std::nullopt;
		}
		size_t dataSize = (m_buffer.size() - HeaderSize) / sizeof(T);
		std::vector<T> data(dataSize);
		std::memcpy(data.data(), m_buffer.data() + HeaderSize, m_buffer.size() - HeaderSize);
		return data;
	}

	template<class T>
	std::optional<std::vector<T>> GetArray() const requires (from_byteable<T>) {
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
	static bytearray Convert(const T &from) requires (to_byteable<T>) {
		return from.ToBytes();
	}

	template<class T>
	static std::pair<T, byte_view> Convert(byte_view from) requires (from_byteable<T>) {
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
	static void StoreBytes(bytearray& dest, const T& src) requires (memcpyable<T> && !cross_convertible<T>) {
		StoreBytes(dest, &src, sizeof(T));
	}
	template<class T>
	static void LoadBytes(byte_view& view, T& dest) requires (memcpyable<T> && !cross_convertible<T>) {
		LoadBytes(view, &dest, sizeof(T));
	}

	template<class T>
	static void StoreBytes(bytearray& dest, const std::vector<T>& src) requires (memcpyable<T> && !cross_convertible<T>) {
		uint32_t size = src.size();
		StoreBytes(dest, size);
		StoreBytes(dest, src.data(), sizeof(T) * size);
	}
	template<class T>
	static void LoadBytes(byte_view& view, std::vector<T>& dest) requires (memcpyable<T> && !cross_convertible<T>) {
		uint32_t size = 0;
		LoadBytes(view, size);
		dest.resize(size);
		LoadBytes(view, dest.data(), sizeof(T) * size);
	}

	template<class T>
	static void StoreBytes(bytearray& dest, const T& src) requires (cross_convertible<T>) {
		bytearray data = Convert<T>(src);
		StoreBytes(dest, data.data(), data.size());
	}
	template<class T>
	static void LoadBytes(byte_view& view, T& dest) requires (cross_convertible<T>) {
		auto&& [ret, last] = Convert<T>(view);
		dest = std::move(ret);
		view = last;
	}

	template<class T>
	static void StoreBytes(bytearray& dest, const std::vector<T>& src) requires (cross_convertible<T>) {
		uint32_t size = src.size();
		StoreBytes(dest, size);
		for (auto&& elem : src) {
			bytearray data = Convert<T>(elem);
			StoreBytes(dest, data.data(), data.size());
		}
	}
	template<class T>
	static void LoadBytes(byte_view& view, std::vector<T>& dest) requires (cross_convertible<T>) {
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

	template<class T>
	static void StoreBytes(bytearray& dest, const T& src) requires (std::same_as<T, std::string>) {
		uint32_t size = src.size();
		StoreBytes(dest, size);
		StoreBytes(dest, src.data(), src.size());
	}
	template<class T>
	static void LoadBytes(byte_view& view, T& dest) requires (std::same_as<T, std::string>) {
		uint32_t size = 0;
		LoadBytes(view, size);
		dest.resize(size);
		LoadBytes(view, dest.data(), size);
	}

	template<class T>
	static void StoreBytes(bytearray& dest, const std::vector<T>& src) requires (std::same_as<T, std::string>) {
		uint32_t size = src.size();
		StoreBytes(dest, size);
		for (auto&& elem : src) {
			StoreBytes(dest, elem);
		}
	}
	template<class T>
	static void LoadBytes(byte_view& view, std::vector<T>& dest) requires (std::same_as<T, std::string>) {
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
