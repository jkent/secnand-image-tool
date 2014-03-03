/*
 * Program to pad bl1 with ECC for 8k page nand on the s5pv210.
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
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <unistd.h>
#include "bch.h"


#define ECC_POLY 0x201B
#define ECC_BITS 16

#define SZ_1K (1024)


struct bch_control *bch;

static const char xor[26] = {
	0x9A, 0xD7, 0xEF, 0x91, 0x88, 0x80, 0xFB, 0xF7,
	0x06, 0x3A, 0x5C, 0x9F, 0x49, 0x24, 0xD0, 0x75,
	0x02, 0xE3, 0x59, 0xE0, 0xE4, 0xBC, 0x1E, 0x20,
	0x70, 0x2E
};


static void show_usage(const char *arg0)
{
	fprintf(stderr, "usage: %s INFILE OUTFILE\n", arg0);
	exit(-1);
}

static int poly_gf(unsigned int poly)
{
  int m = 0;

  while (poly >>= 1)
	m++;

  return m;
}

static void xor_ecc(uint8_t *ecc)
{
	int i;

	for (i = 0; i < bch->ecc_bytes; i++)
		ecc[i] ^= xor[i];
}

int main(int argc, char *argv[])
{
	uint8_t buf[512];
	uint8_t ecc[26];
	int input_fd = 0;
	int output_fd = 0;
	off_t input_len;
	char *in_p, *infile = NULL;
	char *out_p, *outfile = NULL;
	int block, page;

	if (argc != 3)
		show_usage(argv[0]);

	input_fd = open(argv[1], O_RDONLY);
	if (input_fd < 0) {
		fprintf(stderr, "Can't open %s for reading: %s\n", argv[1],
			strerror(errno));
		goto error;
	}

	input_len = lseek(input_fd, 0, SEEK_END);
	if (input_len < 16*SZ_1K) {
		fprintf(stderr, "Not enough data\n");
		goto error;
	}	
	lseek(input_fd, 0, SEEK_SET);

	output_fd = open(argv[2], O_RDWR | O_CREAT, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
	if (output_fd < 0) {
		fprintf(stderr, "Can't open %s for writing: %s\n", argv[2],
			strerror(errno));
		goto error;
	}
	ftruncate(output_fd, 32*SZ_1K);

	infile = mmap(0, 16*SZ_1K, PROT_READ, MAP_SHARED, input_fd, 0);
	if (infile == MAP_FAILED) {
		fprintf(stderr, "Can't mmap %s: %s\n", argv[1], strerror(errno));
		infile = NULL;
		goto error;
	}

	outfile = mmap(0, 32*SZ_1K, PROT_READ|PROT_WRITE, MAP_SHARED, output_fd, 0);
	if (outfile == MAP_FAILED) {
		fprintf(stderr, "Can't mmap %s: %s\n", argv[2], strerror(errno));
		outfile = NULL;
		goto error;
	}

	bch = init_bch(poly_gf(ECC_POLY), ECC_BITS, ECC_POLY);
	if (!bch) {
		fprintf(stderr, "bch init failed\n");
		goto error;
	}

	for (block = 0; block < 4; block++) {
		in_p = infile + (block * 4096);
		out_p = outfile + (block * 8192);
		memcpy(out_p, in_p, 4096);
		memset(out_p + 4096, 0xFF, 4096);
		for (page = 0; page < 8; page++) {
			in_p = infile + (block * 4096) + (page * 512);
			out_p = outfile + (block * 8192) +
				(page * bch->ecc_bytes) + 4108;
			memset(ecc, 0, bch->ecc_bytes);
			encode_bch(bch, in_p, 512, ecc);
			xor_ecc(ecc);
			memcpy(out_p, ecc, bch->ecc_bytes);
		}
	}

	free_bch(bch);
	munmap(infile, 16*SZ_1K);
	munmap(outfile, 32*SZ_1K);
	close(input_fd);
	close(output_fd);

	exit(EXIT_SUCCESS);
	return 0;

error:
	if (infile) {
		munmap(infile, 16*SZ_1K);
		infile = NULL;
	}

	if (outfile) {
 		munmap(outfile, 32*SZ_1K);
		outfile = NULL;
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

