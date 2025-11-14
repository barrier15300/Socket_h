

#include <iostream>
#include <deque>
#include <iomanip>
#include <chrono>
#include <map>

// #include "include/MultiWordInt.h"
// #include "include/ModInt.h"


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
	
	Packet::byte_view FromBytes(Packet::byte_view view) {
		Packet::LoadBytes(view, Level);
		uint32_t len = 0;
		Packet::LoadBytes(view, len);
		Name.resize(len);
		Packet::LoadBytes(view, Name.data(), len);
		return view;
	}
};

struct ContainerInContainer {

	std::vector<std::string> names;

	Packet::buf_t ToBytes() const {
		Packet::buf_t ret;
		uint32_t size = names.size();
		Packet::StoreBytes(ret, size);
		for (auto&& elem : names) {
			uint32_t len = elem.size();
			Packet::StoreBytes(ret, len);
			Packet::StoreBytes(ret, elem.data(), len);
		}
		return ret;
	}

	Packet::byte_view FromBytes(Packet::byte_view view) {
		uint32_t size = 0;
		Packet::LoadBytes(view, size);
		names.reserve(size);
		for (size_t i = 0; i < size; ++i) {
			uint32_t len = 0;
			Packet::LoadBytes(view, len);
			std::string elem;
			elem.resize(len);
			Packet::LoadBytes(view, elem.data(), len);
			names.push_back(std::move(elem));
		}
		return view;
	}
};

struct ContainerInVariable {
	std::vector<ContainerInContainer> container;

	Packet::buf_t ToBytes() const {
		Packet::buf_t ret;
		uint32_t size = container.size();
		Packet::StoreBytes(ret, size);
		for (auto&& elem : container) {
			Packet::StoreBytes(ret, elem);
		}
		return ret;
	}

	Packet::byte_view FromBytes(Packet::byte_view view) {
		uint32_t size = 0;
		Packet::LoadBytes(view, size);
		container.reserve(size);
		for (size_t i = 0; i < size; ++i) {
			ContainerInContainer elem;
			Packet::LoadBytes(view, elem);
			container.push_back(std::move(elem));
		}
		return view;
	}
};

int main(int argc, char* argv[]) {
	
	// arg[1]{ 0 = server, 1 = client }

	//ContainerInContainer elem;
	//
	//elem.names.push_back("test1");
	//elem.names.push_back("test2");
	//elem.names.push_back("test3");
	//elem.names.push_back("test4");
	//
	//ContainerInVariable v;
	//v.container.push_back(elem);
	//v.container.push_back(elem);
	//v.container.push_back(elem);
	//v.container.push_back(elem);
	//
	//Packet pak = Packet(v);
	//
	//ContainerInVariable ret = *pak.Get<ContainerInVariable>();

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

	std::map<IPAddress, std::pair<TCPSocket, ClientData>> clients;
	std::vector<std::optional<TCPSocket>> joinqueue;
	std::deque<TCPSocket*> lostqueue;

	while (true) {
		auto sock = server.Accept();

		if (sock) {
			bool emptyfound = false;
			for (auto&& state : joinqueue) {
				if (!state) {
					state = std::move(*sock);
					emptyfound = true;
					break;
				}
			}
			if (!emptyfound) {
				joinqueue.push_back(std::move(*sock));
			}
		}

		for (auto&& [_, pair] : clients) {
			auto&& [c, cd] = pair;
			if (c.LostConnection()) {
				lostqueue.push_back(&c);
				std::cout << "lost connection: " << cd.Name << std::endl;
			}
		}

		for (auto&& c : joinqueue) {

			if (!c) {
				continue;
			}

			if (c->Available() <= 0) {
				continue;
			}

			auto cd = c->Recv()->Get<ClientData>();

			if (cd) {
				std::cout << "connected: " << cd->Name << std::endl;
				auto addr = c->GetPeerAddress();
				clients[*addr] = {std::move(*c), std::move(*cd)};
				c = std::nullopt;
			}
		}

		while (!lostqueue.empty()) {
			auto p = lostqueue.front();
			lostqueue.pop_front();

			clients.erase(*p->GetPeerAddress());
		}

		for (auto&& [_, pair] : clients) {
			auto&& [c, cd] = pair;

			int available = c.Available();

			if (available <= 0) {
				continue;
			}

			auto val = c.Recv();

			if (!val) {
				continue;
			}

			std::string send = cd.Name + "(" + std::to_string(cd.Level) + "): " + *val->Get<std::string>();

			std::cout << send << std::endl;

			for (auto&& [_, topair] : clients) {
				auto&& [oc, __] = topair;
				if (oc == c) {
					continue;
				}
				oc.Send(send);
			}
		}
	}
}

void Client() {

	TCPSocket server;

	

	std::cout << "input connect server address" << std::endl;
	std::string str_addr;
	std::cin >> str_addr;

	auto op_addr = IPAddress::SolveHostName(str_addr);

	if (!op_addr) {
		std::cout << "can't solved address" << std::endl;
		return;
	}

	std::cout << "input port" << std::endl;
	unsigned short port;
	std::cin >> port;

	if (server.Connect(op_addr->Port(port))) {
		std::cout << "connected server." << std::endl;
	}
	else {
		std::cout << "can't connect server." << std::endl;
		return;
	}

	ClientData _data;
	
	std::cout << "input your Level\n";
	std::cin >> _data.Level;
	std::cout << "input your Name\n";
	std::cin >> _data.Name;

	Packet p = Packet(_data);

	server.Send(p);

	bool stopflag = false;

	std::mutex mtx;

	std::thread inputthread = std::thread{
		[&] {
		while (!stopflag) {
			std::string sendval;
			std::cin >> sendval;
			
			if (sendval == "/exit") {
				stopflag = true;
				break;
			}

			std::lock_guard<std::mutex> lock(mtx);

			Packet pak = Packet(sendval);
			server.Send(sendval);
		}
	}
	};

	while (!stopflag) {

		if (server.LostConnection()) {
			break;
		}
		
		if (server.Available() <= 0) {
			continue;
		}

		auto pak = server.Recv();
		
		if (!pak) {
			continue;
		}

		std::lock_guard<std::mutex> lock(mtx);

		auto val = *pak->Get<std::string>();
		std::cout << val << std::endl;
	}

	stopflag = true;

	inputthread.join();
}
