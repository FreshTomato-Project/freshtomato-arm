
#include "sim.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

int main() {
	int ret;
	uint8_t imsi_ef[] = { 0x08, 0x29, 0x82, 0x60, 0x82, 0x00, 0x00, 0x20, 0x80 };
	char imsi_str[16];
	ret = uqmi_sim_decode_imsi(&imsi_ef[0], 9, &imsi_str[0], sizeof(imsi_str));
	if (ret >= 0)
		fprintf(stderr, "Decoded imsi %s\n", imsi_str);
	assert(ret >= 0);
	assert(strncmp(&imsi_str[0], "228062800000208", sizeof(15)) == 0);
}
