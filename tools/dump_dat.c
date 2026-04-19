#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

// main_dat
extern const uint16_t *danger_spr_data;
extern const uint16_t *panel_spr_data;
extern const uint16_t *mfd1_spr_data;
extern const uint16_t *mfd2_spr_data;
extern const uint16_t *mfd3_spr_data;
extern const uint16_t *compas_spr_data;
extern const uint16_t *curse_spr_data;
extern const uint32_t *lamp1_spr_data[]; // 2 entries
extern const uint32_t *lamp2_spr_data; // 2
extern const uint32_t *holo_spr_data; // 2? (same file)
extern const uint32_t *noise_spr_data; // 4
extern const uint32_t *flm1_spr_data; // 6
extern const uint32_t *flm2_spr_data; // 8
extern const uint32_t *flm3_spr_data; // 12
extern const uint32_t *flm4_spr_data; // 7
extern const uint32_t *chip1_spr_data; // 7
extern const uint32_t *chip2_spr_data; // 6
extern const uint32_t *dust1_spr_data; // 6
extern const uint32_t *smk1_spr_data; // 10
extern const uint32_t *tbomb_spr_data; // 2
extern const uint32_t *mine_spr_data; // 2
extern const uint32_t *AP_spr_data; // special

// en_dat
extern const uint32_t *en1_spr_data; // 25 entries
extern const uint32_t *en2_spr_data; // 25
extern const uint32_t *en3_spr_data; // 13
extern const uint32_t *en4_spr_data; // 17
extern const uint32_t *tank1_spr_data; // 3
extern const uint32_t *tank2_spr_data; // 3
extern const uint32_t *tank3_spr_data; // 3
extern const uint32_t *ttop1_spr_data; // 7
extern const uint32_t *ttop2_spr_data; // 11
extern const uint32_t *ttop3_spr_data; // 7

void hex(const void* data, size_t size) {
	char ascii[17];
	size_t i, j;
	ascii[16] = '\0';
	for (i = 0; i < size; ++i) {
		printf("%02X ", ((unsigned char*)data)[i]);
		if (((unsigned char*)data)[i] >= ' ' && ((unsigned char*)data)[i] <= '~') {
			ascii[i % 16] = ((unsigned char*)data)[i];
		} else {
			ascii[i % 16] = '.';
		}
		if ((i+1) % 8 == 0 || i+1 == size) {
			printf(" ");
			if ((i+1) % 16 == 0) {
				printf("|  %s \n", ascii);
			} else if (i+1 == size) {
				ascii[(i+1) % 16] = '\0';
				if ((i+1) % 16 <= 8) {
					printf(" ");
				}
				for (j = (i+1) % 16; j < 16; ++j) {
					printf("   ");
				}
				printf("|  %s \n", ascii);
			}
		}
	}
}

bool dump(const char *name, const void *data, size_t size) {
	bool res = false;
	char fullname[64];

	snprintf(fullname, sizeof(fullname) - 1, "dump/%s", name);
	FILE* fp = fopen(fullname, "wb");
	if(!fp) return res;
	size_t written = fwrite(data, 1, size, fp);
	if(written == size) {
		res = true;
	} else {
		printf("%s - size mismatch: %u != %u\n", name, written, size);
	}
	fclose(fp);
	return res;
}

bool extract_single_sprite(const char *name, const void *data) {
	//hex(data, 8);
	const unsigned char *d = data;
	int16_t w = d[0] | (d[1] << 8);
	int16_t h = d[2] | (d[3] << 8);

	size_t size = w * h + 4;
	printf("%s, size: %dx%d=%u\n", name, w, h, size);
	if(size <= 0 || size >= SIZE_MAX) {
		printf("Wrong dimensions\n");
		return false;
	}

	return dump(name, data, size);
}

bool extract_multi_sprite(const char *name, const void *data, size_t count) {
	uintptr_t *d = (uintptr_t *)data;
	char n[20];
	for(int i = 0; i < count; i++ ) {
		snprintf(n, sizeof(n) - 1, name, i + 1);
		bool res = extract_single_sprite(n, (void*)*d);
		if (!res) {
			printf("%s: failed at frame\n", n);
			return false;
		}
		d++;
	}

	return true;
}

