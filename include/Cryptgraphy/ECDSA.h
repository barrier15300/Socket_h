#pragma once
#include "MultiWordInt.h"
#include "ModInt.h"
#include "ECPoint.h"
#include "RandomGenerator.h"
#include "SHAKE256.h"

struct secp112r1 {};
struct secp112r2 {};
struct secp128r1 {};
struct secp128r2 {};
struct secp160k1 {};
struct secp160r1 {};
struct secp160r2 {};
struct secp192k1 {};
struct secp192r1 {};
struct secp224k1 {};
struct secp224r1 {};
struct secp256k1 {};
struct secp256r1 {};
struct secp384r1 {};
struct secp521r1 {};

template<size_t _words>
struct secpTraits {
	using baseint_t		= bigint<_words>;
	using int_t			= bigint<baseint_t::Words * 2>;
	using modint_t		= ModInt<int_t>;
	using affin_t		= ECAffin<modint_t>;
	using projective_t	= ECProjective<modint_t>;
};

template<class secpType>
struct secpParameters : public secpTraits<1> {
	static inline const modint_t::Factory xmodp = "-1";
	static inline const modint_t N = xmodp("-1");
	static inline const auto ECParam = WeierstrassParameter<modint_t>(
		xmodp("-1"),
		xmodp("-1")
	);
	static inline const affin_t::Factory affin = ECParam;
	static inline const projective_t::Factory projective = ECParam;
	static inline const affin_t G = affin(
		xmodp("-1"),
		xmodp("-1")
	);
};

template<>
struct secpParameters<secp112r1> : public secpTraits<2> {
	static inline const modint_t::Factory xmodp = "DB7C2ABF62E35E668076BEAD208B";
	static inline const modint_t N = xmodp("DB7C2ABF62E35E7628DFAC6561C5");
	static inline const auto ECParam = WeierstrassParameter<modint_t>(
		xmodp("DB7C2ABF62E35E668076BEAD2088"),
		xmodp("659EF8BA043916EEDE8911702B22")
	);
	static inline const affin_t::Factory affin = ECParam;
	static inline const projective_t::Factory projective = ECParam;
	static inline const affin_t G = affin(
		xmodp("09487239995A5EE76B55F9C2F098"),
		xmodp("ADCD46F5882E3747DEF36E956E97")
	);
};

template<>
struct secpParameters<secp112r2> : public secpTraits<2> {
	static inline const modint_t::Factory xmodp = "DB7C2ABF62E35E668076BEAD208B";
	static inline const modint_t N = xmodp("36DF0AAFD8B8D7597CA10520D04B");
	static inline const auto ECParam = WeierstrassParameter<modint_t>(
		xmodp("6127C24C05F38A0AAAF65C0EF02C"),
		xmodp("51DEF1815DB5ED74FCC34C85D709")
	);
	static inline const affin_t::Factory affin = ECParam;
	static inline const projective_t::Factory projective = ECParam;
	static inline const affin_t G = affin(
		xmodp("4BA30AB5E892B4E1649DD0928643"),
		xmodp("ADCD46F5882E3747DEF36E956E97")
	);
};

template<>
struct secpParameters<secp128r1> : public secpTraits<2> {
	static inline const modint_t::Factory xmodp = "FFFFFFFDFFFFFFFFFFFFFFFFFFFFFFFF";
	static inline const modint_t N = xmodp("FFFFFFFE0000000075A30D1B9038A115");
	static inline const auto ECParam = WeierstrassParameter<modint_t>(
		xmodp("FFFFFFFDFFFFFFFFFFFFFFFFFFFFFFFC"),
		xmodp("E87579C11079F43DD824993C2CEE5ED3")
	);
	static inline const affin_t::Factory affin = ECParam;
	static inline const projective_t::Factory projective = ECParam;
	static inline const affin_t G = affin(
		xmodp("161FF7528B899B2D0C28607CA52C5B86"),
		xmodp("CF5AC8395BAFEB13C02DA292DDED7A83")
	);
};

