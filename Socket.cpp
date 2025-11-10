

#include <iostream>
#include <deque>
#include <iomanip>
#include <chrono>

#include "include/MultiWordInt.h"
#include "include/ModInt.h"


#include "include/Socket.h"

void Server();
void Client();

struct ClientData {

	int Level = 0;
	std::string Name = "NoName";

	Packet::buf_t ToBytes() const {
		Packet::buf_t ret;
		Packet::StoreBytes(ret, Level);
		uint32_t len = Name.size();
		Packet::StoreBytes(ret, len);
		Packet::StoreBytes(ret, Name.data(), len);
		return ret;
	}

	Packet::sentinelbyte_t FromBytes(const Packet::buf_t& src) {
		Packet::sentinelbyte_t it = src.begin();
		Packet::LoadBytes(it, Level);
		uint32_t len = 0;
		Packet::LoadBytes(it, len);
		Name.resize(len);
		Packet::LoadBytes(it, Name.data(), len);
		return it;
	}
};

struct StatusData {

	int stat = 0;
	int val = 0;

};


int main(int argc, char* argv[]) {
	
	std::vector<std::string> args;
	args.insert(args.end(), argv, argv + argc);

	if (args.size() <= 1) {
		return -1;
	}

	if (std::stoi(args[1]) == 0) {
		Server();
	}
	else {
		Client();
	}
	
	return 0;
}

void Server() {

	TCPServer server(8080);

	std::vector<TCPSocket> clients;
	std::deque<TCPSocket*> lostqueue;

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

			if (available <= 0) {
				continue;
			}

			std::string val;
			val = *c.Recv()->Get<std::string>();
			auto address = c.GetPeerAddress();
			std::cout << "recived from (";
			if (address) {
				std::cout << address->Address();
			}
			std::cout << "):" << val << std::endl;

			std::rotate(val.begin(), val.begin() + 1, val.end());
			c.Send(val);
		}
	}
}

void Client() {

	TCPSocket server;

	if (server.Connect(IPAddress::Loopback().Port(8080))) {
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
		
		std::string reciveval;
		std::string sendval;

		std::cout << "input sendval: ";
		std::cin >> sendval;

		server.Send(sendval);

		reciveval = *server.Recv()->Get<std::string>();
		std::cout << "recived from server:" << reciveval << std::endl;
	}
}
