#ifndef __MULTIBYTEINT_V2_H
#define __MULTIBYTEINT_V2_H

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

class bigint_exception {

	static inline const std::string ExceptionPrefix = "bigint: ";

	static std::string setmsg(std::string str) {
		return (ExceptionPrefix + str);
	}

public:

	static std::exception DivisionByZero() {
		return std::exception(setmsg("Division by zero").c_str());
	}

	static std::exception InvalidBase() {
		return std::exception(setmsg("Invalid base").c_str());
	}
};

template<size_t _words = 4, bool _sign = false>
struct bigint {

	using count_t = size_t;
	using word_t = uint_fast64_t;
	using sword_t = int_fast64_t;
	using diff_t = int64_t;
	using halfword_t = uint_fast32_t;

	static constexpr bool IsSigned = _sign;
	static constexpr count_t Words = _words;
	static constexpr count_t WordByte = sizeof(word_t);
	static constexpr count_t WordBytes = Words * WordByte;
	static constexpr count_t WordBits = WordByte * 8;
	static constexpr count_t WordCharSize = WordByte * 2;
	static constexpr count_t AllBits = Words * WordBits;

	static_assert(Words > 0, "invalid WordCount");

	using arr_t = std::array<word_t, Words>;
	using bits_t = std::bitset<AllBits>;
	using signed_t = bigint<Words, true>;
	using unsigned_t = bigint<Words, false>;


	/// Constructor

	constexpr bigint() noexcept {}
	constexpr bigint(const bigint& from) noexcept { *m_impl = *from.m_impl; }
	constexpr bigint(bigint&& from) noexcept : m_impl(nullptr) { m_impl = from.m_impl; from.m_impl = nullptr; }
	template<typename = std::enable_if_t<!IsSigned>>
	constexpr bigint(std::conditional_t<!IsSigned, word_t, std::nullptr_t> from) noexcept { words()[0] = from; }
	template<typename = std::enable_if_t<IsSigned>>
	constexpr bigint(std::conditional_t<IsSigned, sword_t, std::nullptr_t> from) noexcept {
		if (from < 0) {
			from = -from;
			words()[0] = (word_t)from;
			Negate();
		}
		else {
			words()[0] = (word_t)from;
		}
	}
	constexpr bigint(std::initializer_list<word_t> list) { std::copy(std::rbegin(list), std::rend(list), words().begin()); }
	constexpr bigint(std::initializer_list<std::string> list) {
		std::string ret;
		ret.reserve(list.begin()->size() * list.size());
		for (auto&& str : list) {
			ret += str;
		}
		*this = bigint(ret);
	}
	constexpr explicit bigint(const std::string& text) { this->Parse(text); }
	constexpr explicit bigint(std::string_view text) { this->Parse(text); }
	constexpr explicit bigint(const char* text) { this->Parse(text); }
	template<count_t inputlength>
	constexpr bigint(const char(&text)[inputlength]) { this->Parse(std::string_view(text, inputlength - 1)); }
	~bigint() {
		delete m_impl;
		m_impl = nullptr;
	}



	/// Assignment Operator Module

	constexpr bigint& operator=(const bigint& from) noexcept { *m_impl = *from.m_impl; return *this; }
	constexpr bigint& operator=(bigint&& from) noexcept { delete m_impl; m_impl = from.m_impl; from.m_impl = nullptr; return *this; }
	template<typename = std::enable_if_t<!IsSigned>>
	constexpr bigint& operator=(word_t from) noexcept {
		*this = std::move(bigint(from));
		return *this;
	}
	template<typename = std::enable_if_t<IsSigned>>
	constexpr bigint& operator=(sword_t from) noexcept {
		*this = std::move(bigint(from));
		return *this;
	}
	constexpr bigint& operator=(std::string_view text) noexcept {
		return this->Parse(text);
	}
	
	constexpr bigint& operator+=(const bigint& rhs) { return AssignAdd(rhs); }
	constexpr bigint& operator-=(const bigint& rhs) { return AssignSub(rhs); }
	constexpr bigint& operator*=(const bigint& rhs) { return AssignMul(rhs); }
	constexpr bigint& operator/=(const bigint& rhs) { return AssignDiv(rhs); }
	constexpr bigint& operator%=(const bigint& rhs) { return AssignMod(rhs); }

