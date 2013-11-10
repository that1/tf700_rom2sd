// extract-ramdisk.c - ramdisk extractor for Android boot image inside NVidia blob
// Copyright (c) _that@xda
// License: GPLv3

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

#include "bootimg.h"
#include "blob.h"


int extract_ramdisk_from_bootimage(int fd)
{
	// fd must be positioned at start of boot image
	boot_img_hdr hdr;

	read(fd, &hdr, sizeof(hdr));
	if (memcmp(hdr.magic, BOOT_MAGIC, BOOT_MAGIC_SIZE) != 0)
	{
		fprintf(stderr, "Error: Input file is not a valid boot image or signed blob.\n");
		return 1;
	}
	
	lseek(fd, hdr.page_size - sizeof(hdr), SEEK_CUR);

	// skip zImage
	int ksize = (hdr.kernel_size + hdr.page_size - 1) / hdr.page_size * hdr.page_size;
	lseek(fd, ksize, SEEK_CUR);
	
	if (hdr.ramdisk_size > 8000000)
	{
		fprintf(stderr, "Error: ramdisk size fails sanity check, size=%d\n", hdr.ramdisk_size);
		return 1;
	}
	char* ramdisk = malloc(hdr.ramdisk_size);
	if (ramdisk == 0)
	{
		fprintf(stderr, "Error: malloc for ramdisk failed.\n");
		return 1;
	}
	read(fd, ramdisk, hdr.ramdisk_size);
	write(STDOUT_FILENO, ramdisk, hdr.ramdisk_size);
	
	return 0;
}


int extract_ramdisk_from_blob(header_type* hdr, int fd, char* partname)
{
#define MAX_PARTS 8
	// fd must be positioned after blob header, at partition list
	if (hdr->num_parts > MAX_PARTS)
	{
		fprintf(stderr, "Error: number of partitions in blob fails sanity check, num=%d\n", hdr->num_parts);
		return 1;
	}
	part_type parts[MAX_PARTS];
	read(fd, parts, sizeof(part_type) * hdr->num_parts);
	
	int i;
	for(i = 0; i < hdr->num_parts; ++i)
	{
		if (strcmp(parts[i].name, partname) == 0)
		{
			lseek(fd, SECURE_OFFSET + parts[i].offset, SEEK_SET);
			return extract_ramdisk_from_bootimage(fd);
		}
	}
	
	fprintf(stderr, "Error: blob contains no %s partition.\n", partname);
	return 1;
}

int extract_ramdisk_autodetect(int fd)
{
	secure_header_type blob_hdr;
	
	read(fd, &blob_hdr, sizeof(blob_hdr));
	if (memcmp(blob_hdr.magic, SECURE_MAGIC, SECURE_MAGIC_SIZE) == 0)
	{
		if (memcmp(blob_hdr.real_header.magic, MAGIC, MAGIC_SIZE) != 0)
		{
			fprintf(stderr, "Error: MSM-RADIO-UPDATE magic not found.\n");
			return 1;
		}
		// yes, this looks like a blob
		return extract_ramdisk_from_blob(&blob_hdr.real_header, fd, "LNX");
	}
	else
	{
		// not a blob - back to start, maybe a raw boot image
		lseek(fd, 0, SEEK_SET);
	}
	
	// now we should be at "ANDROID!"
	return extract_ramdisk_from_bootimage(fd);
}


int main(int argc, char* argv[])
{
	if (argc < 2)
	{
		fprintf(stderr, "usage: extract-ramdisk.c inputfile\n"
			"\n"
			"inputfile may be a signed blob (-SIGNED-BY-SIGNBLOB-) or an Android boot image (ANDROID!).\n"
			"output is the compressed ramdisk on stdout.\n"
			);
		return 1;
	}
	
	char* filename = argv[1];
	
	int fd = open(filename, O_RDONLY);
	if (fd == -1) {
		fprintf(stderr, "cannot read input file %s, error=%d: %s\n", filename, errno, strerror(errno));
		return 1;
	}
	
	int rc = extract_ramdisk_autodetect(fd);
	
	close(fd);

	return rc;
}
