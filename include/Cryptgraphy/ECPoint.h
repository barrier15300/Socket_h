#pragma once
#include "common.h"

template<IntegralSet T>
class WeierstrassParameter {
public:

	template<class Ta, class Tb>
		requires (
			std::convertible_to<Ta, T> &&
			std::convertible_to<Tb, T>
		)
	constexpr WeierstrassParameter(Ta&& a, Tb&& b) : a(a), b(b) {}

	constexpr bool CheckPoint(const T& x, const T& y) const {
		// Check if y^2 == x^3 + a*x + b
		T left = y * y;
		T right = (x * x * x) + (a * x) + b;
		return left == right;
	}

	constexpr T GetY(const T& x) const {
		return (x * x * x) + (a * x) + b;
	}

	T a;
	T b;
};

template<IntegralSet T>
struct ECAffinPoint {
	constexpr ECAffinPoint(const WeierstrassParameter<T>* p) : x{}, y{}, param(p) {}

	class Factory {
	public:

		constexpr Factory(const WeierstrassParameter<T>& p) : ptr(&p) {}
		
		constexpr Factory(WeierstrassParameter<T>&& p) { ptr = new WeierstrassParameter<T>(std::move(p)); }

		constexpr ECAffinPoint operator()() const {
			return ECAffinPoint(ptr);
		}
		template<class Tx, class Ty>
			requires (
				std::convertible_to<Tx, T> &&
				std::convertible_to<Ty, T>
			)
		constexpr ECAffinPoint operator()(Tx&& x, Ty&& y) const {
			ECAffinPoint point(ptr);
			point.x = std::forward<Tx>(x);
			point.y = std::forward<Ty>(y);
			return point;
		}

		template<class... xArgs, class... yArgs>
			requires (
				(sizeof...(xArgs) > 0) &&
				(sizeof...(yArgs) > 0) &&
				std::constructible_from<T, xArgs...> &&
				std::constructible_from<T, yArgs...>
			)
		constexpr ECAffinPoint Make(std::tuple<xArgs&&...> xargs, std::tuple<yArgs&&...> yargs) const {
			ECAffinPoint point(ptr);
			auto xctor = [&](xArgs... _xargs) -> T { return T(std::forward<xArgs>(_xargs)...); };
			auto yctor = [&](yArgs... _yargs) -> T { return T(std::forward<yArgs>(_yargs)...); };
			point.x = std::apply(xctor, xargs);
			point.y = std::apply(yctor, yargs);
			return point;
		}

		constexpr const WeierstrassParameter<T>& Param() const {
			return *ptr;
		}

	private:

		const WeierstrassParameter<T>* ptr;
	};

	constexpr ECAffinPoint(const ECAffinPoint&) = default;
	constexpr ECAffinPoint(ECAffinPoint&&) = default;
	
	constexpr ECAffinPoint& operator=(const ECAffinPoint&) = default;
	constexpr ECAffinPoint& operator=(ECAffinPoint&&) = default;
	
	T x;
	T y;

	constexpr const WeierstrassParameter<T>& GetParam() const {
		return *param;
	}