	constexpr friend bigint operator+(bigint lhs, const bigint& rhs) { return lhs += rhs; }
	constexpr friend bigint operator-(bigint lhs, const bigint& rhs) { return lhs -= rhs; }
	constexpr friend bigint operator*(bigint lhs, const bigint& rhs) { return lhs *= rhs; }
	constexpr friend bigint operator/(bigint lhs, const bigint& rhs) { return lhs /= rhs; }
	constexpr friend bigint operator%(bigint lhs, const bigint& rhs) { return lhs %= rhs; }



	/// Mathematical Module

	constexpr bigint& Negate() {
		return this->AssignNot().AssignAdd(1);
	}
	constexpr bigint operator-() const {
		return bigint(*this).Negate();
	}
	constexpr bigint Abs() const {
		return *this;
	}
	constexpr bigint Pow(const bigint& x) const {
		bigint ret = 1;
		bigint base = *this;

		count_t nbit = x.GetNBit();

		for (count_t i = 0; i < nbit; ++i) {
			if (x.BitCheck(i)) {
				ret *= base;
			}
			base *= base;
		}

		return ret;
	}
	constexpr bigint Sqrt() const {
		if (*this == 0) {
			return 0;
		}
		bigint left = 1;
		bigint right = *this;
		bigint mid;
		while (left < right) {
			mid = (left + right + 1) >> 1;
			if (mid * mid > *this) {
				right = mid - 1;
			}
			else {
				left = mid;
			}
		}
		return left;
	}
	constexpr bigint Factorial() const {
		bigint ret = 1;
		for (bigint i = 0; i++ < *this;) {
			ret *= i;
		}
		return ret;
	}
	constexpr bigint HyperArrow(const bigint& n, const bigint& b) const {
		if (n == 0) return *this * b;
		if (n == 1) return this->Pow(b);
		if (b == 0) return 1;

		bigint result = *this;
		for (bigint i = 1; i < b; ++i) {
			result = this->HyperArrow(n - 1, result);
		}
		return result;
	}
	

	/// Utility Module

	constexpr bool BitCheck(count_t idx) const {
		return bits()[idx];
	}
	template<typename = std::enable_if_t<IsSigned>>
	constexpr bool IsNegative() const  {
		return BitCheck(AllBits - 1);
	}
	constexpr count_t GetWordLZeroCount(word_t word) const {
		return _lzcnt_u64(word);
	}
	constexpr count_t GetWordNBit(word_t word) const {
		return WordBits - GetWordLZeroCount(word);
	}
	constexpr count_t GetNBit() const {
		for (count_t i = Words; i-- > 0;) {
			if (words()[i] != 0) {
				return i * WordBits + GetWordNBit(words()[i]);
			}
		}
		return AllBits;
	}
	constexpr count_t GetNWord() const {
		for (count_t i = Words; i-- > 0;) {
			if (words()[i] != 0) {
				return i + 1;
			}
		}
		return Words;
	}
	constexpr explicit operator word_t() const {
		return words()[0];
	}
	constexpr explicit operator sword_t() const {
		return words()[0];
	}
	template<typename = std::enable_if_t<!IsSigned>>
	constexpr operator signed_t&() {
		return *(signed_t*)(this);
	}
	template<typename = std::enable_if_t<IsSigned>>
	constexpr operator unsigned_t&()  {
		return *(unsigned_t*)(this);
	}
	constexpr bool AddOutCheck(const bigint& src) const {
		return (this->GetNBit() == AllBits && src.GetNBit() > 0);
	}
	constexpr bool MulOutCheck(const bigint& src) const {
		return (this->GetNBit() + src.GetNBit() > AllBits);
	}
	


	/// Arithmetic Module
	
