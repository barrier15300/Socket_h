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
struct ECAffin {

	using param_t = WeierstrassParameter<T>;
	using ptr_t = std::shared_ptr<const param_t>;

	explicit constexpr ECAffin(ptr_t p) : x{}, y{}, param(p) {}

	class Factory {
	public:

		explicit constexpr Factory(ptr_t p) : ptr(p) {}
		
		constexpr Factory(const param_t& p) : ptr(new param_t(p)) {}
		constexpr Factory(param_t&& p) : ptr(new param_t(std::move(p))) {}

		constexpr ECAffin operator()() const {
			return ECAffin(ptr);
		}
		template<class Tx, class Ty>
			requires (
				std::convertible_to<Tx, T> &&
				std::convertible_to<Ty, T>
			)
		constexpr ECAffin operator()(Tx&& x, Ty&& y) const {
			ECAffin point(ptr);
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
		constexpr ECAffin Make(std::tuple<xArgs&&...> xargs, std::tuple<yArgs&&...> yargs) const {
			ECAffin point(ptr);
			auto xctor = [&](xArgs... _xargs) -> T { return T(std::forward<xArgs>(_xargs)...); };
			auto yctor = [&](yArgs... _yargs) -> T { return T(std::forward<yArgs>(_yargs)...); };
			point.x = std::apply(xctor, xargs);
			point.y = std::apply(yctor, yargs);
			return point;
		}

		constexpr const param_t& Param() const {
			return *ptr;
		}

	private:

		ptr_t ptr;
	};

	constexpr ECAffin(const ECAffin&) = default;
	constexpr ECAffin(ECAffin&&) = default;
	
	constexpr ECAffin& operator=(const ECAffin&) = default;
	constexpr ECAffin& operator=(ECAffin&&) = default;
	
	T x;
	T y;

	constexpr bool IsInf() const {
		return x == 0 && y == 0;
	}

	constexpr const param_t& GetParam() const {
		return *param;
	}
	constexpr ptr_t GetParamPtr() const {
		return param;
	}

	constexpr ECAffin Double() const {
		Factory make(param);
		
		if (IsInf()) { return make(); }

		auto temp = (3 * (x * x) + GetParam().a) / (2 * y);
		auto _x = (temp * temp) - (2 * x);
		auto xtemp = x - _x;
		
		return make(_x, (temp * xtemp) - y);
	}
	constexpr ECAffin Add(const ECAffin& other) const {
		Factory make(param);
		
		if (*this == other) { return Double(); }
		if (IsInf()) { return other; }
		else if (other.IsInf()) { return *this; }
		
		auto temp = (other.y - y) / (other.x - x);
		auto _x = (temp * temp) - x - other.x;
		auto xtemp = x - _x;
		
		return make(_x, (temp * xtemp) - y);
	}
	constexpr ECAffin Scaler(T s) const {
		Factory make(param);

		auto ret = make();
		auto base = *this;

		do {
			if ((s & 1) == 1) {
				ret = ret.Add(base);
			}
			base = base.Double();
		} while ((s >>= 1) != 0);

		return ret;
	}

	constexpr friend bool operator==(const ECAffin& lhs, const ECAffin& rhs) {
		return lhs.x == rhs.x && lhs.y == rhs.y;
	}
	constexpr friend bool operator!=(const ECAffin& lhs, const ECAffin& rhs) {
		return !(lhs == rhs);
	}

private:
	ptr_t param;
};

template<IntegralSet T>
struct ECProjective {

	using param_t = WeierstrassParameter<T>;
	using ptr_t = std::shared_ptr<const param_t>;

	explicit constexpr ECProjective(ptr_t p) : x{}, y{}, z{}, param(p) {}

	class Factory {
	public:
		explicit constexpr Factory(ptr_t p) : ptr(p) {}
		
		constexpr Factory(const param_t& p) : ptr(new param_t(p)) {}
		constexpr Factory(param_t&& p) : ptr(new param_t(std::move(p))) {}

