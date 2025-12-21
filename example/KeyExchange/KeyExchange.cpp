#include "include/Cryptgraphy/KeyManager.h"

int main(int argc, char* argv[]) {

	KeyManager Keya;
	KeyManager Keyb;
	
	auto tp = std::chrono::high_resolution_clock::now();
	
	auto kE = Keya.MakeQKey();
	auto kF = Keyb.MakeQKey();
	
	auto Ga = Keya.MakeSharedKey(kF);
	auto Gb = Keyb.MakeSharedKey(kE);
	
	auto ns = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::high_resolution_clock::now() - tp).count();
	
	bool same = Ga == Gb;
	
	std::cout << (double)ns / 1000 / 1000 / 1000 << "s" << std::endl;
	std::cout << std::boolalpha << "shared key same: " << same << std::endl;
	
	for (auto&& b : Ga) {
		std::cout << std::hex << std::setw(2) << std::setfill('0') << std::right << (int)b;
	}
	std::cout << std::endl;
	
	for (auto&& b : Gb) {
		std::cout << std::hex << std::setw(2) << std::setfill('0') << std::right << (int)b;
	}
	std::cout << std::endl;

	return 0;
}