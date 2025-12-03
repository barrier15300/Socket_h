#pragma once
#include "common.h"

template<IntegralSet T>
class WeierstrassParameter {
public:

	template<class Ta, class Tb>
		requires (
			std::same_as<std::decay_t<Ta>, T> &&
			std::same_as<std::decay_t<Tb>, T>
		)
	constexpr WeierstrassParameter(Ta&& a, Tb&& b) : a(a), b(b) {}

	constexpr bool CheckPoint(const T& x, const T& y) const {
		// Check if y^2 == x^3 + a*x + b
		T left = y * y;
		T right = (x * x * x) + (a * x) + b;
		return left == right;
	}

	T a;
	T b;
};

template<IntegralSet T, IntegralSet Twp>
struct ECAfinPoint {
	constexpr ECAfinPoint(const WeierstrassParameter<Twp>& p) : x{}, y{}, param(&p) {}

	class Factory {
	public:

		constexpr Factory(const WeierstrassParameter<Twp>& p) : param(&p) {}
		
		constexpr ECAfinPoint operator()() const {
			return ECAfinPoint(*param);
		}
		template<class Tx, class Ty>
			requires (
				std::same_as<std::decay_t<Tx>, T> &&
				std::same_as<std::decay_t<Ty>, T>
			)
		constexpr ECAfinPoint operator()(Tx&& x, Ty&& y) const {
			ECAfinPoint point(*param);
			point.x = std::forward<Tx>(x);
			point.y = std::forward<Ty>(y);
			return point;
		}

	private:

		const WeierstrassParameter<Twp>* param;
	};

	constexpr ECAfinPoint(const ECAfinPoint&) = default;
	constexpr ECAfinPoint(ECAfinPoint&&) = default;
	
	constexpr ECAfinPoint& operator=(const ECAfinPoint&) = default;
	constexpr ECAfinPoint& operator=(ECAfinPoint&&) = default;
	
	T x;
	T y;

	constexpr ECAfinPoint Double() const {
		Factory make(*param);
		auto temp = (3 * (x * x) + param->a) / (2 * y);
		auto _x = (temp * temp) - (2 * x);
		auto xtemp = x - _x;
		return make(_x, (temp * xtemp) - y);
	}
	constexpr ECAfinPoint Add(const ECAfinPoint& other) const {
		Factory make(*param);
		if (*this == other) { return Double(); }
		auto temp = (other.y - y) / (other.x - x);
		auto _x = (temp * temp) - x - other.x;
		auto xtemp = x - _x;
		return make(_x, (temp * xtemp) - y);
	}
	constexpr ECAfinPoint Scaler(Twp s) {
		Factory make(*param);

		bool first = true;
		ECAfinPoint ret = make();
		ECAfinPoint base = *this;

		do {
			if ((s & 1) == 1) {
				if (first) {
					ret = base;
					first = false;
				}
				else {
					ret = ret.Add(base);
				}
			}
			base = base.Double();
		} while ((s >>= 1) != (int)0);

		return ret;
	}

	constexpr friend bool operator==(const ECAfinPoint& lhs, const ECAfinPoint& rhs) {
		return lhs.x == rhs.x && lhs.y == rhs.y;
	}
	constexpr friend bool operator!=(const ECAfinPoint& lhs, const ECAfinPoint& rhs) {
		return !(lhs == rhs);
	}

private:
	const WeierstrassParameter<Twp>* param;
};

template<IntegralSet T, IntegralSet _Twp = void>
struct ECProject {
private:
	constexpr ECProject(const WeierstrassParameter<_Twp>& p) : param(&p) {}
public:

	template<IntegralSet Twp>
	class Factory {
	public:
		constexpr Factory(const WeierstrassParameter<Twp>& p) : param(&p) {}
		
		constexpr ECProject<T, Twp> operator()() const {
			return ECProject<T, Twp>(*param);
		}
		template<class Tx, class Ty, class Tz>
			requires (
				std::same_as<std::decay_t<Tx>, T> &&
				std::same_as<std::decay_t<Ty>, T> &&
				std::same_as<std::decay_t<Tz>, T>
			)
		constexpr ECProject<T, Twp> operator()(Tx&& x, Ty&& y, Tz&& z) const {
			ECProject<T, Twp> point(*param);
			point.x = std::forward<Tx>(x);
			point.y = std::forward<Ty>(y);
			point.z = std::forward<Tz>(z);
			return point;
		}

	private:

		const WeierstrassParameter<Twp>* param;
	};

	constexpr ECProject(const ECProject&) = default;
	constexpr ECProject(ECProject&&) = default;

	constexpr ECProject& operator=(const ECProject&) = default;
	constexpr ECProject& operator=(ECProject&&) = default;

	T x;
	T y;
	T z;

private:
	const WeierstrassParameter<_Twp>* param;
};
