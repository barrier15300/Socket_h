#pragma once
#include <type_traits>

#include "MultiWordInt.h"

template<size_t _words, class super = bigint<_words, true>>
struct modint : public super {

	using super::super;

	constexpr modint() = delete;
	constexpr modint(const super& _i, const super& _p) : super(_i), p(_p) {}
	constexpr modint(super&& _i, const super& _p) : super(std::move(_i)), p(_p) {}
	constexpr modint(const super& _i, super&& _p) : super(_i), p(std::move(_p)) {}
	constexpr modint(super&& _i, super&& _p) : super(std::move(_i)), p(std::move(_p)) {}
	constexpr modint(const modint&) = default;
	constexpr modint(modint&&) = default;

	constexpr modint ModPow(const super& x) const {
		modint ret = {1, p};
		modint base = *this;

		auto nbit = x.GetNBit();

		for (auto i = 0; i < nbit; ++i) {
			if (x.BitCheck(i)) {
				ret *= base;
			}
			base *= base;
		}

		return ret;
	}
	constexpr modint Inverse() const {
		return ModPow(p - 2);
	}

	constexpr modint& operator+=(const super& from) {
		if (this->AddOutCheck(from)) {
			(super&)*this %= p;
		}
		return (modint&)((super&)*this += from);
	}
	constexpr modint& operator-=(const super& from) {
		(super&)*this -= from;
		if (this->IsNegative()) {
			(modint&)((super&)*this += p);
		}
		return *this;
	}
	constexpr modint& operator*=(const super& from) {
		if (this->MulOutCheck(from)) {
			(super&)*this %= p;
		}
		return (modint&)((super&)*this *= from);
	}
	constexpr modint& operator/=(const modint& from) {
		auto inv = from.Inverse();
		return *this *= inv;
	}

	constexpr friend modint operator+(const modint& lhs, const modint& rhs) { return modint(lhs) += rhs; }
	constexpr friend modint operator-(const modint& lhs, const modint& rhs) { return modint(lhs) -= rhs; }
	constexpr friend modint operator*(const modint& lhs, const modint& rhs) { return modint(lhs) *= rhs; }
	constexpr friend modint operator/(const modint& lhs, const modint& rhs) { return modint(lhs) /= rhs; }

	super p{};
};


