#pragma once
#include <cstdint>
#include <string_view>
#include <string>
#include <vector>
#include <exception>
#include <stdexcept>
#include <fstream>
#include <cstring>

/// <summary>
/// Header in Packet
/// </summary>

struct Header {

	template<class T>
	using enum32_t = std::enable_if_t<std::is_enum_v<T> && (sizeof(T) == sizeof(uint32_t)), T>;

	Header() {}
	template<class T>
	explicit Header(enum32_t<T> _enum) : Header(static_cast<uint32_t>(_enum)) {}
	explicit Header(uint32_t datatype) {
		Type = datatype;
	}

	template<class T>
	enum32_t<T> TypeAs() const {
		return static_cast<T>(Type);
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

	using buf_t = std::vector<uint8_t>;

	using byte_iterator_t = decltype(std::declval<const buf_t>().end());
	
	template<class containerT>
	struct _view {

		using iterator = decltype(std::declval<containerT>().begin());

		_view(containerT& from) : _beg(from.begin()), _end(from.end()) {}
		_view(containerT& from, ptrdiff_t start_off) : _beg(from.begin() + start_off), _end(from.end()) {}
		_view(containerT& from, ptrdiff_t start_off, ptrdiff_t end_off) : _beg(from.begin() + start_off), _end(from.end() + end_off) {}

		iterator begin() const {
			return _beg;
		}

		iterator end() const {
			return _end;
		}

		iterator _beg;
		iterator _end;

	};

	using byte_view = _view<const buf_t>;
	using mut_byte_view = _view<buf_t>;

	template<class T, class dummyT = std::nullptr_t>
	using to_byteable_d = std::enable_if_t<std::is_same<decltype(std::declval<const T>().ToBytes()), buf_t>::value, dummyT>;
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
	Packet(const buf_t&) = delete;
	Packet(buf_t&&) = delete;
	Packet& operator=(const buf_t&) = delete;
	Packet& operator=(buf_t&&) = delete;

	Packet(uint32_t hash, const void* src, uint32_t size) {
		Header head(hash);
		head.Size = size;
		m_buffer.resize(HeaderSize + head.Size);
		std::memcpy(m_buffer.data(), std::addressof(head), HeaderSize);
		std::memcpy(m_buffer.data() + HeaderSize, src, head.Size);
	}

	template<class enumT>
	Packet(enumT datatype, const void* src, uint32_t size, Header::enum32_t<enumT> dummy_0 = {}) : Packet(static_cast<uint32_t>(datatype), src, size) {}

	Packet(uint32_t id, const std::vector<uint8_t>& data) : Packet(id, data.data(), data.size()) {}
	template<class enumT>
	Packet(enumT type, const std::vector<uint8_t>& data, Header::enum32_t<enumT> dummy_0 = {}) : Packet(static_cast<uint32_t>(type), data.data(), data.size()) {}
	
	template<size_t len>
	Packet(size_t id, const char(&data)[len]) : Packet(id, std::addressof(data), len - 1) {}
	template<class enumT, size_t len>
	Packet(enumT type, const char(&data)[len], Header::enum32_t<enumT> dummy_0 = {}) : Packet(static_cast<uint32_t>(type), std::addressof(data), len - 1) {}
	template<size_t len>
	Packet(const char(&data)[len]) : Packet(Header::type_hash_code<std::string>(), std::addressof(data), len - 1) {}

	Packet(uint32_t id, const std::string& data) : Packet(id, data.data(), data.size()) {}
	template<class enumT>
	Packet(enumT type, const std::string& data, Header::enum32_t<enumT> dummy_0 = {}) : Packet(type, data.data(), data.size()) {}
	Packet(const std::string& data) : Packet(Header::type_hash_code<std::string>(), data.data(), data.size()) {}
	
	template<class T>
	Packet(uint32_t id, const T& data, memcpy_able_d<T> dummy_0 = {}) : Packet(id, std::addressof(data), sizeof(T)) {}
	template<class enumT, class T>
	Packet(enumT type, const T& data, Header::enum32_t<enumT> dummy_0 = {}, memcpy_able_d<T> dummy_1 = {}) : Packet(static_cast<uint32_t>(type), std::addressof(data), sizeof(T)) {}
	template<class T>
	Packet(const T& data, memcpy_able_d<T> dummy_0 = {}) : Packet(Header::type_hash_code<T>(), std::addressof(data), sizeof(T)) {}

	template<class T>
	Packet(uint32_t id, const std::vector<T>& data, memcpy_able_d<T> dummy_0 = {}) : Packet(id, data.data(), data.size() * sizeof(T)) {}
	template<class enumT, class T>
	Packet(enumT type, const std::vector<T>& data, Header::enum32_t<enumT> dummy_0 = {}, memcpy_able_d<T> dummy_1 = {}) : Packet(static_cast<uint32_t>(type), data.data(), data.size() * sizeof(T)) {}
	template<class T>
	Packet(const std::vector<T>& data, memcpy_able_d<T> dummy_0) : Packet(Header::type_hash_code<std::vector<T>>(), data.data(), data.size() * sizeof(T)) {}

	template<class T>
	Packet(uint32_t id, const T& data, cross_convertible_d<T> dummy_0 = {}) {
		buf_t _data = Convert<T>(data);
		*this = Packet(id, _data.data(), _data.size());
	}
	template<class enumT, class T>
	Packet(enumT type, const T& data, Header::enum32_t<enumT> dummy_0 = {}, cross_convertible_d<T> dummy_1 = {}) : Packet(static_cast<uint32_t>(type), data) {}
	template<class T>
	Packet(const T& data, cross_convertible_d<T> dummy_0 = {}) : Packet(Header::type_hash_code<T>(), data) {}

	template<class T>
	Packet(uint32_t id, const std::vector<T>& data, cross_convertible_d<T> dummy_0 = {}) {
		buf_t b;
		b.reserve(data.size() * sizeof(T));
		for (auto&& elem : data) {
			buf_t temp = Convert<T>(elem);
			b.insert(b.end(), temp.begin(), temp.end());
		}
		*this = Packet(id, b.data(), b.size());
	}
	template<class enumT, class T>
	Packet(enumT type, const std::vector<T>& data, Header::enum32_t<enumT> dummy_0 = {}, cross_convertible_d<T> dummy_1 = {}) : Packet(static_cast<uint32_t>(type), data) {}
	template<class T>
	Packet(const std::vector<T>& data, cross_convertible_d<T> dummy_0 = {}) : Packet(Header::type_hash_code<std::vector<T>>(), data) {}

	explicit Packet(std::ifstream& ifs) {
		std::istreambuf_iterator<std::ifstream::char_type> begin(ifs);
		std::istreambuf_iterator<std::ifstream::char_type> end;

		std::string infile(begin, end);

		*this = Packet(Header::type_hash_code<FILE>(), infile);
	}

	size_t Size() const { return m_buffer.size(); }

	const buf_t& GetBuffer() const { return m_buffer; }
	Packet& SetBuffer(buf_t&& src) {
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
		auto&& [ret, _] = Convert<T>({m_buffer, HeaderSize});
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
		byte_view view = {m_buffer, HeaderSize};
		while (view._beg < view._end) {
			auto&& [elem, last] = Convert<T>(view);
			ret.push_back(std::move(elem));
			view._beg = last._beg;
		}
		return ret;
	}

	template<class T>
	static buf_t Convert(const to_byteable<T> &from) {
		return from.ToBytes();
	}

	template<class T>
	static std::pair<from_byteable<T>, byte_view> Convert(byte_view from) {
		T ret;
		byte_view view = ret.FromBytes(from);
		return {ret, view};
	}
	
	static void StoreBytes(buf_t& dest, const void* src, uint32_t size) {
		dest.insert(dest.end(), static_cast<const uint8_t*>(src), static_cast<const uint8_t*>(src) + size);
	}

	template<class T>
	static void StoreBytes(buf_t& dest, const T& src, memcpy_able_d<T> dummy_0 = {}) {
		StoreBytes(dest, &src, sizeof(T));
	}

	template<class T>
	static void StoreBytes(buf_t& dest, const T& src, cross_convertible_d<T> dummy_0 = {}) {
		buf_t data = Convert<T>(src);
		StoreBytes(dest, data.data(), data.size());
	}

	template<class T>
	static void StoreBytes(buf_t& dest, const std::vector<T>& src, memcpy_able_d<T> dummy_0 = {}) {
		uint32_t size = src.size();
		StoreBytes(dest, size);
		StoreBytes(dest, src.data(), sizeof(T) * size);
	}

	template<class T>
	static void StoreBytes(buf_t& dest, const std::vector<T>& src, cross_convertible_d<T> dummy_0 = {}) {
		uint32_t size = src.size();
		StoreBytes(dest, size);
		for (auto&& elem : src) {
			buf_t data = Convert<T>(elem);
			StoreBytes(dest, data.data(), data.size());
		}
	}

	static void StoreBytes(buf_t& dest, const std::string& src) {
		uint32_t size = src.size();
		StoreBytes(dest, size);
		StoreBytes(dest, src.data(), src.size());
	}

	static void LoadBytes(byte_view& view, void* dest, uint32_t size) {
		std::copy(view._beg, view._beg + size, static_cast<uint8_t*>(dest));
		view._beg += size;
	}

	template<class T>
	static void LoadBytes(byte_view& view, T& dest, memcpy_able_d<T> dummy_0 = {}) {
		LoadBytes(view, &dest, sizeof(T));
	}

	template<class T>
	static void LoadBytes(byte_view& view, T& dest, cross_convertible_d<T> dummy_0 = {}) {
		auto&& [ret, last] = Convert<T>(view);
		dest = std::move(ret);
		view._beg = last._beg;
	}

	template<class T>
	static void LoadBytes(byte_view& view, std::vector<T>& dest, memcpy_able_d<T> dummy_0 = {}) {
		uint32_t size = 0;
		LoadBytes(view, size);
		dest.resize(size);
		LoadBytes(view, dest.data(), sizeof(T) * size);
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
			view._beg = last._beg;
		}
	}

	static void LoadBytes(byte_view& view, std::string& dest) {
		uint32_t size = 0;
		LoadBytes(view, size);
		dest.resize(size);
		LoadBytes(view, dest.data(), size);
	}

	bool CheckHeader(size_t option = 1) const {
		return m_buffer.size() < HeaderSize + option;
	}

private:

	buf_t m_buffer{};

};
