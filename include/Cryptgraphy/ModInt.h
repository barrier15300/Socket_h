#pragma once
#include <memory>
#include <utility>

#include "NumberSet.h"

template<IntegralSet T>
struct ModInt {

	constexpr ModInt() : value(0), P(SafeMax()) {}
	constexpr ModInt(const T* p) : P(p) {}

	class Factory {
	public:

		constexpr Factory(const T& p) : ptr(new T(p)) {}
		
		constexpr Factory(T&& p) : ptr(new T(std::move(p))) {}

		template<class... Args>
		constexpr Factory(Args&&... args) : ptr(new T(std::forward<Args>(args)...)) {}

		constexpr ModInt operator()() const {
			return ModInt(ptr);
		}
		template<class Tv>
			requires std::convertible_to<Tv, T>
		constexpr ModInt operator()(Tv&& v) const {
			ModInt ret(ptr);
			ret.value = std::forward<Tv>(v);
			ret.value %= ret.GetP();
			return ret;
		}

		template<class... Args>
		constexpr ModInt Make(Args&&... args) const {
			ModInt ret(ptr);
			ret.value = T(std::forward<Args>(args)...);
			ret.value %= ret.GetP();
			return ret;
		}

		constexpr const T& P() const {
			return *ptr;
		}

	private:
		const T* ptr{};
	};

	constexpr ModInt(const ModInt&) = default;
	constexpr ModInt(ModInt&&) = default;

	constexpr ModInt& operator=(const ModInt&) = default;
	constexpr ModInt& operator=(ModInt&&) = default;

	constexpr ModInt& operator=(const T& from) { value = from % GetP(); return *this; }
	constexpr ModInt& operator=(T&& from) { value = from % GetP(); return *this; }
	constexpr ModInt Pow(T n) const {
		Factory make(GetP());
		ModInt base = make(value);
		ModInt ret = make(1);

		do {
			if ((n & 1) == 1) {
				ret *= base;
			}
			base *= base;
		} while ((n >>= 1) != 0);

		return ret;
	}
	constexpr ModInt Sqrt() const {
		Factory make(GetP());
		if (Pow((GetP() - 1) >> 1) != make(1)) {
			return make(0);
		}
		ModInt q = make(GetP() - 1);
		ModInt m = 0;
		while ((q & 1) == make(0)) {
			q >>= 1;
			m += make(1);
		}
		ModInt z = this->Pow(q.value);
		do {
			z = z.Pow(2);
		} while (z.Pow((GetP() - 1) >> 1) != make(1));
		ModInt c = z.Pow(q.value);
		ModInt t = this->Pow(q.value);
		ModInt r = this->Pow(((q + 1) >> 1).value);
		if (t == make(0)) {
			return make(0);
		}
		m -= make(2);
		while (t != make(1)) {
			while (t.Pow(make(2).Pow(m.value).value) == make(1)) {
				c = c.Pow(2);
				m -= make(1);
			}
			r *= c;
			c *= c;
			t *= c;
			m -= make(1);
		}
		return r;
	}
	constexpr ModInt FermerInv() const {
		return Pow(GetP() - 2);
	}

	constexpr bool CheckP(const ModInt& from) const {
		return P == from.P;
	}
	
	constexpr ModInt& operator+=(const ModInt& rhs) {
		if (!CheckP(rhs)) { return *this; }
		value += rhs.value;
		if (value >= GetP()) {
			value %= GetP();
		}
		return *this;
	}
	constexpr ModInt& operator-=(const ModInt& rhs) {
		if (!CheckP(rhs)) { return *this; }
		bool neg = value < rhs.value;
		value -= rhs.value;
		if (neg) {
			value += GetP();
		}
		return *this;
	}
	constexpr ModInt& operator*=(const ModInt& rhs) {
		if (!CheckP(rhs)) { return *this; }
		value *= rhs.value;
		if (value >= GetP()) {
			value %= GetP();
		}
		return *this;
	}
	constexpr ModInt& operator/=(const ModInt& rhs) {
		if (!CheckP(rhs)) { return *this; }
		*this *= rhs.FermerInv();
		return *this;
	}
	constexpr ModInt& operator%=(const ModInt& rhs) {
		if (!CheckP(rhs)) { return *this; }
		value %= rhs.value;
		return *this;
	}

	constexpr ModInt& operator<<=(size_t shift) {
		value <<= shift;
		return *this;
	}
	constexpr ModInt& operator>>=(size_t shift) {
		value >>= shift;
		return *this;
	}

	constexpr ModInt& operator&=(const ModInt& rhs) {
		value &= rhs.value;
		return *this;
	}
	constexpr ModInt& operator|=(const ModInt& rhs) {
		value |= rhs.value;
		return *this;
	}
	constexpr ModInt& operator^=(const ModInt& rhs) {
		value ^= rhs.value;
		return *this;
	}
	
	constexpr ModInt operator-() const {
		Factory make(GetP());
		return make(0) - *this;
	}
	constexpr ModInt operator+() const { return *this; }

