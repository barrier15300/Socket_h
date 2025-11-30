#pragma once

#include "MultiWordInt.h"
#include <random>

class KeyExchange {
public:

	using baselenint_t = bigint<32>;
	using int_t = bigint<32 * 2 + 1>;

	static inline const int_t P = int_t("FFFFFFFFFFFFFFFFC90FDAA22168C234C4C6628B80DC1CD129024E088A67CC74020BBEA63B139B22514A08798E3404DDEF9519B3CD3A431B302B0A6DF25F14374FE1356D6D51C245E485B576625E7EC6F44C42E9A637ED6B0BFF5CB6F406B7EDEE386BFB5A899FA5AE9F24117C4B1FE649286651ECE45B3DC2007CB8A163BF0598DA48361C55D39A69163FA8FD24CF5F83655D23DCA3AD961C62F356208552BB9ED529077096966D670C354E4ABC9804F1746C08CA18217C32905E462E36CE3BE39E772C180E86039B2783A2EC07A28FB5C55DF06F4C52C9DE2BCBF6955817183995497CEA956AE515D2261898FA051015728E5A8AACAA68FFFFFFFFFFFFFFFF");
	static inline const int_t X = int_t(0x10000000001);
	static inline std::random_device rd;
	static inline std::mt19937 gen;

	KeyExchange() {
		do {
			GenerateTempSecretKey();
		} while (SecretKey >= P || SecretKey <= 2);
	}
	
	void GenerateTempSecretKey() {
		gen.seed(rd());
		constexpr size_t writebytes = baselenint_t::Words * int_t::WordCharSize;
		std::string randombytes;
		randombytes.reserve(writebytes);
		while (writebytes > randombytes.size()) {
			uint32_t val = gen();
			auto temp = int_t::WordToString(val, 16);
			temp = std::string((int_t::WordCharSize / 2) - temp.size(), '0') + temp;
			randombytes += temp;
		}

		SecretKey = int_t(randombytes);
	}

	static int_t ModPow(const int_t& x, const int_t& n) {
		int_t ret = 1;
		int_t base = x;
		for (int_t::count_t i = 0, nbit = n.GetNBit(); i < nbit; ++i) {
			if (n.BitCheck(i)) {
				ret *= base;
				ret %= P;
			}
			base *= base;
			base %= P;
		}

		return ret;
	}

	[[nodiscard]]
	int_t GeneratePublicKey() const {
		return ModPow(X, SecretKey);
	}

	int_t MakeSharedKey(const int_t& pubkey) const {
		return ModPow(pubkey, SecretKey);
	}

	int_t SecretKey = 0;
};

