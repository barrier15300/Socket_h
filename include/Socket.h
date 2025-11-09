#pragma once

#ifdef _MSC_BUILD
#ifndef _INC_WINDOWS
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#pragma comment(lib,"ws2_32.lib")
#endif // _INC_WINDOWS
#else
#include <sys/socket.h>
#endif // _MSC_BUILD

#include <string>
#include <cstdint>
#include <stdexcept>
#include <optional>
#include <future>
#include <vector>
#include <iostream>

#include "AES128.h"
#include "Packet.h"

/// <summary>
/// Debug Utility
/// </summary>

#ifdef _DEBUG
#ifdef _MSC_BUILD
#define _last_error() WSAGetLastError()
#else
#define _last_error() errno
#endif
#define dbg_print() std::cerr << "error in " << __func__ << ": " << _last_error() << std::endl
#else 
#define dbg_print()
#endif


/// <summary>
/// Enums
/// </summary>

enum class IPVersion : int {
	IPv4 = AF_INET,
	IPv6 = AF_INET6
};

enum class Protocol : int {
	TCP = SOCK_STREAM,
	UDP = SOCK_DGRAM,
};


/// <summary>
/// IPAddress
/// </summary>

template<IPVersion type>
struct IPAddressBase {
	constexpr static int VersionValue = static_cast<int>(type);
	constexpr static bool IsIPv4 = VersionValue == (int)IPVersion::IPv4;
	constexpr static bool IsIPv6 = VersionValue == (int)IPVersion::IPv6;

	using rawaddr_t = struct sockaddr_in;
	using rawaddr6_t = struct sockaddr_in6;

	using address_t =
		std::conditional_t<IsIPv4, rawaddr_t,
		std::conditional_t<IsIPv6, rawaddr6_t,
		void>>;

	constexpr static
		std::conditional_t<IsIPv4, decltype(&rawaddr_t::sin_port),
		std::conditional_t<IsIPv6, decltype(&rawaddr6_t::sin6_port),
		void*>> PortPtr() {
		if constexpr (IsIPv4) {
			return &rawaddr_t::sin_port;
		}
		else if constexpr (IsIPv6) {
			return &rawaddr6_t::sin6_port;
		}
		return nullptr;
	}

	constexpr static
		std::conditional_t<IsIPv4, decltype(&rawaddr_t::sin_addr),
		std::conditional_t<IsIPv6, decltype(&rawaddr6_t::sin6_addr),
		void*>> AddressPtr() {
		if constexpr (IsIPv4) {
			return &rawaddr_t::sin_addr;
		}
		else if constexpr (IsIPv6) {
			return &rawaddr6_t::sin6_addr;
		}
		return nullptr;
	}

	constexpr static size_t AddressStringSize() {
		return
			(IsIPv4 ? INET_ADDRSTRLEN :
			(IsIPv6 ? INET6_ADDRSTRLEN : 128));
	}

	IPAddressBase() { this->Version(VersionValue); }
	IPAddressBase(std::string host) : IPAddressBase() { auto addr = SolveHostName(host); if (addr) { *this = addr.value(); } }
	IPAddressBase(uint16_t port) : IPAddressBase() { Port(port); }
	IPAddressBase(std::string host, uint16_t port) : IPAddressBase(std::move(host)) { Port(port); }
	IPAddressBase(const IPAddressBase&) = default;
	IPAddressBase(IPAddressBase&&) = default;
	IPAddressBase(const address_t& addr) : address(addr) {}
	IPAddressBase(address_t&& addr) : address(std::move(addr)) {}

	IPAddressBase& operator=(const IPAddressBase&) = default;
	IPAddressBase& operator=(IPAddressBase&&) = default;

	operator struct sockaddr* () {
		return reinterpret_cast<struct sockaddr*>(&address);
	}