	constexpr friend ModInt operator+(ModInt lhs, const ModInt& rhs) { return lhs += rhs; }
	constexpr friend ModInt operator-(ModInt lhs, const ModInt& rhs) { return lhs -= rhs; }
	constexpr friend ModInt operator*(ModInt lhs, const ModInt& rhs) { return lhs *= rhs; }
	constexpr friend ModInt operator/(ModInt lhs, const ModInt& rhs) { return lhs /= rhs; }
	constexpr friend ModInt operator%(ModInt lhs, const ModInt& rhs) { return lhs %= rhs; }
	constexpr friend ModInt operator+(const T& lhs, const ModInt& rhs) { Factory make(rhs.GetP()); return make(lhs) += rhs; }
	constexpr friend ModInt operator-(const T& lhs, const ModInt& rhs) { Factory make(rhs.GetP()); return make(lhs) -= rhs; }
	constexpr friend ModInt operator*(const T& lhs, const ModInt& rhs) { Factory make(rhs.GetP()); return make(lhs) *= rhs; }
	constexpr friend ModInt operator/(const T& lhs, const ModInt& rhs) { Factory make(rhs.GetP()); return make(lhs) /= rhs; }
	constexpr friend ModInt operator%(const T& lhs, const ModInt& rhs) { Factory make(rhs.GetP()); return make(lhs) %= rhs; }
	constexpr friend ModInt operator+(ModInt lhs, const T& rhs) { Factory make(lhs.GetP()); return lhs += make(rhs); }
	constexpr friend ModInt operator-(ModInt lhs, const T& rhs) { Factory make(lhs.GetP()); return lhs -= make(rhs); }
	constexpr friend ModInt operator*(ModInt lhs, const T& rhs) { Factory make(lhs.GetP()); return lhs *= make(rhs); }
	constexpr friend ModInt operator/(ModInt lhs, const T& rhs) { Factory make(lhs.GetP()); return lhs /= make(rhs); }
	constexpr friend ModInt operator%(ModInt lhs, const T& rhs) { Factory make(lhs.GetP()); return lhs %= make(rhs); }
	
	constexpr friend ModInt operator<<(ModInt lhs, size_t shift) { return lhs <<= shift; }
	constexpr friend ModInt operator>>(ModInt lhs, size_t shift) { return lhs >>= shift; }

	constexpr friend ModInt operator&(ModInt lhs, const ModInt& rhs) { return lhs &= rhs; }
	constexpr friend ModInt operator|(ModInt lhs, const ModInt& rhs) { return lhs |= rhs; }
	constexpr friend ModInt operator^(ModInt lhs, const ModInt& rhs) { return lhs ^= rhs; }
	constexpr friend ModInt operator&(const T& lhs, const ModInt& rhs) { Factory make(rhs.GetP()); return make(lhs) &= rhs; }
	constexpr friend ModInt operator|(const T& lhs, const ModInt& rhs) { Factory make(rhs.GetP()); return make(lhs) |= rhs; }
	constexpr friend ModInt operator^(const T& lhs, const ModInt& rhs) { Factory make(rhs.GetP()); return make(lhs) ^= rhs; }
	constexpr friend ModInt operator&(ModInt lhs, const T& rhs) { Factory make(lhs.GetP()); return lhs &= make(rhs); }
	constexpr friend ModInt operator|(ModInt lhs, const T& rhs) { Factory make(lhs.GetP()); return lhs |= make(rhs); }
	constexpr friend ModInt operator^(ModInt lhs, const T& rhs) { Factory make(lhs.GetP()); return lhs ^= make(rhs); }

	constexpr friend bool operator< (const ModInt& lhs, const ModInt& rhs) { return lhs.value < rhs.value; }
	constexpr friend bool operator> (const ModInt& lhs, const ModInt& rhs) { return rhs < lhs; }
	constexpr friend bool operator<=(const ModInt& lhs, const ModInt& rhs) { return !(lhs > rhs); }
	constexpr friend bool operator>=(const ModInt& lhs, const ModInt& rhs) { return !(rhs < lhs); }
	constexpr friend bool operator==(const ModInt& lhs, const ModInt& rhs) { return !(lhs < rhs) && !(lhs > rhs); }
	constexpr friend bool operator!=(const ModInt& lhs, const ModInt& rhs) { return !(lhs == rhs); }
	constexpr friend bool operator< (const T& lhs, const ModInt& rhs) { Factory make(rhs.GetP()); return make(lhs) < rhs; }
	constexpr friend bool operator> (const T& lhs, const ModInt& rhs) { Factory make(rhs.GetP()); return make(lhs) > rhs; }
	constexpr friend bool operator<=(const T& lhs, const ModInt& rhs) { Factory make(rhs.GetP()); return make(lhs) <= rhs; }
	constexpr friend bool operator>=(const T& lhs, const ModInt& rhs) { Factory make(rhs.GetP()); return make(lhs) >= rhs; }
	constexpr friend bool operator==(const T& lhs, const ModInt& rhs) { Factory make(rhs.GetP()); return make(lhs) == rhs; }
	constexpr friend bool operator!=(const T& lhs, const ModInt& rhs) { Factory make(rhs.GetP()); return make(lhs) != rhs; }
	constexpr friend bool operator< (const ModInt& lhs, const T& rhs) { Factory make(lhs.GetP()); return lhs < make(rhs); }
	constexpr friend bool operator> (const ModInt& lhs, const T& rhs) { Factory make(lhs.GetP()); return lhs > make(rhs); }
	constexpr friend bool operator<=(const ModInt& lhs, const T& rhs) { Factory make(lhs.GetP()); return lhs <= make(rhs); }
	constexpr friend bool operator>=(const ModInt& lhs, const T& rhs) { Factory make(lhs.GetP()); return lhs >= make(rhs); }
	constexpr friend bool operator==(const ModInt& lhs, const T& rhs) { Factory make(lhs.GetP()); return lhs == make(rhs); }
	constexpr friend bool operator!=(const ModInt& lhs, const T& rhs) { Factory make(lhs.GetP()); return lhs != make(rhs); }
	
	T value{};

	constexpr const T& GetP() const {
		return *P;
	}

private:

	static T* SafeMax() {
		static T max = ~(T());
		return &max;
	}

	const T* P;
};
