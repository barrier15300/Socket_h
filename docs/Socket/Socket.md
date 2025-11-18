# Socket.h

このヘッダーでは、基本的なソケット通信に関するインターフェイスを提供します。

---
# enum

| 型名                              | 説明                   | ソース        |
| ------------------------------- | -------------------- | ---------- |
| [IPVersion](enums/IPVersion.md) | IPアドレスのバージョンをまとめた列挙型 | [Source]() |
| [Protocol](enums/Protocol.md)   | 通信で使うプロトコルをまとめた列挙型   | [Source]() |

---
# types

| 型名                                                                   | 説明                                            | ソース        |
| -------------------------------------------------------------------- | --------------------------------------------- | ---------- |
| [IPAddressBase](Socket/classes/IPAddressBase/IPAddressBase.md)       | IPアドレスを同じインターフェイスで扱うための構造体 (class template)   | [Source]() |
| [WinSock](WinSock.md)                                                | Windows環境で必須なWSAの初期化をするためのクラス (singleton)     | [Source]() |
| [SocketTraits](SocketTraits.md)                                      | ソケットで使う型を定義するクラス (class)                      | [Source]() |
| [SocketBase](SocketBase.md)                                          | ソケットの基底クラス (class template)                   | [Source]() |
| [basic_TCPSocket](Socket/classes/basic_TCPSocket/basic_TCPSocket.md) | TCPで送受信の機能を提供するクラス (class template)           | [Source]() |
| [basic_TCPServer](Socket/classes/basic_TCPServer/basic_TCPServer.md) | TCPでクライアントの接続に関する機能を提供をするクラス (class template) | [Source]() |
| [IPAddress](Socket/classes/IPAddressBase/IPAddressBase.md)           | IPv4のアドレス (type-alias)                        | [Source]() |
| [IPv6Address](Socket/classes/IPAddressBase/IPAddressBase.md)         | IPv6のアドレス (type-alias)                        | [Source]() |
| [TCPSocket](Socket/classes/basic_TCPSocket/basic_TCPSocket.md)       | IPv4を使うTCPソケット (type-alias)                   | [Source]() |
| [TCPSocketV6](Socket/classes/basic_TCPSocket/basic_TCPSocket.md)     | IPv6を使うTCPソケット (type-alias)                   | [Source]() |
| [TCPServer](Socket/classes/basic_TCPServer/basic_TCPServer.md)       | IPv4を使うTCPサーバー (type-alias)                   | [Source]() |
| [TCPServerV6](Socket/classes/basic_TCPServer/basic_TCPServer.md)     | IPv6を使うTCPサーバー (type-alias)                   | [Source]() |
