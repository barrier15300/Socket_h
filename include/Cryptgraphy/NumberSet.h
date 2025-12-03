#pragma once
#include <concepts>

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
concept IntegralSet = RealNumberSet<T> && requires(T a, T b) {
	a & b;
	a | b;
	a ^ b;
	a << (size_t)1;
	a >> (size_t)1;
	a <<= (size_t)1;
	a >>= (size_t)1;
};