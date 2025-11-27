module Socket:enums;

enum class IPVersion : int {
	IPv4 = 0, //AF_INET,
	IPv6 = 1, //AF_INET6
};

enum class Protocol : int {
	TCP = 0, //SOCK_STREAM,
	UDP = 1, //SOCK_DGRAM,
};