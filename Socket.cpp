#include <deque>
#include <iomanip>
#include <chrono>
#include <map>

#include "include/Socket.h"
#include "include/Cryptgraphy/KeyManager.h"

int main(int argc, char* argv[]) {
	
	using int_t = bigint<2048>;

	int_t a = 0x123456789abcdef0;
	a = a.Pow(0xffffffff);
	int_t b = a.Pow(0xffffffff);

	auto time = [&]() {
		return std::chrono::high_resolution_clock::now();
	};

	std::cout << "condition:" << std::endl;
	std::cout << "max digits: 2^" << int_t::AllBits << std::endl;
	std::cout << "a = 0x123456789abcdef0^0xffffffff" << std::endl;
	std::cout << "b = a^0xffffffff" << std::endl;
	std::cout << "ret = a * b" << std::endl << std::endl;

	auto tp = time();

	int_t ret = int_t::NormalMul(a, b);

	double t = std::chrono::duration_cast<std::chrono::nanoseconds>(time() - tp).count();

	std::cout << "Normal: " << t / 1000 << "us" << std::endl;
	//std::cout << ret.ToString(16, true, false) << std::endl;

	tp = time();

	ret = int_t::Karatuba(a, b);

	t = std::chrono::duration_cast<std::chrono::nanoseconds>(time() - tp).count();

	std::cout << "Karatuba: " << t / 1000 << "us" << std::endl;
	//std::cout << ret.ToString(16, true, false) << std::endl;

	// before: 2.0759ms
	// after : 

	return 0;
}
