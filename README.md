# Socket

クロスプラットフォームなソケット通信ライブラリです。
サブ機能としてAESによる暗号化や、データ型の変換を担うPacket構造体なんかがあったりします。
ソケット通信部分を除き、基本的にはC++20の標準ライブラリのみを使っているので、
どんな環境でも差がなく利用可能です。

# Features

- おそらく使いやすいインターフェイス ( 自己満なので... )
- クロスプラットフォーム
- IPv4とIPv6に対応
- 非同期通信 ( 先頭にASyncが付いてるもの )
- AES128の暗号化 ( デフォルトでCTRを使う )
- AESはシングルで数100MB/s、並列で500~900MB/sの性能 ( 要最適化ビルド )
- Packet構造体によるスマートなデータ型とバイト列の相互変換

# How to use

- C++20以降が必要です
- `Socket.h`をインクルードするだけです

詳細は、開発中の検証環境でもある`Socket.cpp`にあります。

> [!WARNING]
> `Windows.h`よりも先にインクルードしないとビルドに失敗します。

# Quick Start

今すぐに試してみたいという方は、
Visual Studio 2026 insider Edition
でこちらのリポジトリをクローンします。
その後は、`Socket.cpp`が砂場です。

# Credit

- Apopic ( by https://github.com/apopic )
- Kernel ( by https://github.com/KernelVR )
- ChatGPT ( by OpenAI )

# TODO:

- 構造体のリファレンスなど
