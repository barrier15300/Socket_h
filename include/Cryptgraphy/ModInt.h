#pragma once
#include <memory>
#include <utility>

#include "NumberSet.h"

template<IntegralSet T>
struct ModInt {
private:
	constexpr ModInt(const T* p) : P(p) {}
public:

	constexpr ModInt() : value(0), P(SafeMax()) {}

	class Factory {
	public:

		constexpr Factory(const T& p) : ptr(&p) {}

		constexpr ModInt operator()() const {
			return ModInt(ptr);
		}
		template<class Tv>
			requires std::convertible_to<std::decay_t<Tv>, T>
		constexpr ModInt operator()(Tv&& v) const {
			ModInt ret(ptr);
			ret.value = std::forward<Tv>(v);
			ret.value %= ret.GetP();
			return ret;
		}

	private:
		const T* ptr{};
	};

	constexpr ModInt(const ModInt&) = default;
	constexpr ModInt(ModInt&&) = default;

	constexpr ModInt& operator=(const ModInt&) = default;
	constexpr ModInt& operator=(ModInt&&) = default;

	constexpr ModInt& operator=(const T& from) { value = from % GetP(); return *this; }
	constexpr ModInt& operator=(T&& from) { value = std::move(from); value %= GetP(); return *this; }
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
	constexpr ModInt FermerInv() const {
		return Pow(GetP() - 2);
	}

	constexpr ModInt& operator+=(const ModInt& rhs) {
		value += rhs.value;
		value %= GetP();
		return *this;
	}
	constexpr ModInt& operator-=(const ModInt& rhs) {
		bool neg = value < rhs.value;
		value -= rhs.value;
		if (neg) {
			value += GetP();
		}
		value %= GetP();
		return *this;
	}
	constexpr ModInt& operator*=(const ModInt& rhs) {
		value *= rhs.value;
		value %= GetP();
		return *this;
	}
	constexpr ModInt& operator/=(const ModInt& rhs) {
		*this *= rhs.FermerInv();
		value %= GetP();
		return *this;
	}
	constexpr ModInt& operator%=(const ModInt& rhs) {
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
	constexpr friend ModInt operator+(const T& lhs, const ModInt& rhs) { Factory make(*rhs.P); return make(lhs) += rhs; }
	constexpr friend ModInt operator-(const T& lhs, const ModInt& rhs) { Factory make(*rhs.P); return make(lhs) -= rhs; }
	constexpr friend ModInt operator*(const T& lhs, const ModInt& rhs) { Factory make(*rhs.P); return make(lhs) *= rhs; }
	constexpr friend ModInt operator/(const T& lhs, const ModInt& rhs) { Factory make(*rhs.P); return make(lhs) /= rhs; }
	constexpr friend ModInt operator%(const T& lhs, const ModInt& rhs) { Factory make(*rhs.P); return make(lhs) %= rhs; }
	constexpr friend ModInt operator+(ModInt lhs, const T& rhs) { Factory make(*lhs.P); return lhs += make(rhs); }
	constexpr friend ModInt operator-(ModInt lhs, const T& rhs) { Factory make(*lhs.P); return lhs -= make(rhs); }
	constexpr friend ModInt operator*(ModInt lhs, const T& rhs) { Factory make(*lhs.P); return lhs *= make(rhs); }
	constexpr friend ModInt operator/(ModInt lhs, const T& rhs) { Factory make(*lhs.P); return lhs /= make(rhs); }
	constexpr friend ModInt operator%(ModInt lhs, const T& rhs) { Factory make(*lhs.P); return lhs %= make(rhs); }
	
	constexpr friend ModInt operator<<(ModInt lhs, size_t shift) { return lhs <<= shift; }
	constexpr friend ModInt operator>>(ModInt lhs, size_t shift) { return lhs >>= shift; }

	constexpr friend ModInt operator&(ModInt lhs, const ModInt& rhs) { return lhs &= rhs; }
	constexpr friend ModInt operator|(ModInt lhs, const ModInt& rhs) { return lhs |= rhs; }
	constexpr friend ModInt operator^(ModInt lhs, const ModInt& rhs) { return lhs ^= rhs; }
	constexpr friend ModInt operator&(const T& lhs, const ModInt& rhs) { Factory make(*rhs.P); return make(lhs) &= rhs; }
	constexpr friend ModInt operator|(const T& lhs, const ModInt& rhs) { Factory make(*rhs.P); return make(lhs) |= rhs; }
	constexpr friend ModInt operator^(const T& lhs, const ModInt& rhs) { Factory make(*rhs.P); return make(lhs) ^= rhs; }
	constexpr friend ModInt operator&(ModInt lhs, const T& rhs) { Factory make(*lhs.P); return lhs &= make(rhs); }
	constexpr friend ModInt operator|(ModInt lhs, const T& rhs) { Factory make(*lhs.P); return lhs |= make(rhs); }
	constexpr friend ModInt operator^(ModInt lhs, const T& rhs) { Factory make(*lhs.P); return lhs ^= make(rhs); }

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