	IPAddressBase& Address(const std::string& addr) {
		int ret = inet_pton(VersionValue, addr.c_str(), &((&address)->*AddressPtr()));
		if (ret == 0 || ret == -1) {
			dbg_print();
		}
		return *this;
	}
	std::string Address() const {
		std::string ret(AddressStringSize(), '\0');
		if (inet_ntop(VersionValue, &((&address)->*AddressPtr()), ret.data(), INET_ADDRSTRLEN) == nullptr) {
			dbg_print();
		}
		return ret;
	}
	IPAddressBase& Port(uint16_t port) {
		((&address)->*PortPtr()) = htons(port);
		return *this;
	}
	uint16_t Port() const {
		return ntohs(((&address)->*PortPtr()));
	}
	IPAddressBase& Version(int version) {
		sockaddr* addr = (sockaddr*)&address;
		addr->sa_family = version;
		return *this;
	}
	int Version() {
		sockaddr* addr = (sockaddr*)&address;
		return static_cast<int>(addr->sa_family);
	}

	static IPAddressBase Any() {
		return
			(IsIPv4 ? IPAddressBase("0.0.0.0") :
			(IsIPv6 ? IPAddressBase("::") :
			IPAddressBase()));
	}
	static IPAddressBase Loopback() {
		return
			(IsIPv4 ? IPAddressBase("127.0.0.1") :
			(IsIPv6 ? IPAddressBase("::1") :
			IPAddressBase()));
	}

	static std::optional<IPAddressBase> SolveHostName(const std::string& hostname, Protocol type = Protocol::TCP) {
		struct addrinfo hints = {};
		hints.ai_family = VersionValue;
		hints.ai_socktype = static_cast<int>(type);
		struct addrinfo* res;
		if (getaddrinfo(hostname.c_str(), nullptr, &hints, &res) != 0) {
			dbg_print();
			return std::nullopt;
		}
		IPAddressBase ret;
		ret.address = *(address_t*)res->ai_addr;
		freeaddrinfo(res);
		return ret;
	}
	
protected:
	address_t address{};
};


#ifdef _MSC_BUILD

/// <summary>
/// WSA Management singleton
/// </summary>

class WinSock {
	WinSock() {
		int iResult = WSAStartup(MAKEWORD(2, 2), &data);
		if (iResult != 0) {
			dbg_print();
			return;
		}
		is_init = true;
	}
	~WinSock() {
		WSACleanup();
	}

	bool is_init = false;
	WSADATA data{};
public:

	static WinSock& GetInstance() {
		static WinSock instance;
		return instance;
	}

	const WSADATA& GetData() const {
		return data;
	}

};

#endif


/// <summary>
/// Socket Base
/// </summary>

struct SocketUtil {
	using bytearray = std::vector<uint8_t>;

	template<class T>
	using remove_cvref_t = std::remove_cv_t<std::remove_reference_t<T>>;

	template<class T>
	using stdlayout = std::enable_if_t<std::is_standard_layout<T>::value, T>;

};

template<class ipT, Protocol _protocol>
class SocketBase : public SocketUtil {
public:
	using IPType = ipT;
	
	// using poll_t;
#ifdef _MSC_BUILD
	using poll_t = WSAPOLLFD;
#else
	using poll_t = struct pollfd;
#endif // _MSC_BUILD

	// using sock_t;
#ifdef _MSC_BUILD
	using sock_t = SOCKET;
#else
	using sock_t = int;
#endif // _MSC_BUILD

	SocketBase() {
#ifdef _MSC_BUILD
		WinSock::GetInstance();
#endif // _MSC_BUILD
		sock = socket(ipT::VersionValue, static_cast<int>(_protocol), 0);
		if (!IsValid()) {
			dbg_print();
		}
		pfd.fd = sock;
		pfd.events = POLLIN;
	}
	~SocketBase() {
		Close();
	}
	
	SocketBase(const SocketBase&) = delete;
	SocketBase(SocketBase&& other) noexcept : sock(other.sock), pfd(other.pfd) {
		other.Release();
	}

	SocketBase& operator=(const SocketBase&) = delete;
	SocketBase& operator=(SocketBase&& other) noexcept {
		return *Copy(&other);
	}

	void Close() {
		if (!IsValid()) return;

#ifdef _MSC_BUILD
		closesocket(sock);
#else
		close(sock);
#endif
		Release();
	}

	operator bool() const {
		return IsValid();
	}

	bool IsValid() const {
		return sock != InValidSocket();
	}

