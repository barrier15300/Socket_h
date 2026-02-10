# Protocol

```C++
enum class Protocol : int {
	TCP = SOCK_STREAM,
	UDP = SOCK_DGRAM,
};
```

---
# 概要

Protocolは、ソケットの通信プロトコルを設定、取得するための列挙型である。