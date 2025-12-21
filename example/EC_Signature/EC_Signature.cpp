#include "include/Socket.h"

int main(int argc, char* argv[]) {

	KeyManager key;
	std::string message = "I have skill is write low level programing language.";

	auto q = ECDSA::MakePublicKey(key.GetSecretKey());

	auto v = ECDSA::Sign(key.GetSecretKey(), {message.begin(), message.end()});

	bool ret = ECDSA::Verify(q, v, {message.begin(), message.end()});

	std::cout << "message: \"" << message << "\"" << std::endl;
	std::cout << "Q: {" << q.x.value.ToString(16) << ", " << q.y.value.ToString(16) << "}" << std::endl;
	std::cout << "(r, s)(bytes): ";
	for (auto&& b : v) {
		std::cout << std::hex << std::setw(2) << std::setfill('0') << std::right << (int)b;
	}
	std::cout << std::endl;

	std::cout << std::boolalpha << ret;

	return 0;
}