	bool operator==(const SocketBase& other) const {
		return sock == other.sock;
	}
	bool operator!=(const SocketBase& other) const {
		return sock != other.sock;
	}

#ifdef _MSC_BUILD
	static const WSADATA& GetWinSockData() {
		return WinSock::GetInstance().GetData();
	}
#endif // _MSC_BUILD
	
protected:

	SocketBase* Copy(SocketBase* other) {
		if (this == other) {
			return this;
		}
		if (!IsValid()) {
			Close();
		}
		sock = other->sock;
		pfd = other->pfd;
		other->Release();
		return this;
	}
	void Release() {
		sock = -1;
		pfd.fd = -1;
	}

	static int Poll(poll_t* fds, unsigned int nfds, int timeout) {
#ifdef _MSC_BUILD
		int ret = WSAPoll(fds, nfds, 0);
#else 
		int ret = poll(fds, nfds, 0);
#endif // _MSC_BUILD
		return ret;
	}

	SocketBase(sock_t s) : sock(s) {}

	static sock_t InValidSocket() {
#ifdef _MSC_BUILD
		return INVALID_SOCKET;
#else
		return -1;
#endif
	}

	sock_t sock = InValidSocket();
	poll_t pfd{};

	constexpr static int int_size = sizeof(int);
};


/// <summary>
/// TCP Protocol Socket
/// </summary>

template<class ipT, class sockbase = SocketBase<ipT, Protocol::TCP>>
class basic_TCPSocket : public sockbase {
	template<class, class>
	friend class basic_TCPServer;
protected:

	using sockbase::sockbase;

public:

	using bytearray = typename sockbase::bytearray;

	template<class T>
	using remove_cvref_t = typename sockbase::remove_cvref_t<T>;

	template<class T>
	using stdlayout = typename sockbase::stdlayout<T>;
	
	basic_TCPSocket() : sockbase() {}
	basic_TCPSocket(typename sockbase::IPType addr) : basic_TCPSocket() {
		Connect(addr);
	}

	basic_TCPSocket(const basic_TCPSocket&) = delete;
	basic_TCPSocket(basic_TCPSocket&& other) noexcept : sockbase(std::move(other)) {}

	basic_TCPSocket& operator=(const basic_TCPSocket&) = delete;
	basic_TCPSocket& operator=(basic_TCPSocket&& other) noexcept {
		return *(basic_TCPSocket*)sockbase::Copy(&other);
	}

	bool Connect(typename sockbase::IPType hostaddr, int timeout = 0) {
		if (connect(sockbase::sock, hostaddr, sizeof(typename sockbase::IPType)) < 0) {
			dbg_print();
			return false;
		}
		return true;
	}
	int Available() {
#ifdef _MSC_BUILD
		u_long bytes = 0;
		if (ioctlsocket(sockbase::sock, FIONREAD, &bytes) == SOCKET_ERROR) {
			dbg_print();
			return -1;
		}
		return static_cast<int>(bytes);
#else 
		int bytes = 0;
		if (ioctl(sockbase::sock, FIONREAD, &bytes) < 0) {
			dbg_print();
			return -1;
		}
		return bytes;
#endif
	}
	std::optional<typename sockbase::IPType> GetPeerAddress() {
		typename sockbase::IPType ret;
		int addrlen = sizeof(ret);
		if (getpeername(sockbase::sock, (sockaddr*)ret, &addrlen) != 0) {
			return std::nullopt;
		}
		return ret;
	}
	bool LostConnection() {
		int ret = sockbase::Poll(std::addressof(sockbase::pfd), 1, 0);

		if (ret == 0) {
			return false;
		}

		if (ret < 0) {
			dbg_print();
			int err = GetLastError();
#ifdef _MSC_BUILD
			if (err == WSAEINTR)
#else
			if (err == EINTR)
#endif
			{
				return false;
			}
			return true;
		}

		if (sockbase::pfd.revents & (POLLHUP | POLLERR)) {
			return true;
		}

		if (sockbase::pfd.revents & POLLIN) {
			char buf;
			int r = recv(sockbase::sock, &buf, 1, MSG_PEEK);
			if (r == 0) {
				return true;
			}
			if (r < 0) {
				int err = GetLastError();
#ifdef _MSC_BUILD
				if (err == WSAECONNRESET)
#else
				if (err == ECONNRESET)
#endif
				{
					return true;
				}
			}
		}
		
		return false;
	}
	void _NonBlocking() {
#ifdef _MSC_BUILD
		u_long mode = 1;
		ioctlsocket(sockbase::sock, FIONBIO, &mode);
#else
		int flag = fcntl(sockbase::sock, F_GETFL, 0);
		fcntl(sockbase::sock, F_SETFL, flag | O_NONBLOCK);
#endif
	}

