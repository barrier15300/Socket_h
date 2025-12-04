#include <deque>
#include <iomanip>
#include <chrono>
#include <map>

// #include "include/MultiWordInt.h"
// #include "include/ModInt.h"

#include "include/Socket.h"

void Server();
void Client();

AES128::cbytearray<16> sharedkey = {'0', 'x', '7', '4', '0', 'x', '6', '5', '0', 'x', '7', '3', '0', 'x', '7', '4', };

struct ClientData {

	int Level = 0;
	std::string Name = "NoName";

	Packet::bytearray ToBytes() const {
		Packet::bytearray ret;
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

	Packet::bytearray ToBytes() const {
		Packet::bytearray ret;
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

	Packet::bytearray ToBytes() const {
		Packet::bytearray ret;
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
#include "include/KeyManager.h"

int main(int argc, char* argv[]) {
	
	//KeyManager Keya;
	//KeyManager Keyb;
	//
	//auto tp = std::chrono::high_resolution_clock::now();
	//
	//auto kE = Keya.MakeQKey();
	//auto kF = Keyb.MakeQKey();
	//
	//auto Ga = Keya.MakeSharedKey(kF);
	//auto Gb = Keyb.MakeSharedKey(kE);
	//
	//auto ns = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::high_resolution_clock::now() - tp).count();
	//
	//bool same = Ga == Gb;
	//
	//std::cout << (double)ns / 1000 / 1000 / 1000 << "s" << std::endl;
	//std::cout << std::boolalpha << "shared key same: " << same << std::endl;
	//
	//for (auto&& b : Ga) {
	//	std::cout << std::hex << std::setw(2) << std::setfill('0') << std::right << (int)b;
	//}
	//std::cout << std::endl;
	//
	//for (auto&& b : Gb) {
	//	std::cout << std::hex << std::setw(2) << std::setfill('0') << std::right << (int)b;
	//}
	//std::cout << std::endl;

	for (uint64_t i = 0; i < 128; ++i) {
		std::string message = "Very Very Long Message" + std::string((char*)&i, (char*)&i + 16);
		auto ret = ECDSA::Hasher({message.begin(), message.end()});
	}

	//using int_t = bigint<8>;
	//using modint_t = ModInt<int_t>;
	//using project_t = ECProject<modint_t>;
	//
	//modint_t::Factory xmodp = "ffffffff00000001000000000000000000000000ffffffffffffffffffffffff";
	//project_t::Factory project = WeierstrassParameter<modint_t>(
	//	xmodp("ffffffff00000001000000000000000000000000fffffffffffffffffffffffc"),
	//	xmodp("5ac635d8aa3a93e7b3ebbd55769886bc651d06b0cc53b0f63bce3c3e27d2604b")
	//);
	//
	//auto G = project(
	//	xmodp("6b17d1f2e12c4247f8bce6e563a440f277037d812deb33a0f4a13945d898c296"),
	//	xmodp("4fe342e2fe1a7f9b8ee7eb4a7c0f9e162bce33576b315ececbb6406837bf51f5"),
	//	xmodp(1)
	//);
	//
	//auto view = [](const std::string& name, const project_t& p) {
	//	std::cout << name << ": {"
	//		<< p.x.value.ToString(16) << ", "
	//		<< p.y.value.ToString(16) << ", "
	//		<< p.z.value.ToString(16) << "}"
	//		<< std::endl;
	//};
	//auto check = [](const project_t& p) {
	//	auto a = p.ToAfinPoint();
	//	std::cout << "Check: " << std::boolalpha
	//		<< a.GetParam().CheckPoint(a.x, a.y)
	//		<< std::endl;
	//};
	//
	//auto d = G.Double();
	//
	//view("double", d);
	//check(d);
	//
	//auto add = G.Add(d);
	//
	//view("add", add);
	//check(add);
	//
	//auto scaler = G.Scaler(xmodp(100));
	//
	//view("scaler", scaler);
	//check(scaler);

	// arg[1]{ 0 = server, 1 = client }

	//std::vector<std::string> args;
	//args.insert(args.end(), argv, argv + argc);
	//
	//if (args.size() <= 1) {
	//	return -1;
	//}
	//
	//if (std::stoi(args[1]) == 0) {
	//	Server();
	//}
	//else {
	//	Client();
	//}
	
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

			c->CryptEngine.Init(sharedkey);

			auto cd = c->EncryptionRecv()->Get<ClientData>();

			if (cd) {
				std::cout << "connected: " << cd->Name << std::endl;
				auto addr = c->GetPeerAddress();
				clients[*addr] = {std::move(*c), std::move(*cd)};
				c.reset();
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

			auto val = c.EncryptionRecv();

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
				oc.EncryptionSend(send);
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

	server.CryptEngine.Init(sharedkey);

	ClientData _data;
	
	std::cout << "input your Level\n";
	std::cin >> _data.Level;
	std::cout << "input your Name\n";
	std::cin >> _data.Name;

	Packet p = Packet(_data);

	server.EncryptionSend(p);

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
				server.EncryptionSend(sendval);
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

		auto pak = server.EncryptionRecv();
		
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