		constexpr ECProjective operator()() const {
			return ECProjective(ptr);
		}
		template<class Tx, class Ty, class Tz>
			requires (
				std::convertible_to<Tx, T> &&
				std::convertible_to<Ty, T> &&
				std::convertible_to<Tz, T>
			)
		constexpr ECProjective operator()(Tx&& x, Ty&& y, Tz&& z) const {
			ECProjective point(ptr);
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
		constexpr ECProjective Make(std::tuple<xArgs&&...> xargs, std::tuple<yArgs&&...> yargs, std::tuple<zArgs&&...> zargs) const {
			ECProjective point(ptr);
			auto xctor = [&](xArgs... _xargs) -> T { return T(std::forward<xArgs>(_xargs)...); };
			auto yctor = [&](yArgs... _yargs) -> T { return T(std::forward<yArgs>(_yargs)...); };
			auto zctor = [&](zArgs... _zargs) -> T { return T(std::forward<zArgs>(_zargs)...); };
			point.x = std::apply(xctor, xargs);
			point.y = std::apply(yctor, yargs);
			point.z = std::apply(zctor, zargs);
			return point;
		}

		constexpr const param_t& Param() const {
			return *ptr;
		}

	private:

		ptr_t ptr;
	};

	constexpr ECProjective(const ECProjective&) = default;
	constexpr ECProjective(ECProjective&&) = default;

	constexpr ECProjective(const ECAffin<T>& from) : x(from.x), y(from.y), param(from.GetParamPtr()) {
		z = x / x;
	}
	constexpr ECProjective(ECAffin<T>&& from) : x(from.x), y(from.y), param(from.GetParamPtr()) {
		z = x / x;
	}

	constexpr ECProjective& operator=(const ECProjective&) = default;
	constexpr ECProjective& operator=(ECProjective&&) = default;

	constexpr ECProjective& operator=(const ECAffin<T>& from) {
		return *this = ECProjective(from);
	}
	constexpr ECProjective& operator=(ECAffin<T>&& from) {
		return *this = ECProjective(std::move(from));
	}

	T x;
	T y;
	T z;

	constexpr bool IsInf() const {
		return x == 0 && y == 0 && z == 0;
	}

	constexpr const param_t& GetParam() const {
		return *param;
	}
	constexpr ptr_t GetParamPtr() const {
		return param;
	}
	constexpr ECAffin<T> ToAffin() const {
		typename ECAffin<T>::Factory make(param);
		return make(x / z, y / z);
	}

	constexpr ECProjective Double() const {
		Factory make(param);

		if (IsInf()) {
			return make();
		}

		auto x_sq = x * x;
		auto y_sq = y * y;
		auto z_sq = z * z;

		auto w = GetParam().a * z_sq + 3 * x_sq;
		auto s = y * z;
		auto B = x * y * s;
		auto h = w * w - 8 * B;
		auto sp2 = s * s;
		auto sp3 = sp2 * s;

		return make(
			2 * h * s,
			w * (4 * B - h) - 8 * y_sq * sp2,
			8 * sp3
		);
	}

	constexpr ECProjective Add(const ECProjective& other) const {
		Factory make(param);

		if (*this == other) { return Double(); }
		if (IsInf()) { return other; }
		else if (other.IsInf()) { return *this; }

		auto u1 = other.y * z;
		auto u2 = y * other.z;
		auto v1 = other.x * z;
		auto v2 = x * other.z;

		auto u = u1 - u2;
		auto v = v1 - v2;
		auto w = z * other.z;

		auto vp2 = v * v;
		auto vp3 = vp2 * v;

		auto A = (u * u * w - vp3) - 2 * vp2 * v2;

		return make(
			v * A,
			u * (vp2 * v2 - A) - vp3 * u2,
			vp3 * w
		);
	}

	constexpr ECProjective Scaler(T s) const {
		Factory make(param);

		auto ret = make();
		auto base = *this;

		//size_t nbit = 0;
		//auto t = s;
		//while (t != (int)0) {
		//	++nbit;
		//	t >>= 1;
		//}
		//
		//if (nbit == 0) {
		//	return make();
		//}
		//
		//t = s / s;
		//t <<= nbit - 2;
		//
		//for (size_t i = 1; i < nbit; ++i) {
		//	ret = ret.Double();
		//	if ((s & t) != (int)0) {
		//		ret = ret.Add(base);
		//	}
		//	t >>= 1;
		//}

		while (s != 0) {
			if ((s & 1) ==1) {
				ret = ret.Add(base);
			}
			base = base.Double();
			s >>= 1;
		}

		return ret;
	}

	constexpr friend bool operator==(const ECProjective& lhs, const ECProjective& rhs) {
		return lhs.x == rhs.x && lhs.y == rhs.y && lhs.z == rhs.z;
	}
	constexpr friend bool operator!=(const ECProjective& lhs, const ECProjective& rhs) {
		return !(lhs == rhs);
	}

private:
	ptr_t param;
};