	bool RawSend(const void* src, int size) {
		int ret = send(sockbase::sock, (const char*)src, size, 0);
		return ret >= 0;
	}
	bool RawRecv(void* dest, int size) {
		int received = 0;

		while (received < size) {
			int ret = recv(sockbase::sock, (char*)dest + received, size - received, 0);
			if (ret <= 0) return false;
			received += ret;
		}
		return true;
	}
	
	bool Send(const bytearray& src) {
		return RawSend(src.data(), static_cast<int>(src.size()));
	}
	bool Recv(bytearray& dest) {
		if (dest.empty()) { return false; }
		return RawRecv(dest.data(), static_cast<int>(dest.size()));
	}

	bool Send(const Packet& src) {
		if (src.Size() <= Packet::HeaderSize) {
			return false;
		}
		return Send(src.GetBuffer());
	}
	std::optional<Packet> Recv() {
		bytearray head(Packet::HeaderSize);
		if (Recv(head)) {
			return std::nullopt;
		}
		Packet ret;
		ret = std::move(head);
		bytearray data(ret.GetHeader().Size);
		if (Recv(data)) {
			return std::nullopt;
		}
		bytearray pak;
		pak.reserve(Packet::HeaderSize + data.size());
		pak.insert(pak.end(), head.begin(), head.end());
		pak.insert(pak.end(), data.begin(), data.end());
		ret = std::move(pak);
		return ret;
	}

	bool EncryptionSend(const bytearray& src) {
		typename sockbase::remove_cvref_t<decltype(src)> target;
		Encrypt(src, target);
		return Send(target);
	}
	bool EncryptionRecv(bytearray& dest) {
		bool f = Recv(dest);
		Decrypt(dest, dest);
		return f;
	}

	bool EncryptionSend(const Packet& src) {
		if (src.Size() <= Packet::HeaderSize) {
			return false;
		}
		bytearray data(src.GetBuffer().begin() + Packet::HeaderSize, src.GetBuffer().end());
		Encrypt(data, data);
		bytearray target;
		target.reserve(Packet::HeaderSize + data.size());
		target.insert(target.end(), src.GetBuffer().begin(), src.GetBuffer().begin() + Packet::HeaderSize);
		target.insert(target.end(), data.begin(), data.end());
		return Send(target);
	}
	std::optional<Packet> EncryptionRecv() {
		bytearray head(Packet::HeaderSize);
		if (Recv(head)) {
			return std::nullopt;
		}
		Packet ret;
		ret = std::move(head);
		bytearray data(ret.GetHeader().Size);
		if (Recv(data)) {
			return std::nullopt;
		}
		Decrypt(data, data);
		bytearray pak;
		pak.reserve(Packet::HeaderSize + data.size());
		pak.insert(pak.end(), head.begin(), head.end());
		pak.insert(pak.end(), data.begin(), data.end());
		ret = std::move(pak);
		return ret;
	}

	std::future<bool> ASyncSend(const bytearray& src) {
		return std::async(std::launch::async, [&]() {
			return this->Send(src);
		});
	}
	std::future<bool> ASyncRecv(bytearray& dest) {
		return std::async(std::launch::async, [&]() {
			return this->Recv(dest);
		});
	}

	std::future<bool> ASyncSend(const Packet& src) {
		return std::async(std::launch::async, [&]() {
			return this->Send(src);
		});
	}
	std::future<std::optional<Packet>> ASyncRecv() {
		return std::async(std::launch::async, [&]() {
			return this->Recv();
		});
	}

	std::future<bool> ASyncEncryptionSend(const bytearray& src) {
		return std::async(std::launch::async, [&]() {
			return this->EncryptionSend(src);
		});
	}
	std::future<bool> ASyncEncryptionSend(bytearray& dest) {
		return std::async(std::launch::async, [&]() {
			return this->EncryptionRecv(dest);
		});
	}

