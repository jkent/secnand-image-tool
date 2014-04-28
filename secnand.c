/*
 * TODO: description
 *
 * Copyright (C) 2014 Jeff Kent <jeff@jkent.net>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 as published by
 * the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc., 51
 * Franklin St, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 */

#include <errno.h>
#include <fcntl.h>
#include <getopt.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include "bch.h"


#define SZ_1K (1024)

#define BCH_POLY 0x201B

#define NAND_MAX_ECCBYTES      26
#define NAND_SECTOR_SIZE      512
#define NAND_MAX_ECCDATA_SIZE 416
#define NAND_MAX_OOB_SIZE     640

struct nand_oobfree {
	uint32_t offset;
	uint32_t length;
};

struct nand_ecclayout {
	uint32_t eccbytes;
	uint32_t eccpos[NAND_MAX_OOB_SIZE];
    uint32_t oobavail;
	struct nand_oobfree oobfree[1];
};

static struct nand_ecclayout nand_oob_bl1 = {
	.eccbytes = 208,
	.eccpos = {
		 12,  13,  14,  15,  16,  17,  18,  19,
		 20,  21,  22,  23,  24,  25,  26,  27,
		 28,  29,  30,  31,  32,  33,  34,  35,
		 36,  37,  38,  39,  40,  41,  42,  43,
		 44,  45,  46,  47,  48,  49,  50,  51,
		 52,  53,  54,  55,  56,  57,  58,  59,
		 60,  61,  62,  63,  64,  65,  66,  67,
		 68,  69,  70,  71,  72,  73,  74,  75,
		 76,  77,  78,  79,  80,  81,  82,  83,
		 84,  85,  86,  87,  88,  89,  90,  91,
		 92,  93,  94,  95,  96,  97,  98,  99,
		100, 101, 102, 103, 104, 105, 106, 107,
		108, 109, 110, 111, 112, 113, 114, 115,
		116, 117, 118, 119, 120, 121, 122, 123,
		124, 125, 126, 127, 128, 129, 130, 131,
		132, 133, 134, 135, 136, 137, 138, 139,
		140, 141, 142, 143, 144, 145, 146, 147,
		148, 149, 150, 151, 152, 153, 154, 155,
		156, 157, 158, 159, 160, 161, 162, 163,
		164, 165, 166, 167, 168, 169, 170, 171,
		172, 173, 174, 175, 176, 177, 178, 179,
		180, 181, 182, 183, 184, 185, 186, 187,
		188, 189, 190, 191, 192, 193, 194, 195,
		196, 197, 198, 199, 200, 201, 202, 203,
		204, 205, 206, 207, 208, 209, 210, 211,
		212, 213, 214, 215, 216, 217, 218, 219},
	.oobfree = {
 		{.offset = 2,
		.length = 10} }
};

static struct nand_ecclayout nand_oob_64 = {
	.eccbytes = 24,
	.eccpos = {
		 40,  41,  42,  43,  44,  45,  46,  47,
		 48,  49,  50,  51,  52,  53,  54,  55,
		 56,  57,  58,  59,  60,  61,  62,  63},
	.oobfree = {
 		{.offset = 2,
		.length = 38} }
};