template<>
struct secpParameters<secp128r2> : public secpTraits<2> {
	static inline const modint_t::Factory xmodp = "FFFFFFFDFFFFFFFFFFFFFFFFFFFFFFFF";
	static inline const modint_t N = xmodp("3FFFFFFF7FFFFFFFBE0024720613B5A3");
	static inline const auto ECParam = WeierstrassParameter<modint_t>(
		xmodp("D6031998D1B3BBFEBF59CC9BBFF9AEE1"),
		xmodp("5EEEFCA380D02919DC2C6558BB6D8A5D")
	);
	static inline const affin_t::Factory affin = ECParam;
	static inline const projective_t::Factory projective = ECParam;
	static inline const affin_t G = affin(
		xmodp("7B6AA5D85E572983E6FB32A7CDEBC140"),
		xmodp("27B6916A894D3AEE7106FE805FC34B44")
	);
};

template<>
struct secpParameters<secp160k1> : public secpTraits<3> {
	static inline const modint_t::Factory xmodp = "00FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFEFFFFAC73";
	static inline const modint_t N = xmodp("0100000000000000000001B8FA16DFAB9ACA16B6B3");
	static inline const auto ECParam = WeierstrassParameter<modint_t>(
		xmodp("0"),
		xmodp("7")
	);
	static inline const affin_t::Factory affin = ECParam;
	static inline const projective_t::Factory projective = ECParam;
	static inline const affin_t G = affin(
		xmodp("003B4C382CE37AA192A4019E763036F4F5DD4D7EBB"),
		xmodp("00938CF935318FDCED6BC28286531733C3F03C4FEE")
	);
};

template<>
struct secpParameters<secp160r1> : public secpTraits<3> {
	static inline const modint_t::Factory xmodp = "00FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF7FFFFFFF";
	static inline const modint_t N = xmodp("0100000000000000000001F4C8F927AED3CA752257");
	static inline const auto ECParam = WeierstrassParameter<modint_t>(
		xmodp("00FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF7FFFFFFC"),
		xmodp("001C97BEFC54BD7A8B65ACF89F81D4D4ADC565FA45")
	);
	static inline const affin_t::Factory affin = ECParam;
	static inline const projective_t::Factory projective = ECParam;
	static inline const affin_t G = affin(
		xmodp("004A96B5688EF573284664698968C38BB913CBFC82"),
		xmodp("0023A628553168947D59DCC912042351377AC5FB32")
	);
};

template<>
struct secpParameters<secp160r2> : public secpTraits<3> {
	static inline const modint_t::Factory xmodp = "00FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFEFFFFAC73";
	static inline const modint_t N = xmodp("0100000000000000000000351EE786A818F3A1A16B");
	static inline const auto ECParam = WeierstrassParameter<modint_t>(
		xmodp("00FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFEFFFFAC70"),
		xmodp("00B4E134D3FB59EB8BAB57274904664D5AF50388BA")
	);
	static inline const affin_t::Factory affin = ECParam;
	static inline const projective_t::Factory projective = ECParam;
	static inline const affin_t G = affin(
		xmodp("0052DCB034293A117E1F4FF11B30F7199D3144CE6D"),
		xmodp("00FEAFFEF2E331F296E071FA0DF9982CFEA7D43F2E")
	);
};

template<>
struct secpParameters<secp192k1> : public secpTraits<3> {
	static inline const modint_t::Factory xmodp = "FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFEFFFFEE37";
	static inline const modint_t N = xmodp("FFFFFFFFFFFFFFFFFFFFFFFE26F2FC170F69466A74DEFD8D");
	static inline const auto ECParam = WeierstrassParameter<modint_t>(
		xmodp("0"),
		xmodp("3")
	);
	static inline const affin_t::Factory affin = ECParam;
	static inline const projective_t::Factory projective = ECParam;
	static inline const affin_t G = affin(
		xmodp("DB4FF10EC057E9AE26B07D0280B7F4341DA5D1B1EAE06C7D"),
		xmodp("9B2F2F6D9C5628A7844163D015BE86344082AA88D95E2F9D")
	);
};

template<>
struct secpParameters<secp192r1> : public secpTraits<3> {
	static inline const modint_t::Factory xmodp = "FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFEFFFFFFFFFFFFFFFF";
	static inline const modint_t N = xmodp("FFFFFFFFFFFFFFFFFFFFFFFF99DEF836146BC9B1B4D22831");
	static inline const auto ECParam = WeierstrassParameter<modint_t>(
		xmodp("FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFEFFFFFFFFFFFFFFFC"),
		xmodp("64210519E59C80E70FA7E9AB72243049FEB8DEECC146B9B1")
	);
	static inline const affin_t::Factory affin = ECParam;
	static inline const projective_t::Factory projective = ECParam;
	static inline const affin_t G = affin(
		xmodp("188DA80EB03090F67CBF20EB43A18800F4FF0AFD82FF1012"),
		xmodp("07192B95FFC8DA78631011ED6B24CDD573F977A11E794811")
	);
};

