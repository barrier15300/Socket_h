#pragma once
#include "common.h"

///
/// fixed-size
///

template<size_t _words, bool _sign = false>
struct bigint {
	using count_t = size_t;
	using word_t = uint64_t;
	using sword_t = int64_t;
	using diff_t = int64_t;
	using halfword_t = uint32_t;

	static constexpr bool IsSigned = _sign;
	static constexpr count_t Words = _words;
	static constexpr count_t WordByte = sizeof(word_t);
	static constexpr count_t WordBytes = Words * WordByte;
	static constexpr count_t WordBits = WordByte * 8;
	static constexpr count_t WordCharSize = WordByte * 2;
	static constexpr count_t AllBits = Words * WordBits;

	using arr_t = std::array<word_t, Words>;
	using arr_view = std::span<const word_t>;
	using arr_ref = std::span<word_t>;
	using bits_t = std::bitset<AllBits>;
	using signed_t = bigint<Words, true>;
	using unsigned_t = bigint<Words, false>;

	/// Constructor

	constexpr bigint() noexcept {}
	constexpr bigint(const bigint& from) noexcept { *m_words = *from.m_words; }
	constexpr bigint(bigint&& from) noexcept : m_words(nullptr) { m_words = from.m_words; from.m_words = nullptr; }
	template<count_t fromwords, bool fromsigned>
	constexpr bigint(const bigint<fromwords, fromsigned>& from) noexcept {
		const count_t copywords = (fromwords < Words) ? fromwords : Words;
		std::copy_n(from.words().begin(), copywords, words().begin());
		if constexpr (fromwords < Words) {
			std::fill_n(words().begin() + copywords, Words - fromwords, 0);
		}
		if constexpr (IsSigned && fromsigned) {
			if (from.IsNegative()) {
				Negate();
			}
		}
	}
	constexpr bigint(word_t from) noexcept requires(!IsSigned) { words()[0] = from; }
	constexpr bigint(sword_t from) noexcept requires(IsSigned) {
		if (from < 0) {
			from = -from;
			words()[0] = static_cast<word_t>(from);
			Negate();
		}
		else {
			words()[0] = static_cast<word_t>(from);
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
	template<std::ranges::range R>
	constexpr bigint(const R& arr) {
		words() = constexpr_bytes_cast<arr_t>(
			constexpr_bytes_cast<R>(arr)
		);
	}
	constexpr explicit bigint(std::string_view text) {
		*this = Parse(text);
	}
	template<count_t inputlength>
	constexpr bigint(const char(&text)[inputlength]) { *this = Parse(std::string_view(text, inputlength - 1)); }
	constexpr ~bigint() {
		delete m_words;
		m_words = nullptr;
	}

	/// Assignment Operator Module

	constexpr bigint& operator=(const bigint& from) noexcept { *m_words = *from.m_words; return *this; }
	constexpr bigint& operator=(bigint&& from) noexcept {
		if (from.m_words == m_words) {
			return *this;
		}
		delete m_words;
		m_words = from.m_words;
		from.m_words = nullptr;
		return *this;
	}
	constexpr bigint& operator=(word_t from) noexcept requires(!IsSigned) {
		*this = std::move(bigint(from));
		return *this;
	}
	constexpr bigint& operator=(sword_t from) noexcept requires(IsSigned) {
		*this = std::move(bigint(from));
		return *this;
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
	constexpr bigint operator-() const { return bigint(*this).Negate(); }
	constexpr bigint operator+() const { return *this; }
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
	constexpr bool IsNegative() const requires(IsSigned) {
		return BitCheck(AllBits - 1);
	}
	constexpr count_t GetWordLZeroCount(word_t word) const {
		return WordBits - std::bit_width(word);
	}
	constexpr count_t GetWordNBit(word_t word) const {
		return std::bit_width(word);
	}
	constexpr count_t GetNWord() const {
		for (count_t i = Words; i-- > 0;) {
			if (words()[i] != 0) {
				return i + 1;
			}
		}
		return Words;
	}
	constexpr count_t GetNBit() const {
		for (count_t i = Words; i-- > 0;) {
			if (words()[i] != 0) {
				return i * WordBits + GetWordNBit(words()[i]);
			}
		}
		return AllBits;
	}
	constexpr explicit operator word_t() const {
		return words()[0];
	}
	constexpr explicit operator sword_t() const {
		return words()[0];
	}
	constexpr operator signed_t&() requires(IsSigned) {
		return *this;  // TODO: remove unneccesary conversion
	}
	constexpr operator unsigned_t& () requires(!IsSigned) {
		return *this;  // TODO: remove unneccesary conversion
	}
	template<std::ranges::range R>
		requires (std::is_convertible_v<word_t, std::ranges::range_value_t<R>>)
	constexpr bigint& FromWords(R&& r) {
		auto beg = m_words->begin();
		auto end = m_words->end();
		for (const auto&& elem : r) {
			if (beg == end) {
				break;
			}
			*(beg++) = elem;
		}
		for (; beg != end; ++beg) {
			*beg = 0;
		}
		return *this;
	}

	/// Arithmetic Module
	
	static constexpr bool AddBase(word_t *dest, word_t src, bool carry) noexcept {
		word_t a = *dest;
		word_t b = src + static_cast<word_t>(carry);
		*dest += b;
		return (b < src) || (*dest < a);
	}
	constexpr bigint& AssignAdd(const bigint& src) noexcept {
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
	static constexpr std::pair<word_t, word_t> MulBase(word_t a, word_t b)  {
		constexpr size_t halfbits = sizeof(halfword_t) * 8;
		constexpr word_t halfmask = ((static_cast<word_t>(1) << halfbits) - 1);

		word_t al = a & halfmask;
		word_t ah = (a >> halfbits) & halfmask;
		word_t bl = b & halfmask;
		word_t bh = (b >> halfbits) & halfmask;

		word_t l1 = al * bl;
		word_t l2 = ah * bl;
		word_t h1 = al * bh;
		word_t h2 = ah * bh;
		
		word_t t1 = l1;
		word_t t2 = h2;

		word_t mls = (l2 & halfmask) + (h1 & halfmask);
		word_t mhs = (l2 >> halfbits) + (h1 >> halfbits);

		word_t mlc = mls >> halfbits;

		bool c = AddBase(&t1, mls << halfbits, false);
		c = AddBase(&t2, mhs + mlc, c);

		return { t1, t2 };
	}
	static constexpr bigint NormalMul(const bigint& x, const bigint& y) {
		bigint ret = 0;

		for (count_t j = 0; j < Words; ++j) {
			const word_t y_word = y.words()[j];

			if (y_word == 0) {
				continue;
			}

			word_t carry = 0;
			bool carryflag = false;

			for (count_t i = 0; i + j < Words; ++i) {
				word_t temp = carry;

				const auto [lower, upper] = MulBase(
					y_word,
					x.words()[i]
				);

				carryflag = AddBase(&temp, lower, carryflag);

				carry = upper + carryflag;

				carryflag = AddBase(
					std::addressof(ret.words()[i + j]),
					temp,
					false
				);
			}
		}

		return ret;
	}
	static constexpr bigint Karatuba(const bigint& x, const bigint& y) {
		bigint ret = 0;
		
		if (x == 0 || y == 0) {
			return ret;
		}

		count_t nbit = std::max(x.GetNBit(), y.GetNBit());
		count_t halfbits = (nbit + (nbit & 1)) / 2;

		if (halfbits <= WordBits * 2) {
			ret = NormalMul(x, y);
			return ret;
		}

		bigint halfmask = (bigint(1) << halfbits) - 1;

		bigint xl = x; 
		bigint xh = x; 
		bigint yl = y; 
		bigint yh = y; 

		xl &= halfmask;
		xh >>= halfbits;
		yl &= halfmask;
		yh >>= halfbits;
		
		bigint z0 = Karatuba(xl, yl);
		bigint z2 = Karatuba(xh, yh);
		
		xl += xh;
		yl += yh;
		bigint z1 = Karatuba(xl, yl);

		z1 -= z0;
		z1 -= z2;
		ret += z0;
		
		z1 <<= halfbits;
		z2 <<= (2 * halfbits);

		ret += z1;
		ret += z2;
		
		return ret;
	}
	constexpr bigint& AssignMul(bigint src) {
		return *this = NormalMul(*this, src);
	}
	constexpr std::pair<bigint&, bigint> AssignDivMod(bigint src) {
		
		assert(src != 0 && "Division by Zero");

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
	constexpr bigint& AssignLeftShift(count_t c) {
		bits() <<= c;
		return *this;
	}
	constexpr bigint& AssignRightShift(count_t c) {
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
	constexpr bigint& operator<<=(count_t c) { return AssignLeftShift(c); }
	constexpr bigint& operator>>=(count_t c) { return AssignRightShift(c); }
	constexpr friend bigint operator<<(bigint lhs, count_t c) { return lhs.AssignLeftShift(c); }
	constexpr friend bigint operator>>(bigint lhs, count_t c) { return lhs.AssignRightShift(c); }
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
	
	static constexpr char ToUpper(char c) {
		return ('a' <= c && c <= 'z') ?
			(c - ('a' - 'A')) :
			(c);
	}
	static constexpr std::string_view DigitsTable = "0123456789abcdefghijklmnopqrstuvwxyz";
	static constexpr auto DigitsTableUpper = DigitsTable | std::ranges::views::transform([](auto x) { return ToUpper(x); });
	
	static constexpr std::string WordToString(word_t v, int base) {
		std::string ret;
		ret.reserve(WordCharSize);
		while (v != 0) {
			ret.push_back(DigitsTable[v % base]);
			v /= base;
		}
		std::reverse(ret.begin(), ret.end());
		return ret;
	}
	static constexpr word_t CharSpanToWord(std::span<const char> text, int base) {
		auto it = text.begin();
		auto end = text.end();
		word_t ret = 0;

		auto getidx = [&](char c) -> size_t {
			size_t idx = DigitsTable.find(c);
			if (idx != std::string_view::npos) {
				return idx;
			}
			return DigitsTableUpper.find(c);
		};

		for (; it != end; ++it) {
			auto& c = *it;
			size_t idx = getidx(c);
			switch (c) {
			case ' ':
			case ',':
				continue;
			}

			if (idx == std::string_view::npos) {
				break;
			}

			ret = ret * base + idx;
		}
		return ret;
	}
	static constexpr bigint Parse(std::string_view text, int base = 16) {
		
		assert((base >= 2 && base <= 36) && "Invalid base");

		auto proc = text.substr(0, text.find_first_not_of("0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ"));
		auto it = proc.rbegin();
		auto end = proc.rend();
		count_t c = 0;
		bool breaked = false;

		bigint ret;

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

			ret.words()[c] = CharSpanToWord({copybegin.base(), copyend.base()}, base);

			++c;
		} while (c < Words && !breaked);
		
		return ret;
	}
	constexpr std::string ToHex(bool upper = true, bool padding = true) const {
		std::string ret;
		for (auto w : words()) {
			char buffer[WordCharSize + 1]{};
			char* beg = std::begin(buffer);
			char* end = std::end(buffer);

			std::string temp = WordToString(w, 16);
			
			std::reverse(temp.begin(), temp.end());
			temp += std::string(WordCharSize - temp.size(), '0');

			ret += temp;
		}

		std::reverse(ret.begin(), ret.end());

		if (!padding) {
			size_t idx = ret.find_first_not_of('0');
			ret = ret.substr(
				(idx == std::string::npos) ? 
				ret.size() - 1 : idx
			);
		}

		if (upper) {
			std::transform(
				ret.begin(),
				ret.end(),
				ret.begin(),
				ToUpper
			);
		}

		return ret;
	}
	constexpr std::string ToBase64() const {
		constexpr std::string_view list = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

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
	constexpr Cryptgraphy::bytearray ToBytes() const {
		Cryptgraphy::bytearray ret;
		ret.reserve(WordBytes);
		for (size_t i = 0; i < WordBytes; ++i) {
			ret.push_back(*(reinterpret_cast<const Cryptgraphy::byte_t*>(words().data()) + i));  // NOTE: not constexpr
		}
		return ret;
	}
	constexpr std::string ToString(int base = 10, bool upper = true, bool padding = false) const {
		assert((base >= 2 && base <= 36) && "Invalid base");

		word_t word_digits = static_cast<word_t>(WordBits / std::log2(base));
		bigint word_base = bigint(base).Pow(word_digits);

		if constexpr (Words == 1) {

			auto val = std::conditional_t<IsSigned, sword_t, word_t>(*this);

			std::string temp = WordToString(val, base);
			
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

		do {
			word_t mod = static_cast<word_t>(rem.AssignDivMod(word_base).second);

			std::string temp = WordToString(mod, base);

			std::reverse(temp.begin(), temp.end());
			temp += std::string(word_digits - temp.size(), '0');

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
				ToUpper
			);
		}

		return ret;
	}
#ifdef _DEBUG // in Debug Mode
	const char* DebugView() const {
		static thread_local std::string buf;
		buf = ToHex();
		return buf.c_str();
	}
#endif

	/// Internal Resource

	constexpr arr_t& words() noexcept { return *m_words; }
	constexpr const arr_t& words() const noexcept { return *m_words; }
	constexpr bits_t& bits() {
		return *reinterpret_cast<bits_t*>(m_words->data());  // TODO: resolve potential undefined behavior
	}
	constexpr const bits_t& bits() const {
		return *reinterpret_cast<const bits_t*>(m_words->data());  // TODO: resolve potential undefined behavior
	}

private:

	arr_t* m_words = new arr_t();
};

#if 0

///
/// variable-size
///

template<bool _sign>
struct bigint<0, _sign> {
	using count_t = size_t;
	using diff_t = int64_t;
	using word_t = uint64_t;
	using sword_t = int64_t;
	
	static constexpr bool IsSigned = _sign;
	// static constexpr count_t Words = 0;
	static constexpr count_t WordByte = sizeof(word_t);
	static constexpr count_t WordBits = WordByte * 8;
	static constexpr count_t WordCharSize = WordByte * 2;
	// static constexpr count_t WordBytes = Words * WordByte;
	// static constexpr count_t AllBits = Words * WordBits;

	using arr_t = std::vector<word_t>;
	using arr_view = std::span<const word_t>;
	using arr_ref = std::span<word_t>;
	using signed_t = bigint<0, true>;
	using unsigned_t = bigint<0, false>;

	constexpr count_t GetWords() const noexcept {
		return m_words.size();
	}
	constexpr count_t GetWordBytes() const noexcept {
		return GetWords() * WordByte;
	}
	constexpr count_t GetAllBits() const noexcept {
		return GetWords() * WordBits;
	}
	constexpr count_t GetNWord() const noexcept {
		for (count_t i = m_words.size(); i-- > 0;) {
			if (m_words[i] != 0) {
				return i + 1;
			}
		}
		return GetWords();
	}
	constexpr count_t GetNBit() const noexcept {
		count_t idx = GetWords() - 1;
		count_t word_nbit = std::bit_width(m_words[idx]);
		return word_nbit == 0 ? GetAllBits() : idx * WordBits + word_nbit;
	}

	constexpr void Resize(count_t newsize) noexcept {
		m_words.resize(newsize, 0);
	}

	static constexpr bool AddBase(word_t src, word_t *dest, bool carry) noexcept {
		word_t a = *dest;
		word_t b = src + static_cast<word_t>(carry);
		*dest += b;
		return (b < src) || (*dest < a);
	}
	constexpr bigint& AssignAdd(const bigint& rhs) noexcept {
		bool carry = false;
		for (count_t i = 0; i < GetWords(); ++i) {
			carry = AddBase(
				rhs.m_words[i],
				std::addressof(this->m_words[i]),
				carry
			);
		}
		return *this;
	}

	constexpr bigint& AssignNOT() noexcept {
		std::transform(
			std::execution::unseq,
			m_words.begin(),
			m_words.end(),
			m_words.begin(),
			std::bit_not<word_t>()
		);
		return *this;
	}
	constexpr bigint& AssignAND(const bigint& rhs) noexcept {
		std::transform(
			std::execution::unseq,
			m_words.begin(),
			m_words.end(),
			rhs.m_words.begin(),
			m_words.begin(),
			std::bit_and<word_t>()
		);
		return *this;
	}
	constexpr bigint& AssignOR(const bigint& rhs) noexcept {
		std::transform(
			std::execution::unseq,
			m_words.begin(),
			m_words.end(),
			rhs.m_words.begin(),
			m_words.begin(),
			std::bit_or<word_t>()
		);
		return *this;
	}
	constexpr bigint& AssignXOR(const bigint& rhs) noexcept {
		std::transform(
			std::execution::unseq,
			m_words.begin(),
			m_words.end(),
			rhs.m_words.begin(),
			m_words.begin(),
			std::bit_xor<word_t>()
		);
		return *this;
	}
	static constexpr word_t WordShiftBase(word_t low, word_t high, count_t n) noexcept {
		return (low >> n) | (high << (WordBits - n));
	}
	constexpr bigint& AssignLeftShift(count_t n) noexcept {
		count_t wordshift = n >> std::bit_width(WordBits - 1);
		count_t bitshift = n & (WordBits - 1);

		for (count_t i = GetWords() - wordshift; i-- > 0;) {
			m_words[i + wordshift] = WordShiftBase(
				i == 0 ? 0 : m_words[i - 1],
				m_words[i],
				bitshift
			);
		}

		auto offset = std::min(wordshift, GetWords());
		std::fill(std::execution::unseq, m_words.begin(), m_words.begin() + offset, 0);

		return *this;
	}
	constexpr bigint& AssignRightShift(count_t n) noexcept {
		count_t wordshift = n >> std::bit_width(WordBits - 1);
		count_t bitshift = WordBits - (n & (WordBits - 1));

		for (count_t i = wordshift, c = GetWords(); i < c; ++i) {
			m_words[i - wordshift] = WordShiftBase(
				m_words[i],
				m_words[i + 1],
				bitshift
			);
		}

		auto offset = std::min(wordshift, GetWords());
		std::fill(std::execution::unseq, m_words.rbegin(), m_words.rbegin() + offset, 0);

		return *this;
	}

	constexpr bigint& operator~() noexcept {
		return AssignNOT();
	}
	constexpr bigint& operator&=(const bigint& rhs) noexcept {
		return AssignAND(rhs);
	}
	constexpr bigint& operator|=(const bigint& rhs) noexcept {
		return AssignOR(rhs);
	}
	constexpr bigint& operator^=(const bigint& rhs) noexcept {
		return AssignXOR(rhs);
	}

	static constexpr auto Compare(const bigint& lhs, const bigint& rhs) noexcept {
		count_t words[2] = {lhs.GetWords(), rhs.GetWords()};
		auto [words_min, words_max] = std::minmax(words[0], words[1]);
		
		bool is_bigger_l = words_max == words[0];
		
		const bigint& longer = is_bigger_l ? lhs : rhs;
		const bigint& shorter = is_bigger_l ? rhs : lhs;
		
		if (!IsZeroInRef(arr_view(longer.m_words).last(words_max - words_min))) {
			return is_bigger_l ?
				std::strong_ordering::greater : std::strong_ordering::less;
		}
		
		for (count_t i = words_min; i-- > 0;) {
			auto com = lhs.m_words[i] <=> rhs.m_words[i];
			if (!std::is_eq(com)) {
				return com;
			}
		}

		return std::strong_ordering::equal;
	}
	constexpr auto Compare(const bigint& rhs) const noexcept {
		return Compare(*this, rhs);
	}
	friend constexpr auto operator<=>(const bigint& lhs, const bigint& rhs) noexcept {
		return Compare(lhs, rhs);
	}

private:

	static constexpr bool IsZeroInRef(arr_view v) noexcept {
		for (auto&& elem : v) {
			if (elem != 0) {
				return false;
			}
		}
		return true;
	}

	arr_t m_words{};

};

#endif