static struct nand_ecclayout nand_oob_512 = {
	.eccbytes = 442,
	.eccpos = {
		 36,  37,  38,  39,  40,  41,  42,  43,
		 44,  45,  46,  47,  48,  49,  50,  51,
		 52,  53,  54,  55,  56,  57,  58,  59,
		 60,  61,  64,  65,  66,  67,  68,  69,
		 70,  71,  72,  73,  74,  75,  76,  77,
		 78,  79,  80,  81,  82,  83,  84,  85,
		 86,  87,  88,  89,  92,  93,  94,  95,
		 96,  97,  98,  99, 100, 101, 102, 103,
		104, 105, 106, 107, 108, 109, 110, 111,
		112, 113, 114, 115, 116, 117, 120, 121,
		122, 123, 124, 125, 126, 127, 128, 129,
		130, 131, 132, 133, 134, 135, 136, 137,
		138, 139, 140, 141, 142, 143, 144, 145,
		148, 149, 150, 151, 152, 153, 154, 155,
		156, 157, 158, 159, 160, 161, 162, 163,
		164, 165, 166, 167, 168, 169, 170, 171,
		172, 173, 176, 177, 178, 179, 180, 181,
		182, 183, 184, 185, 186, 187, 188, 189,
		190, 191, 192, 193, 194, 195, 196, 197,
		198, 199, 200, 201, 204, 205, 206, 207,
		208, 209, 210, 211, 212, 213, 214, 215,
		216, 217, 218, 219, 220, 221, 222, 223,
		224, 225, 226, 227, 228, 229, 232, 233,
		234, 235, 236, 237, 238, 239, 240, 241,
		242, 243, 244, 245, 246, 247, 248, 249,
		250, 251, 252, 253, 254, 255, 256, 257,
		260, 261, 262, 263, 264, 265, 266, 267,
		268, 269, 270, 271, 272, 273, 274, 275,
		276, 277, 278, 279, 280, 281, 282, 283,
		284, 285, 288, 289, 290, 291, 292, 293,
		294, 295, 296, 297, 298, 299, 300, 301,
		302, 303, 304, 305, 306, 307, 308, 309,
		310, 311, 312, 313, 316, 317, 318, 319,
		320, 321, 322, 323, 324, 325, 326, 327,
		328, 329, 330, 331, 332, 333, 334, 335,
		336, 337, 338, 339, 340, 341, 344, 345,
		346, 347, 348, 349, 350, 351, 352, 353,
		354, 355, 356, 357, 358, 359, 360, 361,
		362, 363, 364, 365, 366, 367, 368, 369,
		372, 373, 374, 375, 376, 377, 378, 379,
		380, 381, 382, 383, 384, 385, 386, 387,
		388, 389, 390, 391, 392, 393, 394, 395,
		396, 397, 400, 401, 402, 403, 404, 405,
		406, 407, 408, 409, 410, 411, 412, 413,
		414, 415, 416, 417, 418, 419, 420, 421,
		422, 423, 424, 425, 428, 429, 430, 431,
		432, 433, 434, 435, 436, 437, 438, 439,
		440, 441, 442, 443, 444, 445, 446, 447,
		448, 449, 450, 451, 452, 453, 456, 457,
		458, 459, 460, 461, 462, 463, 464, 465,
		466, 467, 468, 469, 470, 471, 472, 473,
		474, 475, 476, 477, 478, 479, 480, 481,
		484, 485, 486, 487, 488, 489, 490, 491,
		492, 493, 494, 495, 496, 497, 498, 499,
		500, 501, 502, 503, 504, 505, 506, 507,
		508, 509},
	.oobfree = {
		{.offset = 4,
		.length = 32 } }
};

