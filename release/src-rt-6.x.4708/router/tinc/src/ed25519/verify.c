#include "ed25519.h"
#include "sha512.h"
#include "ge.h"
#include "sc.h"

#include "../utils.h"

int ed25519_verify(const unsigned char *signature, const unsigned char *message, size_t message_len, const unsigned char *public_key) {
	unsigned char h[64];
	unsigned char checker[32];
	sha512_context hash;
	ge_p3 A;
	ge_p2 R;

	if(signature[63] & 224) {
		return 0;
	}

	if(ge_frombytes_negate_vartime(&A, public_key) != 0) {
		return 0;
	}

	sha512_init(&hash);
	sha512_update(&hash, signature, 32);
	sha512_update(&hash, public_key, 32);
	sha512_update(&hash, message, message_len);
	sha512_final(&hash, h);

	sc_reduce(h);
	ge_double_scalarmult_vartime(&R, h, &A, signature + 32);
	ge_tobytes(checker, &R);

	if(!mem_eq(checker, signature, sizeof checker)) {
		return 0;
	}

	return 1;
}