	std::future<bool> ASyncEncryptionSend(const Packet& src) {
		return std::async(std::launch::async, [&]() {
			return this->EncryptionSend(src);
		});
	}
	std::future<std::optional<Packet>> ASyncEncryptionSend() {
		return std::async(std::launch::async, [&]() {
			return this->EncryptionRecv();
		});
	}

	template<class T>
	bool _Send(const stdlayout<T>& target) {
		return RawSend(&target, sizeof(T));
	}
	template<class T>
	bool _Recv(stdlayout<T>& target) {
		return RawRecv(&target, sizeof(T));
	}
	
	template<class T>
	std::future<bool> _ASyncSend(const stdlayout<T>& target) {
		return std::async(std::launch::async, [this, target]() {
			return this->_Send<T>(target);
		});
	}
	template<class T>
	std::future<bool> _ASyncRecv(stdlayout<T>& target) {
		return std::async(std::launch::async, [this, &target]() {
			return this->_Recv<T>(target);
		});
	}

protected:

	bool Crypt(const std::vector<uint8_t>& src, std::vector<uint8_t>& dest, typename AES128::cryptmode_t mode) {
		return ((&cryptengine)->*mode)(src, dest, src.size());
	}
	bool Encrypt(const std::vector<uint8_t>& src, std::vector<uint8_t>& dest) {
		return Crypt(src, dest, &AES128::CTREncrypt);
	}
	bool Decrypt(const std::vector<uint8_t>& src, std::vector<uint8_t>& dest) {
		return Crypt(src, dest, &AES128::CTRDecrypt);
	}

	AES128 cryptengine;
};


/// <summary>
/// TCP Protocol Server
/// </summary>

template<class ipT, class sockbase = SocketBase<ipT, Protocol::TCP>>
class basic_TCPServer : public sockbase {
public:

	using TCPSocket = basic_TCPSocket<ipT>;

	basic_TCPServer() : sockbase() {}
	basic_TCPServer(uint16_t port) : basic_TCPServer() {
		Listen(port);
	}

	basic_TCPServer(const basic_TCPServer&) = delete;
	basic_TCPServer(basic_TCPServer&& other) noexcept : sockbase(std::move(other)) {}

	basic_TCPServer& operator=(const basic_TCPServer&) = delete;
	basic_TCPServer& operator=(basic_TCPServer&& other) noexcept {
		return *(basic_TCPServer*)sockbase::Copy(&other);
	}

	bool Bind(typename sockbase::IPType addr) {
		if (bind(sockbase::sock, addr, sizeof(typename sockbase::IPType)) < 0) {
			dbg_print();
			return false;
		}
		int opt = 1;
		setsockopt(sockbase::sock, SOL_SOCKET, SO_REUSEADDR, (const char*)&opt, sizeof(int));
		return true;
	}
	bool Listen(uint16_t port, int backlog = 128) {
		this->Bind(typename sockbase::IPType(port));
		if (listen(sockbase::sock, backlog) != 0) {
			dbg_print();
			return false;
		}
		return true;
	}
	void StopListen() {
		*this = basic_TCPServer();
	}
	std::optional<TCPSocket> Accept() {
		int ret = sockbase::Poll(std::addressof(sockbase::pfd), 1, 0);

		if (!(ret > 0 && (sockbase::pfd.revents & POLLIN))) {
			return std::nullopt;
		}

		TCPSocket client = accept(sockbase::sock, nullptr, nullptr);
		if (!client.IsValid()) {
			dbg_print();
			return std::nullopt;
		}
		client.pfd.fd = client.sock;
		client.pfd.events = POLLIN;
		return client;
	}

};


/// <summary>
/// using typedef
/// </summary>

using IPAddress = IPAddressBase<IPVersion::IPv4>;
using IPv6Address = IPAddressBase<IPVersion::IPv6>;

using TCPSocket = basic_TCPSocket<IPAddress>;
using TCPSocketV6 = basic_TCPSocket<IPv6Address>;

using TCPServer = basic_TCPServer<IPAddress>;
using TCPServerV6 = basic_TCPServer<IPv6Address>;


#undef dbg_print

// TODO:
// 	Datagram (UDP) Protocol implement