	constexpr bool AddBase(word_t *dest, word_t src, bool carry) const {
		word_t max = *dest > src ? *dest : src;
		*dest += src + carry;
		return *dest < max;
	}
	constexpr bigint& AssignAdd(const bigint& src) {
		bool carry = false;
		for (count_t i = 0; i < Words; ++i) {
			carry = AddBase(
				std::addressof(this->words()[i]),
				src.words()[i],
				carry
			);
		}
		return *this;
	}
	constexpr bigint& AssignSub(const bigint& src) {
		bool borrow = true;
		for (count_t i = 0; i < Words; ++i) {
			borrow = AddBase(
				std::addressof(this->words()[i]),
				~src.words()[i],
				borrow
			);
		}
		return *this;
	}
	constexpr std::pair<word_t, word_t> MulBase(word_t a, word_t b) const {
		constexpr size_t halfbits = sizeof(halfword_t) * 8;
		constexpr word_t halfmask = (((word_t)1 << halfbits) - 1);
		word_t al = a & halfmask;  // a[1][ ]
		word_t ah = (a >> halfbits) & halfmask; // a[ ][1]
		word_t bl = b & halfmask;  // b[1][ ]
		word_t bh = (b >> halfbits) & halfmask; // b[ ][1]

		word_t t1 = al * bl; // t1[1][1][ ][ ]
		word_t t2 = ah * bh; // t2[ ][ ][?][?]

		word_t tm1 = al * bh; // tm1[ ][?][?][ ]
		word_t tm2 = bl * ah; // tm2[ ][1][1][ ]

		tm1 += (t1 >> halfbits) & halfmask; // tm1[ ][1][1][ ]

		word_t tm = tm1;						// tm[ ][?][?][ ]
		bool carry = AddBase(&tm, tm2, false);  // tm[ ][1][1][ ]

		(t1 &= halfmask) |= (tm & halfmask) << halfbits; // t1[1][1][ ][ ] 
		t2 += ((tm >> halfbits) & halfmask) + carry;	 // t2[ ][ ][1][1]

		return { t1, t2 }; // ret[1][1][1][1]
	}
	constexpr bigint& AssignMul(bigint src) {
		
		const bigint base = *this;
		*this = 0;
		
		for (count_t j = 0; j < Words; ++j) {
			const word_t src_word = src.words()[j];
			
			if (src_word == 0) {
				continue;
			}
			
			word_t carry = 0;
			bool carryflag = false;
			
			for (count_t i = 0; i + j < Words; ++i) {
				word_t temp = carry;
			
				const auto [lower, upper] = MulBase(
					src_word,
					base.words()[i]
				);
				
				carryflag = AddBase(&temp, lower, carryflag);
				
				carry = upper + carryflag;
				
				carryflag = AddBase(
					std::addressof(this->words()[i + j]),
					temp,
					false
				);
			}
		}

		return *this;
	}
	constexpr std::pair<bigint&, bigint> AssignDivMod(bigint src) {
		if (src == 0) {
			throw bigint_exception::DivisionByZero();
		}

		bigint rem = *this;
		*this = 0;
		
		while (!(rem < src)) {

			diff_t shift = rem.GetNBit() - src.GetNBit();
			{
				bigint temp = src << shift;
				if (rem < temp) {
					temp >>= 1;
					--shift;
				}
				rem -= temp;
				bits()[shift] = true;
			}
		}

		return { *this, rem };
	}
	constexpr bigint& AssignDiv(const bigint& src) {
		return AssignDivMod(src).first;
	}
	constexpr bigint& AssignMod(const bigint& src) {
		return *this = std::move(AssignDivMod(src).second);
	}



	/// Logical Module

