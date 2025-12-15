#pragma once
#include "common.h"

class RandomGenerator {
public:

	uint32_t Next() {
		gen.seed(rd());
		return gen();
	}

	Cryptgraphy::bytearray NextBytes(size_t size) {
		constexpr size_t blockunit = sizeof(decltype(gen()));
		Cryptgraphy::bytearray ret(size);
		
		if (size == 0) { return ret; }
		
		auto it = ret.begin();
		auto end = ret.end();
		
		gen.seed(rd());

		do {
			auto diff = end - it;
			size_t write = (diff > blockunit) ? blockunit : diff;
		
			uint32_t val = gen();
			std::memcpy(std::to_address(it), &val, write);
			it += write;
		} while (it != end);

		return ret;
	}

private:

	std::mt19937 gen{};
	std::random_device rd;

};