template<>
struct secpParameters<secp224k1> : public secpTraits<4> {
	static inline const modint_t::Factory xmodp = "00FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFEFFFFE56D";
	static inline const modint_t N = xmodp("010000000000000000000000000001DCE8D2EC6184CAF0A971769FB1F7");
	static inline const auto ECParam = WeierstrassParameter<modint_t>(
		xmodp("0"),
		xmodp("5")
	);
	static inline const affin_t::Factory affin = ECParam;
	static inline const projective_t::Factory projective = ECParam;
	static inline const affin_t G = affin(
		xmodp("00A1455B334DF099DF30FC28A169A467E9E47075A90F7E650EB6B7A45C"),
		xmodp("007E089FED7FBA344282CAFBD6F7E319F7C0B0BD59E2CA4BDB556D61A5")
	);
};

template<>
struct secpParameters<secp224r1> : public secpTraits<4> {
	static inline const modint_t::Factory xmodp = "FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF000000000000000000000001";
	static inline const modint_t N = xmodp("FFFFFFFFFFFFFFFFFFFFFFFFFFFF16A2E0B8F03E13DD29455C5C2A3D");
	static inline const auto ECParam = WeierstrassParameter<modint_t>(
		xmodp("FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFEFFFFFFFFFFFFFFFFFFFFFFFE"),
		xmodp("B4050A850C04B3ABF54132565044B0B7D7BFD8BA270B39432355FFB4")
	);
	static inline const affin_t::Factory affin = ECParam;
	static inline const projective_t::Factory projective = ECParam;
	static inline const affin_t G = affin(
		xmodp("B70E0CBD6BB4BF7F321390B94A03C1D356C21122343280D6115C1D21"),
		xmodp("BD376388B5F723FB4C22DFE6CD4375A05A07476444D5819985007E34")
	);
};

template<>
struct secpParameters<secp256k1> : public secpTraits<4> {
	static inline const modint_t::Factory xmodp = "FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFEFFFFFC2F";
	static inline const modint_t N = xmodp("FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFEBAAEDCE6AF48A03BBFD25E8CD0364141");
	static inline const auto ECParam = WeierstrassParameter<modint_t>(
		xmodp("0"),
		xmodp("7")
	);
	static inline const affin_t::Factory affin = ECParam;
	static inline const projective_t::Factory projective = ECParam;
	static inline const affin_t G = affin(
		xmodp("79BE667EF9DCBBAC55A06295CE870B07029BFCDB2DCE28D959F2815B16F81798"),
		xmodp("483ADA7726A3C4655DA4FBFC0E1108A8FD17B448A68554199C47D08FFB10D4B8")
	);
};

template<>
struct secpParameters<secp256r1> : public secpTraits<4> {
public:
	static inline const modint_t::Factory xmodp = "ffffffff00000001000000000000000000000000ffffffffffffffffffffffff";
	static inline const modint_t N = xmodp("ffffffff00000000ffffffffffffffffbce6faada7179e84f3b9cac2fc632551");
	static inline const auto ECParam = WeierstrassParameter<modint_t>(
		xmodp("ffffffff00000001000000000000000000000000fffffffffffffffffffffffc"),
		xmodp("5ac635d8aa3a93e7b3ebbd55769886bc651d06b0cc53b0f63bce3c3e27d2604b")
	);
	static inline const affin_t::Factory affin = ECParam;
	static inline const projective_t::Factory projective = ECParam;
	static inline const affin_t G = affin(
		xmodp("6b17d1f2e12c4247f8bce6e563a440f277037d812deb33a0f4a13945d898c296"),
		xmodp("4fe342e2fe1a7f9b8ee7eb4a7c0f9e162bce33576b315ececbb6406837bf51f5")
	);
};