	constexpr int Compare(const bigint& src) const {
		auto lhs_b = words().rbegin(), lhs_e = words().rend(), rhs_b = src.words().rbegin();
		auto [ret_t, ret_s] = std::mismatch(lhs_b, lhs_e, rhs_b);
		if (ret_t == lhs_e && ret_s == (rhs_b + std::distance(lhs_b, lhs_e))) {
			return 0;
		}
		return (*ret_t > *ret_s) ? 1 : -1;
	}
	constexpr friend bool operator==(const bigint& lhs, const bigint& rhs) { return lhs.Compare(rhs) == 0; }
	constexpr friend bool operator!=(const bigint& lhs, const bigint& rhs) { return lhs.Compare(rhs) != 0; }
	constexpr friend bool operator< (const bigint& lhs, const bigint& rhs) { return lhs.Compare(rhs) <  0; }
	constexpr friend bool operator<=(const bigint& lhs, const bigint& rhs) { return lhs.Compare(rhs) <= 0; }
	constexpr friend bool operator> (const bigint& lhs, const bigint& rhs) { return lhs.Compare(rhs) >  0; }
	constexpr friend bool operator>=(const bigint& lhs, const bigint& rhs) { return lhs.Compare(rhs) >= 0; }
	constexpr bigint& AssignLeftShift(word_t c) {
		bits() <<= c;
		return *this;
	}
	constexpr bigint& AssignRightShift(word_t c) {
		if constexpr (IsSigned) {
			if (this->IsNegative()) {
				unsigned_t shiftmask = 1;
				shiftmask <<= AllBits - c - 1;
				--shiftmask;
				shiftmask.AssignNot();
				bits() >>= c;
				*this |= shiftmask;
			}
			else {
				bits() >>= c;
			}
		}
		else {
			bits() >>= c;
		}
		return *this;
	}
	constexpr bigint& operator<<=(word_t c) { return AssignLeftShift(c); }
	constexpr bigint& operator>>=(word_t c) { return AssignRightShift(c); }
	constexpr friend bigint operator<<(bigint lhs, word_t c) { return lhs.AssignLeftShift(c); }
	constexpr friend bigint operator>>(bigint lhs, word_t c) { return lhs.AssignRightShift(c); }
	constexpr bigint& AssignNot() { bits().flip(); return *this; }
	constexpr bigint operator~() const { return bigint(*this).AssignNot(); }
	constexpr bigint& AssignAnd(const bigint& src) { bits() &= src.bits(); return *this; }
	constexpr bigint& AssignOr(const bigint& src) { bits() |= src.bits(); return *this; }
	constexpr bigint& AssignXor(const bigint& src) { bits() ^= src.bits(); return *this; }
	constexpr bigint& operator&=(const bigint& src) { return AssignAnd(src); }
	constexpr bigint& operator|=(const bigint& src) { return AssignOr(src); }
	constexpr bigint& operator^=(const bigint& src) { return AssignXor(src); }
	constexpr friend bigint operator&(bigint lhs, const bigint& rhs) { return lhs.AssignAnd(rhs); }
	constexpr friend bigint operator|(bigint lhs, const bigint& rhs) { return lhs.AssignOr (rhs); }
	constexpr friend bigint operator^(bigint lhs, const bigint& rhs) { return lhs.AssignXor(rhs); }
	constexpr bigint& operator++() { return AssignAdd(1); }
	constexpr bigint operator++(int) { auto ret = *this; AssignAdd(1); return ret; }
	constexpr bigint& operator--() { return AssignSub(1); }
	constexpr bigint operator--(int) { auto ret = *this; AssignSub(1); return ret; }



	/// IO Module

