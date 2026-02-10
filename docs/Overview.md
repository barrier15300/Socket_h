# Overview

ようこそ、ここは、ライブラリのポータルドキュメントです。

---
# Words

| 単語   | 意味                                  |
| ---- | ----------------------------------- |
| ソケット | 異なるマシン間で通信可能なオブジェクトを指す(TCPSocketなど) |
| バイト列 | uint8_tが並んでいる配列のこと                  |
| データ型 | バイト列との相互変換可能である型                    |


---
# Headers

## メイン

| ヘッダファイル                           | 説明                      | 対応状況 | ソース        |
| --------------------------------- | :---------------------- | :--: | ---------- |
| [Socket.h](Main/Socket/Socket.md) | 基本的なソケット通信を提供するヘッダー     |  o   | [Source]() |
| [Packet.h](Main/Packet/Packet.md) | データ型とバイト列の相互変換を提供するヘッダー |  o   | [Source]() |

## 暗号

| ヘッダファイル                                                             | 説明                        | 対応状況 | ソース        |
| ------------------------------------------------------------------- | :------------------------ | :--: | ---------- |
| [AES128.h](Cryptgraphy/AES128/AES128.md)                            | 128bitのAESによる暗号化を提供するヘッダー |  o   | [Source]() |
| [ECDSA.h](Cryptgraphy/ECDSA/ECDSA.md)                               | ECDSAによる認証を提供するヘッダー       |  o   | [Source]() |
| [ECPoint.h](Cryptgraphy/ECPoint/ECPoint.md)                         | 楕円曲線を表現する構造体を定義するヘッダー     |  o   | [Source]() |
| [KeyManager.h](Cryptgraphy/KeyManager/KeyManager.md)                | 鍵交換アルゴリズムを提供するヘッダー        |  o   | [Source]() |
| [MultiWordInt.h](Cryptgraphy/MultiWordInt/MultiWordInt.md)          | 多倍長整数を提供するヘッダー            |  o   | [Source]() |
| [ModInt.h](Cryptgraphy/ModInt/ModInt.md)                            | 整数に対する有限体を提供するヘッダー        |  o   | [Source]() |
| [NumberSet.h](Cryptgraphy/NumberSet/NumberSet.md)                   | 数の集合を定義するヘッダー             |  o   | [Source]() |
| [RandomGenerator.h](Cryptgraphy/RandomGenerator/RandomGenerator.md) | 乱数列の生成を提供するヘッダー           |  o   | [Source]() |
| [SHAKE256.h](Cryptgraphy/SHAKE256/SHAKE256.md)                      | SHA-3規格のSHAKE256を提供するヘッダー |  o   | [Source]() |