	constexpr ECAffinPoint Double() const {
		Factory make(GetParam());
		auto temp = (3 * (x * x) + GetParam().a) / (2 * y);
		auto _x = (temp * temp) - (2 * x);
		auto xtemp = x - _x;
		return make(_x, (temp * xtemp) - y);
	}
	constexpr ECAffinPoint Add(const ECAffinPoint& other) const {
		Factory make(GetParam());
		if (*this == other) { return Double(); }
		auto temp = (other.y - y) / (other.x - x);
		auto _x = (temp * temp) - x - other.x;
		auto xtemp = x - _x;
		return make(_x, (temp * xtemp) - y);
	}
	constexpr ECAffinPoint Scaler(T s) const {
		Factory make(GetParam());

		bool first = true;
		ECAffinPoint ret = make();
		ECAffinPoint base = *this;

		do {
			if ((s & 1) == (int)1) {
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

	constexpr friend bool operator==(const ECAffinPoint& lhs, const ECAffinPoint& rhs) {
		return lhs.x == rhs.x && lhs.y == rhs.y;
	}
	constexpr friend bool operator!=(const ECAffinPoint& lhs, const ECAffinPoint& rhs) {
		return !(lhs == rhs);
	}

private:
	const WeierstrassParameter<T>* param;
};

template<IntegralSet T>
struct ECProject {
	constexpr ECProject(const WeierstrassParameter<T>* p) : param(p) {}

	class Factory {
	public:
		constexpr Factory(const WeierstrassParameter<T>& p) : ptr(&p) {}
		
		constexpr Factory(WeierstrassParameter<T>&& p) { ptr = new WeierstrassParameter<T>(std::move(p)); }


		constexpr ECProject operator()() const {
			return ECProject(ptr);
		}
		template<class Tx, class Ty, class Tz>
			requires (
				std::convertible_to<Tx, T> &&
				std::convertible_to<Ty, T> &&
				std::convertible_to<Tz, T>
			)
		constexpr ECProject operator()(Tx&& x, Ty&& y, Tz&& z) const {
			ECProject point(ptr);
			point.x = std::forward<Tx>(x);
			point.y = std::forward<Ty>(y);
			point.z = std::forward<Tz>(z);
			return point;
		}

		template<class... xArgs, class... yArgs, class... zArgs>
			requires (
				(sizeof...(xArgs) > 0) &&
				(sizeof...(yArgs) > 0) &&
				(sizeof...(zArgs) > 0) &&
				std::constructible_from<T, xArgs...> &&
				std::constructible_from<T, yArgs...> &&
				std::constructible_from<T, zArgs...>
			)
		constexpr ECProject Make(std::tuple<xArgs&&...> xargs, std::tuple<yArgs&&...> yargs, std::tuple<zArgs&&...> zargs) const {
			ECProject point(ptr);
			auto xctor = [&](xArgs... _xargs) -> T { return T(std::forward<xArgs>(_xargs)...); };
			auto yctor = [&](yArgs... _yargs) -> T { return T(std::forward<yArgs>(_yargs)...); };
			auto zctor = [&](zArgs... _zargs) -> T { return T(std::forward<zArgs>(_zargs)...); };
			point.x = std::apply(xctor, xargs);
			point.y = std::apply(yctor, yargs);
			point.z = std::apply(zctor, zargs);
			return point;
		}

		constexpr const WeierstrassParameter<T>& Param() const {
			return *ptr;
		}

	private:

		const WeierstrassParameter<T>* ptr;
	};

	constexpr ECProject(const ECProject&) = default;
	constexpr ECProject(ECProject&&) = default;

	constexpr ECProject(const ECAffinPoint<T>& from) : ECProject(ECProject::Factory(from.GetParam())(from.x, from.y, from.y)) {
		z = 1;
	}
	constexpr ECProject(ECAffinPoint<T>&& from) : ECProject(ECProject::Factory(from.GetParam())(std::move(from.x), std::move(from.y), from.y)) {
		z = 1;
	}

	constexpr ECProject& operator=(const ECProject&) = default;
	constexpr ECProject& operator=(ECProject&&) = default;

	constexpr ECProject& operator=(const ECAffinPoint<T>& from) {
		return *this = ECProject(from);
	}
	constexpr ECProject& operator=(ECAffinPoint<T>&& from) {
		return *this = ECProject(std::move(from));
	}

	T x;
	T y;
	T z;

	constexpr const WeierstrassParameter<T>& GetParam() const {
		return *param;
	}
	constexpr ECAffinPoint<T> ToAffinPoint() const {
		typename ECAffinPoint<T>::Factory make(GetParam());
		return make(x / z, y / z);
	}

	constexpr ECProject Double() const {
		Factory make(GetParam());

		auto x_sq = x * x;
		auto y_sq = y * y;
		auto z_sq = z * z;

		auto w = GetParam().a * z_sq + x_sq * 3;
		auto s = y * z;
		auto B = x * y * s;
		auto h = w * w - B * 8;
		auto sp2 = s * s;
		auto sp3 = sp2 * s;

		return make(
			h * s * 2,
			w * (4 * B - h) - y_sq * sp2 * 8,
			sp3 * 8
		);
	}

	constexpr ECProject Add(const ECProject& from) const {
		Factory make(GetParam());

		if (*this == from) { return Double(); }

		auto u1 = from.y * z;
		auto u2 = y * from.z;
		auto v1 = from.x * z;
		auto v2 = x * from.z;

		auto u = u1 - u2;
		auto v = v1 - v2;
		auto w = z * from.z;

		auto vp2 = v * v;
		auto vp3 = vp2 * v;

		auto A = (u * u * w - vp3) - 2 * vp2 * v2;

		return make(
			v * A,
			u * (vp2 * v2 - A) - vp3 * u2,
			vp3 * w
		);
	}

	constexpr ECProject Scaler(T s) const {
		Factory make(GetParam());

		bool first = true;
		ECProject ret = make();
		ECProject base = *this;

		do {
			if ((s & 1) == (int)1) {
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

	constexpr friend bool operator==(const ECProject& lhs, const ECProject& rhs) {
		return lhs.x == rhs.x && lhs.y == rhs.y && lhs.z == rhs.z;
	}
	constexpr friend bool operator!=(const ECProject& lhs, const ECProject& rhs) {
		return !(lhs == rhs);
	}

private:
	const WeierstrassParameter<T>* param;
};
