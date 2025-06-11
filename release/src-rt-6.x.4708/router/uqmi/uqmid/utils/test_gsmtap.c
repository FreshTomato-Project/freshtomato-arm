#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "../gsmtap_util.h"
#include <talloc.h>

struct modem {
	int foo;
};

int main() {
	uint8_t data[1024] = { 0, 1, 2, 3, 4 };
	int ret = gsmtap_enable("127.0.0.1");
	printf("gsmtap enable: %d\n", ret);
	gsmtap_send(NULL, data, sizeof(data));

	return 0;
}
