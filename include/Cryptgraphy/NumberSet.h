#pragma once
#include <concepts>

#include <stddef.h>

template<class T>
concept RealNumberSet = requires(T a, T b) {
	a += b;
	a -= b;
	a *= b;
	a /= b;
	a %= b;

	a + b;
	a - b;
	a * b;
	a / b;
	a % b;

	a < b;
	a > b;
	a <= b;
	a >= b;
	a == b;
	a != b;

	-a;
	+a;
};

template<class T>
concept IntegralSet = RealNumberSet<T> && requires(T a, T b, size_t n) {
	a & b;
	a | b;
	a ^ b;
	a << n;
	a >> n;
	a <<= n;
	a >>= n;
};
