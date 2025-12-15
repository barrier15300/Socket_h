#pragma once
#include "ECDSA.h"

class KeyFactoryDH {
public:

	using baseint_t = bigint<32>;
	using int_t = bigint<baseint_t::Words * 2>;
	using modint_t = ModInt<int_t>;

	static inline const modint_t::Factory xmodp = "FFFFFFFFFFFFFFFFC90FDAA22168C234C4C6628B80DC1CD129024E088A67CC74020BBEA63B139B22514A08798E3404DDEF9519B3CD3A431B302B0A6DF25F14374FE1356D6D51C245E485B576625E7EC6F44C42E9A637ED6B0BFF5CB6F406B7EDEE386BFB5A899FA5AE9F24117C4B1FE649286651ECE45B3DC2007CB8A163BF0598DA48361C55D39A69163FA8FD24CF5F83655D23DCA3AD961C62F356208552BB9ED529077096966D670C354E4ABC9804F1746C08CA18217C32905E462E36CE3BE39E772C180E86039B2783A2EC07A28FB5C55DF06F4C52C9DE2BCBF6955817183995497CEA956AE515D2261898FA051015728E5A8AACAA68FFFFFFFFFFFFFFFF";
	static inline const modint_t X = xmodp("80000001F000000F80000001F0000001");
	
	static Cryptgraphy::bytearray GeneratePublicKey(const baseint_t& D) {
		auto&& pub = static_cast<baseint_t>(X.Pow(D).value);
		return pub.ToBytes();
	}

	static Cryptgraphy::bytearray MakeSharedKey(const baseint_t& D, const Cryptgraphy::bytearray& fromQ) {
		modint_t Q = xmodp(fromQ);
		auto&& equalQ = static_cast<baseint_t>(Q.Pow(D).value);
		return equalQ.ToBytes();
	}
};

class KeyFactoryECDH : public secpParameters<secp256r1> {
public:

	using Super = secpParameters<secp256r1>;

	using types = secpTraits<Super::baseint_t::Words>;

	using baseint_t = types::baseint_t;
	using int_t = types::int_t;
	using modint_t = types::modint_t;
	using affin_t = types::affin_t;
	using projective_t = types::projective_t;

	static Cryptgraphy::bytearray GeneratePublicKey(const baseint_t& D) {
		auto&& pubpoint = projective_t(Super::G).Scaler(Super::xmodp(D) % Super::N);
		auto&& x = static_cast<baseint_t>(pubpoint.ToAffin().x.value);
		return x.ToBytes();
	}

	static Cryptgraphy::bytearray MakeSharedKey(const baseint_t& D, const Cryptgraphy::bytearray& fromQ) {
		auto Q = Super::projective(
			Super::xmodp(fromQ),
			Super::projective.Param().GetY(Super::xmodp(fromQ)).Sqrt(),
			Super::xmodp(1)
		);

		auto&& equalQ = projective_t(Q).Scaler(Super::xmodp(D) % Super::N);
		auto x = static_cast<baseint_t>(equalQ.ToAffin().x.value);

		return x.ToBytes();
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

	const typename Factory::baseint_t& GetSecretKey() const {
		return SecretKey;
	}

private:
	Factory::baseint_t SecretKey = 0;
};

using KeyManager = basic_KeyManager<KeyFactoryECDH>;