template<>
struct secpParameters<secp384r1> : public secpTraits<6> {
	static inline const modint_t::Factory xmodp = "FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFEFFFFFFFF0000000000000000FFFFFFFF";
	static inline const modint_t N = xmodp("FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFC7634D81F4372DDF581A0DB248B0A77AECEC196ACCC52973");
	static inline const auto ECParam = WeierstrassParameter<modint_t>(
		xmodp("FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFEFFFFFFFF0000000000000000FFFFFFFC"),
		xmodp("B3312FA7E23EE7E4988E056BE3F82D19181D9C6EFE8141120314088F5013875AC656398D8A2ED19D2A85C8EDD3EC2AEF")
	);
	static inline const affin_t::Factory affin = ECParam;
	static inline const projective_t::Factory projective = ECParam;
	static inline const affin_t G = affin(
		xmodp("AA87CA22BE8B05378EB1C71EF320AD746E1D3B628BA79B9859F741E082542A385502F25DBF55296C3A545E3872760AB7"),
		xmodp("3617DE4A96262C6F5D9E98BF9292DC29F8F41DBD289A147CE9DA3113B5F0B8C00A60B1CE1D7E819D7A431D7C90EA0E5F")
	);
};

template<>
struct secpParameters<secp521r1> : public secpTraits<9> {
	static inline const modint_t::Factory xmodp = "01FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF";
	static inline const modint_t N = xmodp("01FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFA51868783BF2F966B7FCC0148F709A5D03BB5C9B8899C47AEBB6FB71E91386409");
	static inline const auto ECParam = WeierstrassParameter<modint_t>(
		xmodp("01FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFC"),
		xmodp("0051953EB9618E1C9A1F929A21A0B68540EEA2DA725B99B315F3B8B489918EF109E156193951EC7E937B1652C0BD3BB1BF073573DF883D2C34F1EF451FD46B503F00")
	);
	static inline const affin_t::Factory affin = ECParam;
	static inline const projective_t::Factory projective = ECParam;
	static inline const affin_t G = affin(
		xmodp("00C6858E06B70404E9CD9E3ECB662395B4429C648139053FB521F828AF606B4D3DBAA14B5E77EFE75928FE1DC127A2FFA8DE3348B3C1856A429BF97E7E31C2E5BD66"),
		xmodp("011839296A789A3BC0045C8A5FB42C7D1BD998F54449579B446817AFBD17273E662C97EE72995EF42640C550B9013FAD0761353C7086A272C24088BE94769FD16650")
	);
};

template<class Param>
class basic_ECDSA : public secpParameters<Param> {
public:

	using Super = secpParameters<Param>;

	using types = secpTraits<Super::baseint_t::Words>;

	using baseint_t = types::baseint_t;
	using int_t = types::int_t;
	using modint_t = types::modint_t;
	using affin_t = types::affin_t;
	using projective_t = types::projective_t;

	using bytearray = Cryptgraphy::bytearray;

	static constexpr bytearray(*hasher)(const bytearray&, size_t) = &SHAKE256::HasherN;
	static inline RandomGenerator rd{};
	static inline const modint_t::Factory xmodn = Super::N.value;

	static affin_t MakePublicKey(const baseint_t& privateKey) {
		auto q = projective_t(Super::G).Scaler(xmodn(privateKey)).ToAffin();
		return q;
	}

	static bytearray Sign(const baseint_t& privateKey, const bytearray& message) {
		auto h = hasher(privateKey.ToBytes(), 64);
		auto&& e = xmodn(hasher(message, 32));
		auto&& sk = bytearray(h.begin() + 32, h.end());

		sk.reserve(sk.size() + message.size());
		sk.insert(sk.end(), message.begin(), message.end());

		auto k = xmodn(hasher(sk, 32));
		
		auto&& r = xmodn(projective_t(Super::G).Scaler(k).ToAffin().x.value);
		auto s = (e + r * xmodn(privateKey)) / k.value;
		
		auto _r = baseint_t(r.value).ToBytes();
		auto _s = baseint_t(s.value).ToBytes();
		
		bytearray ret;
		ret.reserve(64);
		ret.insert(ret.end(), _r.begin(), _r.end());
		ret.insert(ret.end(), _s.begin(), _s.end());
		
		return ret;
	}

	static bool Verify(const affin_t& q, const bytearray& v, const bytearray& message) {
		auto&& e = xmodn(hasher(message, 32));

		auto&& r = xmodn(bytearray{v.begin(), v.begin() + 32});
		auto&& s = xmodn(bytearray{v.begin() + 32, v.end()});

		auto u1 = projective_t(Super::G).Scaler(xmodn((e / s).value));
		auto u2 = projective_t(q).Scaler(xmodn((r / s).value));

		auto&& ret = u1.Add(u2).ToAffin().x % Super::N;

		return r == ret;
	}
};

using ECDSA = basic_ECDSA<secp256r1>;