static struct nand_ecclayout nand_oob_640 = {
	.eccbytes = 442,
	.eccpos = {
		 40,  41,  42,  43,  44,  45,  46,  47,
		 48,  49,  50,  51,  52,  53,  54,  55,
		 56,  57,  58,  59,  60,  61,  62,  63,
		 64,  65,  68,  69,  70,  71,  72,  73,
		 74,  75,  76,  77,  78,  79,  80,  81,
		 82,  83,  84,  85,  86,  87,  88,  89,
		 90,  91,  92,  93,  96,  97,  98,  99,
		100, 101, 102, 103, 104, 105, 106, 107,
		108, 109, 110, 111, 112, 113, 114, 115,
		116, 117, 118, 119, 120, 121, 124, 125,
		126, 127, 128, 129, 130, 131, 132, 133,
		134, 135, 136, 137, 138, 139, 140, 141,
		142, 143, 144, 145, 146, 147, 148, 149,
		152, 153, 154, 155, 156, 157, 158, 159,
		160, 161, 162, 163, 164, 165, 166, 167,
		168, 169, 170, 171, 172, 173, 174, 175,
		176, 177, 180, 181, 182, 183, 184, 185,
		186, 187, 188, 189, 190, 191, 192, 193,
		194, 195, 196, 197, 198, 199, 200, 201,
		202, 203, 204, 205, 208, 209, 210, 211,
		212, 213, 214, 215, 216, 217, 218, 219,
		220, 221, 222, 223, 224, 225, 226, 227,
		228, 229, 230, 231, 232, 233, 236, 237,
		238, 239, 240, 241, 242, 243, 244, 245,
		246, 247, 248, 249, 250, 251, 252, 253,
		254, 255, 256, 257, 258, 259, 260, 261,
		264, 265, 266, 267, 268, 269, 270, 271,
		272, 273, 274, 275, 276, 277, 278, 279,
		280, 281, 282, 283, 284, 285, 286, 287,
		288, 289, 292, 293, 294, 295, 296, 297,
		298, 299, 300, 301, 302, 303, 304, 305,
		306, 307, 308, 309, 310, 311, 312, 313,
		314, 315, 316, 317, 320, 321, 322, 323,
		324, 325, 326, 327, 328, 329, 330, 331,
		332, 333, 334, 335, 336, 337, 338, 339,
		340, 341, 342, 343, 344, 345, 348, 349,
		350, 351, 352, 353, 354, 355, 356, 357,
		358, 359, 360, 361, 362, 363, 364, 365,
		366, 367, 368, 369, 370, 371, 372, 373,
		376, 377, 378, 379, 380, 381, 382, 383,
		384, 385, 386, 387, 388, 389, 390, 391,
		392, 393, 394, 395, 396, 397, 398, 399,
		400, 401, 404, 405, 406, 407, 408, 409,
		410, 411, 412, 413, 414, 415, 416, 417,
		418, 419, 420, 421, 422, 423, 424, 425,
		426, 427, 428, 429, 432, 433, 434, 435,
		436, 437, 438, 439, 440, 441, 442, 443,
		444, 445, 446, 447, 448, 449, 450, 451,
		452, 453, 454, 455, 456, 457, 460, 461,
		462, 463, 464, 465, 466, 467, 468, 469,
		470, 471, 472, 473, 474, 475, 476, 477,
		478, 479, 480, 481, 482, 483, 484, 485,
		488, 489, 490, 491, 492, 493, 494, 495,
		496, 497, 498, 499, 500, 501, 502, 503,
		504, 505, 506, 507, 508, 509, 510, 511,
		512, 513},
	.oobfree = {
		{.offset = 4,
		.length = 32 } }
};

static const char xor_bch8[13] = {
	0xEF, 0x51, 0x2E, 0x09, 0xED, 0x93, 0x9A, 0xC2,
	0x97, 0x79, 0xE5, 0x24, 0xB5
};

static const char xor_bch16[26] = {
	0x9A, 0xD7, 0xEF, 0x91, 0x88, 0x80, 0xFB, 0xF7,
	0x06, 0x3A, 0x5C, 0x9F, 0x49, 0x24, 0xD0, 0x75,
	0x02, 0xE3, 0x59, 0xE0, 0xE4, 0xBC, 0x1E, 0x20,
	0x70, 0x2E
};

static void xor_bch(struct bch_control *bch, unsigned char *ecc)
{
	int i;

	switch (bch->t) {
	case 8:
		for (i = 0; i < bch->ecc_bytes; i++)
			ecc[i] ^= xor_bch8[i];
		break;
	case 16:
		for (i = 0; i < bch->ecc_bytes; i++)
			ecc[i] ^= xor_bch16[i];
		break;
	}
}

static int gf_order(unsigned int poly)
{
	int m = 0;

	while (poly >>= 1)
		m++;

	return m;
}

/**
 * encode_1bit - Encode 1-bit ECC (4 byte output)
 * @buf:        input buffer with raw data
 * @eccsize:    input buffer length
 * @code:       output buffer with ECC
 */
void encode_1bit(const unsigned char *buf, unsigned int eccsize, unsigned char *code)
{
	unsigned int i, j;
	unsigned char byte, bytep;
	unsigned char colp = 0;
	unsigned int rowp = 0x3FFFFF;

	for (i = 0; i < eccsize; i++) {
		byte = buf[i];
		bytep = (0x6996 >> ((byte ^ byte >> 4) & 0xf)) & 1;
		colp ^= byte;
		for (j = 0; j < 11; j++) {
			if (i & (1 << j))
				rowp ^= bytep << ((2 * j) + 1);
			else
				rowp ^= bytep << (2 * j);
		}
	}

	code[0] = rowp & 0xFF;
	code[1] = (rowp >> 8) & 0xFF;
	code[2] = 0xFC | (rowp >> 16) & 0x03;
	code[3] = 0xF0 | (rowp >> 18) & 0x0F;

	if (((colp >> 7) & 1) ^ ((colp >> 6) & 1) ^ ((colp >> 5) & 1) ^ ((colp >> 4) & 1))
		code[2] &= ~(1 << 7);

	if (((colp >> 3) & 1) ^ ((colp >> 2) & 1) ^ ((colp >> 1) & 1) ^ ((colp >> 0) & 1))
		code[2] &= ~(1 << 6);

	if (((colp >> 7) & 1) ^ ((colp >> 6) & 1) ^ ((colp >> 3) & 1) ^ ((colp >> 2) & 1))
		code[2] &= ~(1 << 5);

	if (((colp >> 5) & 1) ^ ((colp >> 4) & 1) ^ ((colp >> 1) & 1) ^ ((colp >> 0) & 1))
		code[2] &= ~(1 << 4);

	if (((colp >> 7) & 1) ^ ((colp >> 5) & 1) ^ ((colp >> 3) & 1) ^ ((colp >> 1) & 1))
		code[2] &= ~(1 << 3);

	if (((colp >> 6) & 1) ^ ((colp >> 4) & 1) ^ ((colp >> 2) & 1) ^ ((colp >> 0) & 1))
		code[2] &= ~(1 << 2);
}

