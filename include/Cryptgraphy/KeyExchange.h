#pragma once
#include "ECDSA.h"

class KeyFactoryDH {
public:

	using baselenint_t = bigint<32>;
	using int_t = bigint<32 * 2>;
	using modint_t = ModInt<int_t>;

	static inline const auto xmodp = modint_t::Factory("FFFFFFFFFFFFFFFFC90FDAA22168C234C4C6628B80DC1CD129024E088A67CC74020BBEA63B139B22514A08798E3404DDEF9519B3CD3A431B302B0A6DF25F14374FE1356D6D51C245E485B576625E7EC6F44C42E9A637ED6B0BFF5CB6F406B7EDEE386BFB5A899FA5AE9F24117C4B1FE649286651ECE45B3DC2007CB8A163BF0598DA48361C55D39A69163FA8FD24CF5F83655D23DCA3AD961C62F356208552BB9ED529077096966D670C354E4ABC9804F1746C08CA18217C32905E462E36CE3BE39E772C180E86039B2783A2EC07A28FB5C55DF06F4C52C9DE2BCBF6955817183995497CEA956AE515D2261898FA051015728E5A8AACAA68FFFFFFFFFFFFFFFF");
	static inline const modint_t X = xmodp.Make("80000001F000000F80000001F0000001");
	static inline RandomGenerator rg{};

	KeyFactoryDH() {
		do {
			SecretKey = int_t(rg.NextBytes(baselenint_t::WordBytes));
		} while (SecretKey >= X.GetP() || SecretKey <= 2);
	}

	[[nodiscard]]
	baselenint_t GeneratePublicKey() const {
		return X.Pow(SecretKey).value;
	}

	[[nodiscard]]
	baselenint_t MakeSharedKey(const int_t& pubkey) const {
		return xmodp(pubkey).Pow(SecretKey).value;
	}

	int_t SecretKey = 0;
};

class KeyFactoryECDH {
private:




	KeyFactoryECDH() {

	}




};