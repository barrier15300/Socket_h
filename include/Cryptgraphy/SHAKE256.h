#pragma once
#include "common.h"

class SHAKE256 {
public:

	using byte_t = Cryptgraphy::byte_t;
	using bytearray = Cryptgraphy::bytearray;
	using byte_view = Cryptgraphy::byte_view;

	static constexpr size_t r = 1088;
	static constexpr size_t b = 25;
	static constexpr size_t w = 64;
	static constexpr size_t l = std::bit_width(w) - 1;
	static constexpr size_t length = b * w;
	using word_t = uint64_t;
	
	struct state {

		constexpr state() {}
		constexpr state(const bytearray& from) {
			auto it = reinterpret_cast<byte_t*>(m_words.data());       // NOTE: not constexpr
			auto end = reinterpret_cast<byte_t*>(m_words.data() + b);  // NOTE: not constexpr
			for (auto&& c : from) {
				*it = c;
				if (++it == end) {
					break;
				}
			}
		}

		struct reference {
			friend state;

			constexpr reference(word_t* p, size_t z) : ptr(p), idx(z) {}
			constexpr reference(const reference&) = default;
			constexpr reference(reference&&) noexcept = default;

			constexpr reference& operator=(const reference& from) {
				*this = static_cast<bool>(from);
				return *this;
			}
			constexpr reference& operator=(reference&& from) noexcept {
				*this = static_cast<bool>(from);
				return *this;
			}

			constexpr reference& operator=(bool b) {
				if (b) {
					*ptr |= posword();
				}
				else {
					*ptr &= ~posword();
				}
				return *this;
			}
			constexpr explicit operator bool() const {
				return *ptr & posword();
			}
			constexpr bool operator~() const {
				return !*this;
			}
			constexpr void flip() {
				*this = ~(*this);
			}

			constexpr word_t posword() const {
				return (word_t)1 << idx;
			}

			word_t* ptr = nullptr;
			size_t idx = 0;
		};

		constexpr word_t& operator()(size_t x, size_t y) {
			return m_words[GetIdx(x, y)];
		}
		constexpr word_t operator()(size_t x, size_t y) const {
			return m_words[GetIdx(x, y)];
		}

		constexpr reference operator()(size_t x, size_t y, size_t z) {
			return reference(&m_words[GetIdx(x, y)], z);
		}
		constexpr bool operator()(size_t x, size_t y, size_t z) const {
			return (m_words[GetIdx(x, y)] >> z) & 1;
		}

		constexpr operator bytearray() const {
			bytearray ret;
			ret.reserve(sizeof(m_words));
			for (size_t i = 0, c = sizeof(m_words); i < c; ++i) {
				ret.push_back(*(reinterpret_cast<const byte_t*>(m_words.data()) + i));  // NOTE: not constexpr
			}
			return ret;
		}

		constexpr size_t GetIdx(size_t x, size_t y) const {
			return x + 5 * y;
		}

		std::array<word_t, b> m_words{};
	};

	static constexpr bool rc(size_t t) {
		size_t m = t % 255;
		if (m == 0) {
			return true;
		}
		uint8_t R = 0b00000001;
		for (size_t i = 0; i < m; ++i) {
			bool b = R & 0b10000000;
			R <<= 1;
			if (b) { R ^= 0b01110001; };
		}
		return R & 1;
	}

	static constexpr state theta(const state& a) {
		state ret;

		word_t C[5]{};
		word_t D[5]{};

		// step 1
		for (size_t x = 0; x < 5; ++x) {
			C[x] = a(x, 0) ^ a(x, 1) ^ a(x, 2) ^ a(x, 3) ^ a(x, 4);
		}

		// step 2
		for (size_t x = 0; x < 5; ++x) {
			D[x] = C[(x + 4) % 5] ^ std::rotl(C[(x + 1) % 5], 1);
		}

		// step 3
		for (size_t y = 0; y < 5; ++y) {
			for (size_t x = 0; x < 5; ++x) {
				ret(x, y) = a(x, y) ^ D[x];
			}
		}

		return ret;
	}
	static constexpr state rho(const state&& a) {
		state ret;
		
		// step 1
		ret(0, 0) = a(0, 0);

		// step 2
		std::pair pos{1, 0};
		
		// step 3
		for (size_t t = 0; t <= 23; ++t) {
			size_t temp = (((t + 1) * (t + 2)) >> 1) & (w - 1);
			ret(pos.first, pos.second) = std::rotl(a(pos.first, pos.second), temp);
			pos = {
				pos.second,
				(2 * pos.first + 3 * pos.second) % 5
			};
		}

		return ret;
	}
	static constexpr state pi(const state&& a) {
		state ret;

		// step 1
		for (size_t y = 0; y < 5; ++y) {
			for (size_t x = 0; x < 5; ++x) {
				ret(x, y) = a((x + 3 * y) % 5, x);
			}
		}

		return ret;
	}
	static constexpr state chi(const state&& a) {
		state ret;

		// step 1
		for (size_t y = 0; y < 5; ++y) {
			for (size_t x = 0; x < 5; ++x) {
				ret(x, y) = a(x, y) ^ (~a((x + 1) % 5, y) & a((x + 2) % 5, y));
			}
		}

		return ret;
	}
	static constexpr state iota(state&& A, size_t ir) {
		word_t RC = 0;
		
		for (size_t j = 0; j <= l; ++j) {
			size_t idx = ((static_cast<size_t>(1) << j) - 1);
			if (rc(j + 7 * ir)) {
				RC |= (word_t)1 << idx;
			}
			else {
				RC &= ~((word_t)1 << idx);
			}
		}

		A(0, 0) ^= RC;

		return A;
	}
	static constexpr state Round(const state& a, size_t ir) {
		return iota(chi(pi(rho(theta(a)))), ir);
	}

	template<size_t nr>
	static constexpr bytearray KECCAKp(const bytearray& s) {
		state A = s;
		for (size_t i = 12 + 2 * l - nr, c = 12 + 2 * l; i != c; ++i) {
			A = Round(A, i);
		}
		return (bytearray)A;
	}
	static constexpr bytearray SPONGE(const bytearray& N, size_t outlen) {
		constexpr size_t r_8 = r / 8;
		size_t pad = r_8 - (N.size() % r_8);
		
		bytearray p(pad);
		p.front() ^= 0x1f;
		p.back() ^= 0x80;
		
		bytearray P;
		P.reserve(N.size() + p.size());
		P.insert(P.end(), N.begin(), N.end());
		P.insert(P.end(), p.begin(), p.end());

		size_t n = P.size() / r_8;
		constexpr size_t c = 1600 - r;
		bytearray S(1600 / 8);
		
		for (size_t i = 0; i < n; ++i) {
			byte_view t(P.begin() + (i * r_8), r_8);
		
			state pS = S;
			state ps = {{t.begin(), t.end()}};

			for (size_t y = 0; y < 5; ++y) {
				for (size_t x = 0; x < 5; ++x) {
					pS(x, y) ^= ps(x, y);
				}
			}

			S = KECCAKp<24>(pS);
		}
		
		bytearray Z;
		Z.reserve(r_8);
		
		while (Z.size() < outlen) {
			byte_view t(S.begin(), r_8);
			
			Z.insert(Z.end(), t.begin(), t.end());
			
			S = KECCAKp<24>(S);
		}
		
		return bytearray(Z.begin(), Z.begin() + outlen);
	}

	static constexpr bytearray Hasher256(const bytearray& N) {
		return SPONGE(N, 32);
	}
	static constexpr bytearray HasherN(const bytearray& N, size_t bytes) {
		return SPONGE(N, bytes);
	}
};
