#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <stddef.h>

#define TRX_MAGIC           0x30524448  /* "HDR0" */
#define NETGEAR_MAGIC       0x5E24232A  /* Netgear CHK */
#define TRX_MAX_LEN         0x4000000   /* 64MB */

static uint32_t crc32_tab[256];
static int verbose = 0;

static void init_crc32(void) {
    uint32_t i, j, c;
    for (i = 0; i < 256; i++) {
        for (c = i, j = 8; j > 0; j--)
            c = (c & 1) ? (0xEDB88320L ^ (c >> 1)) : (c >> 1);
        crc32_tab[i] = c;
    }
}

static uint32_t crc32_calc(uint32_t crc, const uint8_t *buf, size_t len) {
    while (len--) crc = crc32_tab[(crc ^ *buf++) & 0xFF] ^ (crc >> 8);
    return crc;
}

struct trx_header {
    uint32_t magic, len, crc32, flag_version, offsets[3];
};

static int perform_check(const char *fn) {
    int fd, res = 1;
    struct stat st;
    uint8_t *buf;

    if (!fn) return 1;
    fd = open(fn, O_RDONLY);
    if (fd < 0) {
        fprintf(stderr, "ERROR: Cannot open %s\n", fn);
        return 4;
    }
    
    if (fstat(fd, &st) < 0 || st.st_size < 32) {
        close(fd);
        return 3;
    }

    buf = mmap(NULL, st.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
    if (buf == MAP_FAILED) {
        close(fd);
        return 4;
    }

    if (verbose) {
        printf("\nAnalyzing File: %s\n", fn);
        printf("Size: %zu bytes\n", (size_t)st.st_size);
    }

    uint32_t magic = *(uint32_t *)buf;

    /* 1. TRX Check */
    if (magic == TRX_MAGIC) {
        struct trx_header *h = (struct trx_header *)buf;
        if (verbose) {
            printf("Type: TRX (Broadcom)\n");
            printf("  Magic: 0x%08x (HDR0)\n", h->magic);
            printf("  Header Length: %u\n", h->len);
        }

        if (h->len > st.st_size || h->len > TRX_MAX_LEN) {
            if (verbose) printf("  ERROR: Length mismatch or exceeds 64MB limit\n");
            res = 3;
        } else {
            init_crc32();
            uint32_t calc = crc32_calc(0xFFFFFFFF, (uint8_t *)&h->flag_version, h->len - 12);
            if (verbose) {
                printf("  Expected CRC:   0x%08x\n", h->crc32);
                printf("  Calculated CRC: 0x%08x\n", calc);
            }
            res = (calc == h->crc32) ? 0 : 2;
        }
    } 
    /* 2. CHK Check */
    else if (magic == NETGEAR_MAGIC) {
        if (verbose) printf("Type: Netgear CHK (Verified by Magic)\n");
        res = 0; 
    } 
    /* 3. BIN/Generic Check */
    else {
        int found = 0;
        for (int i = 0; i < 512 && i < st.st_size - 32; i++) {
            if (memcmp(buf + i, "U2ND", 4) == 0 || memcmp(buf + i, "HDR0", 4) == 0) {
                if (verbose) printf("Type: BIN (Linksys/ASUS) - Identifier found at offset %d\n", i);
                found = 1;
                res = 0;
                break;
            }
        }
        if (!found && verbose) printf("Type: Unknown (Magic: 0x%08x)\n", magic);
    }

    /* Final Output */
    if (!verbose) {
        printf("%s: %s\n", fn, (res == 0) ? "VALID" : "INVALID");
    } else {
        printf("Final Result: %s\n------------------------------\n", (res == 0) ? "✓ VALID" : "✗ INVALID");
    }

    munmap(buf, st.st_size);
    close(fd);
    return res;
}

int trx_check_main(int argc, char *argv[]) {
    int ch, i;
    int first_file_idx = -1;
    verbose = 0;
    optind = 0;

    while ((ch = getopt(argc, argv, "vhi:")) != -1) {
        switch (ch) {
            case 'v': verbose = 1; break;
            case 'i': first_file_idx = optind - 1; break;
            case 'h': 
            default:
                printf("TRX/CHK/BIN Firmware Checker\n");
                printf("Usage: trx_check [-v] -i <file1> [file2...]\n");
                return 0;
        }
    }

    if (first_file_idx != -1) {
        for (i = first_file_idx; i < argc; i++) {
            if (strcmp(argv[i], "-i") == 0) continue;
            perform_check(argv[i]);
        }
    } else {
        printf("Error: Use -i to specify firmware files.\n");
    }

    return 0;
}
