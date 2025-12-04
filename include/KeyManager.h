#pragma once
#include "Cryptgraphy/ECDSA.h"
#include "Packet.h"

class KeyFactoryDH {
public:

	using baseint_t = bigint<32>;
	using int_t = bigint<baseint_t::Words * 2>;
	using modint_t = ModInt<int_t>;

	static inline const modint_t::Factory xmodp = "FFFFFFFFFFFFFFFFC90FDAA22168C234C4C6628B80DC1CD129024E088A67CC74020BBEA63B139B22514A08798E3404DDEF9519B3CD3A431B302B0A6DF25F14374FE1356D6D51C245E485B576625E7EC6F44C42E9A637ED6B0BFF5CB6F406B7EDEE386BFB5A899FA5AE9F24117C4B1FE649286651ECE45B3DC2007CB8A163BF0598DA48361C55D39A69163FA8FD24CF5F83655D23DCA3AD961C62F356208552BB9ED529077096966D670C354E4ABC9804F1746C08CA18217C32905E462E36CE3BE39E772C180E86039B2783A2EC07A28FB5C55DF06F4C52C9DE2BCBF6955817183995497CEA956AE515D2261898FA051015728E5A8AACAA68FFFFFFFFFFFFFFFF";
	static inline const modint_t X = xmodp("80000001F000000F80000001F0000001");
	
	[[nodiscard]]
	static Cryptgraphy::bytearray GeneratePublicKey(const baseint_t& D) {
		auto&& pub = (baseint_t)X.Pow(D).value;
		
		Cryptgraphy::bytearray ret;
		ret.reserve(baseint_t::WordBytes);

		Packet::StoreBytes(ret, pub.words());

		return ret;
	}

	[[nodiscard]]
	static Cryptgraphy::bytearray MakeSharedKey(const baseint_t& D, const Cryptgraphy::bytearray& fromQ) {
		baseint_t temp;
		Cryptgraphy::byte_view view(fromQ);

		Packet::LoadBytes(view, temp.words());
		modint_t Q = xmodp(temp);

		auto&& equalQ = (baseint_t)Q.Pow(D).value;

		Cryptgraphy::bytearray ret;
		ret.reserve(baseint_t::WordBytes);

		Packet::StoreBytes(ret, equalQ.words());

		return ret;
	}
};

class KeyFactoryECDH : public secpParameters<secp256r1> {
public:

	[[nodiscard]]
	static Cryptgraphy::bytearray GeneratePublicKey(const baseint_t& D) {
		auto&& pubpoint = project_t(G).Scaler(xmodp(D));

		auto&& x = (baseint_t)pubpoint.ToAfinPoint().x.value;

		Cryptgraphy::bytearray ret;

		ret.reserve(baseint_t::WordBytes);

		Packet::StoreBytes(ret, x.words());

		return ret;
	}

	[[nodiscard]]
	static Cryptgraphy::bytearray MakeSharedKey(const baseint_t& D, const Cryptgraphy::bytearray& fromQ) {
		baseint_t temp;
		Cryptgraphy::byte_view view(fromQ);

		Packet::LoadBytes(view, temp.words());

		afinpoint_t Q = afinpoint(
			xmodp(temp),
			G.GetParam().GetY(xmodp(temp)).Sqrt()
		);

		auto&& equalQ = project_t(Q).Scaler(xmodp(D));
		auto x = (baseint_t)equalQ.ToAfinPoint().x.value;

		Cryptgraphy::bytearray ret;

		ret.reserve(baseint_t::WordBytes);

		Packet::StoreBytes(ret, x.words());

		return ret;
	}
};

template<class Factory>
class basic_KeyManager : public Factory {
public:

	static inline RandomGenerator rg{};

	basic_KeyManager() {
		typename Factory::int_t ret = 0;
		do {
			ret = typename Factory::baseint_t(rg.NextBytes(Factory::baseint_t::WordBytes));
		} while (ret >= Factory::xmodp.P() || ret <= 2);
		SecretKey = ret;
	}

	Cryptgraphy::bytearray MakeQKey() const {
		return Factory::GeneratePublicKey(SecretKey);
	}

	Cryptgraphy::bytearray MakeSharedKey(const Cryptgraphy::bytearray& fromQKey) const {
		return Factory::MakeSharedKey(SecretKey, fromQKey);
	}

	typename const Factory::baseint_t& GetSecretKey() const {
		return SecretKey;
	}

private:
	Factory::baseint_t SecretKey = 0;
};

using KeyManager = basic_KeyManager<KeyFactoryECDH>;