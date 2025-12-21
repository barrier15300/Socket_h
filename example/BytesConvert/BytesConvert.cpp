#include "include/Socket.h"

struct ContainerInContainer {

	std::vector<std::string> names;

	Packet::bytearray ToBytes() const {
		Packet::bytearray ret;
		Packet::StoreBytes(ret, names);
		return ret;
	}

	Packet::byte_view FromBytes(Packet::byte_view view) {
		Packet::LoadBytes(view, names);
		return view;
	}
};

struct ContainerInVariable {
	std::vector<ContainerInContainer> container;

	Packet::bytearray ToBytes() const {
		Packet::bytearray ret;
		Packet::StoreBytes(ret, container);
		return ret;
	}

	Packet::byte_view FromBytes(Packet::byte_view view) {
		Packet::LoadBytes(view, container);
		return view;
	}
};

int main(int argc, char* argv[]) {

	ContainerInVariable data{};
	ContainerInContainer cic{};

	std::string str = "test";

	cic.names.push_back(str); str += "t";
	cic.names.push_back(str); str += "t";
	cic.names.push_back(str); str += "t";
	cic.names.push_back(str); str += "t";

	data.container.push_back(cic);

	str = "test2";

	cic.names.push_back(str); str += "b";
	cic.names.push_back(str); str += "b";
	cic.names.push_back(str); str += "b";
	cic.names.push_back(str); str += "b";

	data.container.push_back(cic);

	str = "magic";

	cic.names.push_back(str); str += "m";
	cic.names.push_back(str); str += "m";
	cic.names.push_back(str); str += "m";
	cic.names.push_back(str); str += "m";

	data.container.push_back(cic);

	str = "test";

	cic.names.push_back(str); str += "z";
	cic.names.push_back(str); str += "z";
	cic.names.push_back(str); str += "z";
	cic.names.push_back(str); str += "z";

	data.container.push_back(cic);

	Packet pak = Packet(data);

	auto& buf = pak.GetBuffer();

	for (auto&& c : buf) {
		std::cout << std::uppercase << std::setfill('0') << std::setw(2) << std::hex << std::right << static_cast<int>(c);
	}

	return 0;
}