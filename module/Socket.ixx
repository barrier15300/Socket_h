export module Socket;

#ifdef _MSC_BUILD
#define NOMINMAX
#include <winsock2.h>
#include <ws2tcpip.h>
#include <ws2def.h>
#include <windows.h>
#pragma comment(lib,"ws2_32.lib")
#else
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <netinet/in.h>
#include <netdb.h>
#include <fcntl.h>
#include <poll.h>
#endif // _MSC_BUILD

import Socket.Common;
import Socket.CrossLayered;

static int GetLastSocketError() {
#ifdef _MSC_BUILD
	return WSAGetLastError();
#else
	return errno;
#endif
}

#ifdef _DEBUG
#define dbg_print_impl() std::cerr << "error in " << __func__ << ": " << GetLastSocketError() << std::endl
#else
#define dbg_print_impl()
#endif

#define dbg_print() dbg_print_impl()

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

import :enums;

export {
	
	enum class IPVersion;
	enum class Protocol;
	
	





	
}