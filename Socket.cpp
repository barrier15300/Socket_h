

#include <iostream>
#include <deque>
#include <iomanip>
#include <chrono>

#include "include/Socket.h"
#include "include/AES128.h"
#include "include/MultiWordInt.h"
#include "include/ModInt.h"

void Server();
void Client();

int main(int argc, char* argv[]) {
	
	modint<4> mi(1, 7);

	AES128 aes;
	
	AES128::cbytearray<16> key{};
	AES128::cbytearray<16> iv{};

	std::fill(iv.begin(), iv.end(), '\xaa');

	aes.Init(key);
	aes.Initializer(iv);

	size_t len = 1 << 7;
	std::string s = std::string(len, '\x55');
	//std::string s = "In a quiet corner of the digital realm, streams of encrypted thoughts flow endlessly, guarded by elegant ciphers born of pure logic.";
	AES128::bytearray message{s.begin(), s.end()};
	
	auto chip = aes.CBCEncrypt(message);

	if (chip) {
		for (auto b : *chip) {
			std::cout << std::setw(2) << std::setfill('0') << std::hex << std::right << std::uppercase << (int)b;
		}
		std::cout << std::endl << std::endl;
	}
	else {
		return -1;
	}
	
	auto p = aes.CBCDecrypt(*chip);

	if (p) {
		for (auto b : *p) {
			std::cout << std::setw(2) << std::setfill('0') << std::hex << std::right << std::uppercase << (int)b;
		}
		std::cout << std::endl << std::endl;
	}

	return 0;
}

void Server() {

	TCPServerV6 server(8080);

	std::vector<TCPSocketV6> clients;
	std::deque<TCPSocketV6*> lostqueue;

	while (true) {
		auto sock = server.Accept();

		if (sock) {
			std::cout << "connected: " << sock->GetPeerAddress()->Address() << std::endl;
			clients.push_back(std::move(*sock));
		}

		for (auto&& c : clients) {
			if (c.LostConnection()) {
				lostqueue.push_back(&c);
				std::cout << "lost connection: " << c.GetPeerAddress()->Address() << std::endl;
			}
		}

		while (!lostqueue.empty()) {
			auto p = lostqueue.front();
			lostqueue.pop_front();

			clients.erase(std::remove(clients.begin(), clients.end(), *p), clients.end());
		}

		for (auto&& c : clients) {
			
			int available = c.Available();
			int val = 0;
			
			if (available > 0) {
				c._Recv<int>(val);
				auto address = c.GetPeerAddress();
				std::cout << "recived from (";
				if (address) {
					std::cout << address->Address();
				}
				std::cout << "):" << val << std::endl;
				
				++val;
				c._Send<int>(val);
			}
		}
	}
}

void Client() {

	TCPSocketV6 server;

	if (server.Connect(IPv6Address::Loopback().Port(8080))) {
		std::cout << "connected server." << std::endl;
	}
	else {
		std::cout << "can't connect server." << std::endl;
		return;
	}

	while (true) {
		if (server.LostConnection()) {
			break;
		}
		
		int available = server.Available();
		int reciveval = 0;

		int sendval = 0;

		std::cout << "input sendval: ";
		std::cin >> sendval;

		server._Send<int>(sendval);

		server._Recv<int>(reciveval);
		std::cout << "recived from server:" << reciveval << std::endl;
	}
}
