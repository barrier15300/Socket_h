# IPAddressBase
```C++
template<IPVersion _type>
struct IPAddressBase;

using IPAddress = IPAddressBase<IPVersion::IPv4>;
using IPv6Address = IPAddressBase<IPVersion::IPv6>;
```

---
# 概要

IPAddressBase構造体は、同じインターフェイスで別々のバージョンを扱う事が可能な、IPアドレスを格納する構造体である。
テンプレートパラメータとしてIPアドレスのバージョンを受け取るようになっており、使用を容易にするため、以下のパラメータ設定済みエイリアスが定義されている。

| 型名          | 説明               | ソース        |
| ----------- | ---------------- | ---------- |
| IPAddress   | IPv4のアドレスとして使用する | [Source]() |
| IPv6Address | IPv6のアドレスとして使用する | [Source]() |

# メンバ関数

## 構築・破棄

| 名前                | 説明      | ソース        |
| ----------------- | ------- | ---------- |
| [(constructor)]() | コンストラクタ | [Source]() |
| [(destructor)]()  | デストラクタ  | [Source]() |
| operator=         | 代入演算子   | [Source]() |

---
# メンバーアクセス

| 名前          | 説明                   | ソース        |
| ----------- | -------------------- | ---------- |
| [Address]() | アドレスを文字列で設定/取得する     | [Source]() |
| [Port]()    | ポート番号を設定/取得する        | [Source]() |
| [Version]() | IPアドレスのバージョンを設定/取得する | [Source]() |

---
# 定数プロパティ

| 名前           | 説明               | ソース        |
| ------------ | ---------------- | ---------- |
| [Any]()      | ワイルドカードアドレスを取得する | [Source]() |
| [Loopback]() | ローカルホストアドレスを取得する | [Source]() |