	constexpr bigint& Parse(std::string_view text, int base = 16) {
		auto proc = text.substr(0, text.find_first_not_of("0123456789abcdefABCDEF"));
		auto it = proc.rbegin();
		auto end = proc.rend();
		count_t c = 0;
		bool breaked = false;
		do {
			
			auto copybegin = it;
			auto copyend = it;

			if (end - it < WordCharSize) {
				copybegin = end;
				breaked = true;
			}
			else {
				it += WordCharSize;
				copybegin = it;
			}

			auto test = std::string(copybegin.base(), copyend.base());

			this->words()[c] = std::stoull(test, nullptr, base);

			++c;
		} while (c < Words && !breaked);

		return *this;
	}
	constexpr std::string ToHex(bool upper = true, bool padding = true) const {
		std::string ret;
		for (auto w : words()) {
			char buffer[WordCharSize + 1]{};
			char* beg = std::begin(buffer);
			char* end = std::end(buffer);

			auto [p, ec] = std::to_chars(beg, end, w, 16);
			
			std::string temp(buffer, p);
			
			std::reverse(temp.begin(), temp.end());
			temp += std::string(WordCharSize - (p - beg), '0');

			ret += temp;
		}

		std::reverse(ret.begin(), ret.end());

		if (upper) {
			std::transform(
				ret.begin(),
				ret.end(),
				ret.begin(),
				[](char c) -> char {
					return std::toupper(c);
				}
			);
		}

		if (!padding) {
			size_t idx = ret.find_first_not_of('0');
			ret = ret.substr(
				(idx == std::string::npos) ? 
				ret.size() - 1 : idx
			);
		}

		return ret;
	}
	constexpr std::string ToBase64() const {
		constexpr std::string_view list = "ABCDEFGHIJKNMLOPQRSTUVWXYZabcdefghijknmlopqrstuvwxyz0123456789+/";

		std::string ret;
		ret.reserve(this->GetNBit() / std::log2(64) + 1);

		bigint rem = *this;

		do {
			char word = list[((word_t)rem) & ((1 << 6) - 1)];
			ret += word;
			rem >>= 6;
		} while (!(rem < 64));

		if (rem != 0) {
			rem <<= 7 - rem.GetNBit();
			char word = list[((word_t)rem) & ((1 << 6) - 1)];
			ret += word;
		}

		std::reverse(ret.begin(), ret.end());

		count_t lmod = ((ret.size() - 1) & 3) + 1;
		ret += std::string(4 - lmod, '=');
		
		return ret;
	}
	constexpr std::vector<uint8_t> ToBytes() const {
		std::vector<uint8_t> ret;
		if (*this == 0) { ret.resize(1); return ret; }
		count_t n = this->GetNBit();
		count_t bytes = n / 8 + 1;
		ret.resize(bytes);
		std::memcpy(ret.data(), words().data(), bytes);
		return ret;
	}
	/*
	constexpr std::string ToString(int base = 10, bool upper = true, bool padding = false) const {
		constexpr std::string_view list = "0123456789abcdefghijkmnlopqrstuvwxyz";
		
		if (base < 2 && base > 36) {
			throw bigint_exception::InvalidBase();
		}

		word_t word_digits = (word_t)(WordBits / std::log2(base));
		bigint word_base = bigint(base).Pow(word_digits);

		if constexpr (Words == 1) {

			auto val = std::conditional_t<IsSigned, sword_t, word_t>(*this);

			std::string temp;
			temp.resize(word_digits + 1);

			char* beg = std::to_address(temp.begin());
			char* end = std::to_address(temp.end());

			auto [p, ec] = std::to_chars(beg, end, val, base);
			temp.erase(p - beg);

			return temp;
		}

		std::string ret;
		ret.reserve(this->GetNBit() / std::log2(base) + 1);

		bigint rem = *this;
		bool neg = false;

		if (rem == 0) {
			return "0";
		}

		if constexpr (IsSigned) {
			if (IsNegative()) {
				bigint soc = ~(bigint(1) << (AllBits - 1));
				rem = ~(rem & soc) - soc;
				neg = true;
			}
		}

		std::string temp;
		temp.resize(word_digits + 1);

		do {
			word_t mod = (word_t)rem.AssignDivMod(word_base).second;

			char* beg = std::to_address(temp.begin());
			char* end = std::to_address(temp.end());

			auto [p, ec] = std::to_chars(beg, end, mod, base);
			temp.erase(p - beg);

			std::reverse(temp.begin(), temp.end());
			temp += std::string(word_digits - (p - beg), '0');

			ret += temp;

		} while (rem != 0);
		
		if (!padding) {
			ret = ret.substr(0, ret.find_last_not_of('0') + 1);
		}

		if (neg) {
			ret.push_back('-');
		}

		std::reverse(ret.begin(), ret.end());
		
		if (upper) {
			std::transform(
				ret.begin(),
				ret.end(),
				ret.begin(),
				[](char c) -> char {
					return std::toupper(c);
				}
			);
		}

		return ret;
	}
	*/
#if defined(_DEBUG) // in Debug Mode
	const char* DebugView() const {
		static thread_local std::string buf;
		buf = ToHex();
		return buf.c_str();
	}
#endif

private:

	/// Internal Resource

	arr_t& words() const { return m_impl->m_words; }
	bits_t& bits() const { return m_impl->m_bits; }

	union _impl_resource {
		arr_t m_words{};
		bits_t m_bits;
	};

	_impl_resource* m_impl = new _impl_resource();
};

#endif // __MULTIBYTEINT_V2_H