static void show_usage(const char *arg0)
{
	fprintf(stderr, "Usage: %s [OPTIONS] INPUTFILE OUTPUTFILE\n\n", arg0);

	fprintf(stderr, "  %-15s %s\n", "-b, --bl1ecc", "16k BL1, ECC strength of 8 or 16");
	fprintf(stderr, "  %-15s %s\n", "-e, --eccbits", "ECC strength of 1, 8 or 16  (required)");
	fprintf(stderr, "  %-15s %s\n", "-o, --oobsize", "NAND oob size, 64, 512 or 640 (required)");
	fprintf(stderr, "  %-15s %s\n", "-p, --pagesize", "NAND page size, either 2k or 8k (required)");
	exit(-1);
}

int main(int argc, char *argv[])
{
	unsigned int bl1ecc = 0;
	unsigned int eccbits = 0;
	unsigned int pagesize = 0;
	unsigned int oobsize = 0;
	unsigned char *inputfile;
	unsigned char *outputfile;
	unsigned int argerror = 0;

	struct bch_control *bch;
	int input_fd = 0;
	int output_fd = 0;
	uint8_t ecc[NAND_MAX_ECCBYTES];
	uint8_t sector[NAND_SECTOR_SIZE];
	uint8_t eccdata[NAND_MAX_ECCDATA_SIZE];
	uint8_t oobdata[NAND_MAX_OOB_SIZE];
	int eccsteps, ecctotal;
	int step;
	int i, j, n;
	int done = 0;
	uint32_t *eccpos;

	while (1) {
		static struct option long_options[] = {
			{"bl1ecc",   required_argument, 0, 'b'},
			{"eccbits",  required_argument, 0, 'e'},
			{"oobsize",  required_argument, 0, 'o'},
			{"pagesize", required_argument, 0, 'p'},
			{0,          0,                 0, 0  }
		};
		int option_index = 0;
		int c = getopt_long(argc, argv, "b:e:o:p:",
		                long_options, &option_index);
		if (c == -1)
			break;
 
		switch (c) {
		case 'b':
			if (bl1ecc) {
				fprintf(stderr, "%s: bl1ecc should only be specified once\n",
				        argv[0]);
				show_usage(argv[0]);
			}
			bl1ecc = strtoul(optarg, NULL, 10);
			if (bl1ecc != 8 && bl1ecc != 16) {
				fprintf(stderr, "%s: bl1ecc must be 8 or 16\n",
						argv[0]);
				show_usage(argv[0]);
			}
			break;

		case 'e':
			if (eccbits) {
				fprintf(stderr, "%s: eccbits should only be specified once\n",
				        argv[0]);
				show_usage(argv[0]);
			}
			eccbits = strtoul(optarg, NULL, 10);
			if (eccbits != 1 && eccbits != 8 && eccbits != 16) {
				fprintf(stderr, "%s: eccbits must be 1, 8, or 16\n",
						argv[0]);
				show_usage(argv[0]);
			}
			break;

		case 'o':
			if (oobsize) {
				fprintf(stderr, "%s: oobsize should only be specified once\n",
				        argv[0]);
				show_usage(argv[0]);
			}
			oobsize = strtoul(optarg, NULL, 10);
			if (oobsize != 64 && oobsize != 512 && oobsize != 640) {
				fprintf(stderr, "%s: oobsize must be 64, 512, or 640\n",
						argv[0]);
				show_usage(argv[0]);
			}
			break;

		case 'p':
			if (pagesize) {
				fprintf(stderr, "%s: pagesize should only be specified once\n",
				        argv[0]);
				show_usage(argv[0]);
			}
			if (strcasecmp("2k", optarg) == 0) {
				pagesize = 2*SZ_1K;
			}
			else if (strcasecmp("8k", optarg) == 0) {
				pagesize = 8*SZ_1K;
			}
			else {
				fprintf(stderr, "%s: pagesize must be 2k or 8k\n", argv[0]);
				show_usage(argv[0]);
			}
			break;
  
		case '?':
			exit(-1);
 
		default:
			abort();
		}
	}

	if (!pagesize) {
		fprintf(stderr, "%s: pagesize must be specified\n", argv[0]);
		argerror = 1;
	}

	if (!oobsize) {
		fprintf(stderr, "%s: oobsize must be specified\n", argv[0]);
		argerror = 1;
	}

	if (!eccbits) {
		fprintf(stderr, "%s: eccbits must be specified\n", argv[0]);
		argerror = 1;
	}

	if (argc - optind < 1) {
		fprintf(stderr, "%s: input file must be specified\n", argv[0]);
		argerror = 1;
	}

	if (argc - optind < 2) {
		fprintf(stderr, "%s: output file must be specified\n", argv[0]);
		argerror = 1;
	}

	if (argc - optind > 2) {
		fprintf(stderr, "%s: too many arguments\n", argv[0]);
		argerror = 1;
	}
		
	if (argerror) {
		show_usage(argv[0]);
	}

	if (pagesize == 2*SZ_1K && oobsize != 64) {
		fprintf(stderr, "%s: invalid oobsize for 2k page\n", argv[0]);
		show_usage(argv[0]);
	}

	if (pagesize == 8*SZ_1K && (oobsize != 512 && oobsize != 640)) {
		fprintf(stderr, "%s: invalid oobsize for 8k page\n", argv[0]);
		show_usage(argv[0]);
	}

	inputfile = argv[optind];
	outputfile = argv[optind + 1];
	/* end of argument parsing */

	input_fd = open(inputfile, O_RDONLY);
	if (input_fd < 0) {
		fprintf(stderr, "Can't open %s for reading: %s\n", inputfile,
			strerror(errno));
		goto error;
	}

	output_fd = creat(outputfile, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
	if (output_fd < 0) {
		fprintf(stderr, "Can't create %s: %s\n", outputfile,
			strerror(errno));
		goto error;
	}

	/* BL1 */
	if (bl1ecc) {
		bch = init_bch(gf_order(BCH_POLY), bl1ecc, BCH_POLY);
		if (!bch) {
			fprintf(stderr, "bch init failed\n");
			goto error;
		}

		eccpos = nand_oob_bl1.eccpos;
		eccsteps = pagesize / sizeof(sector);

		for (i = 0, step = 0; i < 32; i++, step++) {
			if (step == 0) {
				memset(eccdata, 0xFF, sizeof(eccdata));
			}
			n = read(input_fd, sector, sizeof(sector));
			if (n < 0) {
				fprintf(stderr, "Error reading: %s\n", strerror(errno));
				goto error;
			}
			if (n < sizeof(sector)) {
				fprintf(stderr, "sector underrun: %d read\n", n);
				memset(sector + n, 0xFF, sizeof(sector) - n);
			}
			memset(ecc, 0, bch->ecc_bytes);
			encode_bch(bch, sector, sizeof(sector), ecc);
			xor_bch(bch, ecc);
			memcpy(eccdata + (bch->ecc_bytes * step), ecc, bch->ecc_bytes);

			n = write(output_fd, sector, sizeof(sector));
			if (n < 0) {
				fprintf(stderr, "Error writing: %s\n", strerror(errno));
				goto error;
			}

			/* special case for 8k pages */
			if (pagesize == 8*SZ_1K && step == 7) {
				/* make 4k "oob" sector */
				step++;
				memset(sector, 0xFF, sizeof(sector));
				ecctotal = bch->ecc_bytes * 8;
				for (j = 0; j < ecctotal; j++) {
					sector[eccpos[j]] = eccdata[j];
				}

				memset(ecc, 0, bch->ecc_bytes);
				encode_bch(bch, sector, sizeof(sector), ecc);
				xor_bch(bch, ecc);
				memcpy(eccdata + (bch->ecc_bytes * step), ecc, bch->ecc_bytes);

				/* write 4k "oob" sector */
				n = write(output_fd, sector, sizeof(sector));
				if (n < 0) {
					fprintf(stderr, "Error writing: %s\n", strerror(errno));
					goto error;
				}

				/* write 8k padding */
				step++;
				memset(sector, 0xFF, sizeof(sector));
				for (; step < eccsteps; step++) {
					n = write(output_fd, sector, sizeof(sector));
					if (n < 0) {
						fprintf(stderr, "Error writing: %s\n", strerror(errno));
						goto error;
					}
				}
				step -= 1;
			}

			if (step == eccsteps - 1) {
				memset(oobdata, 0xFF, oobsize);
				ecctotal = bch->ecc_bytes * eccsteps;
				for (j = 0; j < ecctotal; j++) {
					oobdata[eccpos[j]] = eccdata[j];
				}
				n = write(output_fd, oobdata, oobsize);
				if (n < 0) {
					fprintf(stderr, "Error writing: %s\n", strerror(errno));
					goto error;
				}
				step = -1;
			}
		}

		free_bch(bch);
		bch = NULL;
	}

	/* BL2/data */
	if (eccbits == 1) {
		ecctotal = 4 * eccsteps;
	}
	else if (eccbits > 1) {
		bch = init_bch(gf_order(BCH_POLY), eccbits, BCH_POLY);
		if (!bch) {
			fprintf(stderr, "bch init failed\n");
			goto error;
		}
		ecctotal = bch->ecc_bytes * eccsteps;
	}

	switch (oobsize) {
	case 64:
		eccpos = nand_oob_64.eccpos;
		break;
	case 512:
		eccpos = nand_oob_512.eccpos;
		break;
	case 640:
		eccpos = nand_oob_640.eccpos;
		break;
	}

	while (!done) {
		n = read(input_fd, sector, sizeof(sector));
		if (n < 0) {
			fprintf(stderr, "Error reading: %s\n", strerror(errno));
			goto error;
		}
		if (n == 0) {
			break;
		}

		memset(eccdata, 0xFF, sizeof(eccdata));

		for (step = 0; step < eccsteps; step++) {
			if (!done) {
				if (step > 0) {
					n = read(input_fd, sector, sizeof(sector));
					if (n < 0) {
						fprintf(stderr, "Error reading: %s\n", strerror(errno));
						goto error;
					}
				}

				if (n == 0) {
					memset(sector, 0xFF, sizeof(sector));
					done = 1;
				}
			}

			if (!done) {
				if (n < sizeof(sector)) {
					memset(sector + n, 0xFF, sizeof(sector) - n);
				}

				if (eccbits == 1) {
					encode_1bit(sector, sizeof(sector), ecc);
					memcpy(eccdata + (4 * step), ecc, 4);
				}
				else if (eccbits > 1) {
					memset(ecc, 0, bch->ecc_bytes);
					encode_bch(bch, sector, sizeof(sector), ecc);
					xor_bch(bch, ecc);
					memcpy(eccdata + (bch->ecc_bytes * step), ecc, bch->ecc_bytes);
				}
			}

			n = write(output_fd, sector, sizeof(sector));
			if (n < 0) {
				fprintf(stderr, "Error writing: %s\n", strerror(errno));
				goto error;
			}
		}

		memset(oobdata, 0xFF, oobsize);
		for (j = 0; j < ecctotal; j++) {
			oobdata[eccpos[j]] = eccdata[j];
		}
		n = write(output_fd, oobdata, oobsize);
		if (n < 0) {
			fprintf(stderr, "Error writing: %s\n", strerror(errno));
			goto error;
		}
	}

	if (bch) {
		free_bch(bch);
		bch = NULL;
	}
		
	close(input_fd);
	close(output_fd);

	exit(EXIT_SUCCESS);
	return 0;

error:
	if (bch) {
		free_bch(bch);
		bch = NULL;
	}

	if (input_fd) {
		close(input_fd);
		input_fd = 0;
	}

	if (output_fd) {
		close(output_fd);
		output_fd = 0;
	}

	exit(EXIT_FAILURE);
	return 1;
}