int main(int argc, char *argv[]) {
	int res = 0;

	#define CHECK(x) if(!x) res++;

	CHECK(extract_single_sprite("danger.spr", &danger_spr_data))
	CHECK(extract_single_sprite("panel.spr", &panel_spr_data))
	CHECK(extract_single_sprite("mfd1.spr", &mfd1_spr_data))
	CHECK(extract_single_sprite("mfd2.spr", &mfd2_spr_data))
	CHECK(extract_single_sprite("mfd3.spr", &mfd3_spr_data))
	CHECK(extract_single_sprite("compas.spr", &compas_spr_data))
	CHECK(extract_single_sprite("curse.spr", &curse_spr_data))
	CHECK(extract_multi_sprite("lamp1%02d.spr", &lamp1_spr_data, 2))
	CHECK(extract_multi_sprite("lamp2%02d.spr", &lamp2_spr_data, 2))
	CHECK(extract_multi_sprite("holo%02d.spr", &holo_spr_data, 2))
	CHECK(extract_multi_sprite("noise%02d.spr", &noise_spr_data, 4))
	CHECK(extract_multi_sprite("flm1%02d.spr", &flm1_spr_data, 6))
	CHECK(extract_multi_sprite("flm2%02d.spr", &flm2_spr_data, 8))
	CHECK(extract_multi_sprite("flm3%02d.spr", &flm3_spr_data, 12))
	CHECK(extract_multi_sprite("flm4%02d.spr", &flm4_spr_data, 7))
	CHECK(extract_multi_sprite("chip1%02d.spr", &chip1_spr_data, 7))
	CHECK(extract_multi_sprite("chip2%02d.spr", &chip2_spr_data, 6))
	CHECK(extract_multi_sprite("dust1%02d.spr", &dust1_spr_data, 6))
	CHECK(extract_multi_sprite("smk1%02d.spr", &smk1_spr_data, 10))
	CHECK(extract_multi_sprite("tbomb%02d.spr", &tbomb_spr_data, 2))
	CHECK(extract_multi_sprite("mine%02d.spr", &mine_spr_data, 2))
	CHECK(extract_multi_sprite("lasa1%02d.spr", &AP_spr_data, 8))
	CHECK(extract_multi_sprite("lasa2%02d.spr", &AP_spr_data+8, 8))
	CHECK(extract_multi_sprite("lasa3%02d.spr", &AP_spr_data+16, 8))
	CHECK(extract_multi_sprite("lasa4%02d.spr", &AP_spr_data+24, 8))
	CHECK(extract_multi_sprite("apb%02d.spr", &AP_spr_data+32, 6))
	CHECK(extract_multi_sprite("lasa5%02d.spr", &AP_spr_data+38, 7))

	CHECK(extract_multi_sprite("en1%02d.spr", &en1_spr_data, 25))
	CHECK(extract_multi_sprite("en2%02d.spr", &en2_spr_data, 25))
	CHECK(extract_multi_sprite("en3%02d.spr", &en3_spr_data, 13))
	CHECK(extract_multi_sprite("en4%02d.spr", &en4_spr_data, 17))
	CHECK(extract_multi_sprite("tank1%02d.spr", &tank1_spr_data, 3))
	CHECK(extract_multi_sprite("tank2%02d.spr", &tank2_spr_data, 3))
	CHECK(extract_multi_sprite("tank3%02d.spr", &tank3_spr_data, 3))
	CHECK(extract_multi_sprite("ttop1%02d.spr", &ttop1_spr_data, 7))
	CHECK(extract_multi_sprite("ttop2%02d.spr", &ttop2_spr_data, 11))
	CHECK(extract_multi_sprite("ttop3%02d.spr", &ttop3_spr_data, 7))

	if(res == 0) {
		printf("ok\n");
	} else {
		printf("%d failed\n", res);
	}

	return 0;
}
