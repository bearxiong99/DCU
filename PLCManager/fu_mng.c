#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <sys/stat.h>

#include "fu_mng.h"
#include "tools.h"

#include "chipid.h"
#include "comm.h"
#include "utils.h"
#include "eefc.h"

#ifdef FU_MNF_DEBUG_CONSOLE
#	define FU_INFO_DEBUG(a)   printf a
#else
#	define FU_INFO_DEBUG(a)   (void)0
#endif

static int si_fu_mng_id;

#define BUFFER_WRITE         8192
#define BUFFER_READ          128

static bool get_file_size(const char* filename, uint32_t* size)
{
	struct stat st;

	if (stat(filename, &st) != 0) {
		fprintf(stderr, "Could not access '%s'\n", filename);
		return false;
	}

	*size = st.st_size;
	return true;
}

static bool write_flash(int fd, const struct _chip* chip, const char* filename, uint32_t addr, uint32_t size)
{
	FILE* file = fopen(filename, "rb");
	if (!file) {
		fprintf(stderr, "Could not open '%s' for reading\n", filename);
		return false;
	}

	uint8_t buffer[BUFFER_WRITE];
	uint32_t total = 0;
	while (total < size) {
		uint32_t count = MIN(BUFFER_WRITE, size - total);
		if (fread(buffer, 1, count, file) != count) {
			fprintf(stderr, "Error while reading from '%s'", filename);
			fclose(file);
			return false;
		}

		if (!eefc_write(fd, chip, buffer, addr, count)) {
			fclose(file);
			return false;
		}

		total += count;
		addr += count;
		fprintf(stderr, "Wrote '%d' bytes\n", total);
	}

	fclose(file);
	return true;
}

static bool verify_flash(int fd, const struct _chip* chip, const char* filename, uint32_t addr, uint32_t size)
{
	int i;

	FILE* file = fopen(filename, "rb");
	if (!file) {
		fprintf(stderr, "Could not open '%s' for reading\n", filename);
		return false;
	}

	uint8_t buffer1[BUFFER_READ];
	uint8_t buffer2[BUFFER_READ];
	uint32_t total = 0;
	while (total < size) {
		uint32_t count = MIN(BUFFER_READ, size - total);
		if (fread(buffer1, 1, count, file) != count) {
			fprintf(stderr, "Error while reading from '%s'\n", filename);
//			fclose(file);
//			return false;
		}

		if (!eefc_read(fd, chip, buffer2, addr, count)) {
			fprintf(stderr, "Error while reading from address 0x%08x\n", chip->flash_addr + addr);
//			fclose(file);
//			return false;
		}

		for (i = 0; i < BUFFER_READ; i++) {
			if (buffer1[i] != buffer2[i]) {
				fprintf(stderr, "Verify failed, first difference at offset %d\n", chip->flash_addr + addr + i);
//				fclose(file);
//				return false;
			}
		}

		total += count;
		addr += count;
	}

	fclose(file);
	printf("Verify passed\n");
	return true;
}

void fu_mng_init(int _app_id)
{
	si_fu_mng_id = _app_id;
}


int fu_mng_start(char *pc_fu_filename)
{
	int fd = -1;
	char* port = "/dev/ttyS2";
	uint32_t filesize = 0;

	/* PPP interface down */
	printf("PPP interface down\n");
	tools_plc_down();

	/* Erase device */
	printf("Erase device\n");
	tools_plc_erase();

	/* Open Port */
	printf("Port: %s\n", port);
	fd = samba_open(port);
	if (fd < 0) {
		fprintf(stderr, "Error opening port\n");
		return -1;
	}

	/* Test connectivity: Read flash size */
	const struct _chip* chip;
	if (!chipid_identity_serie(fd, &chip)) {
		fprintf(stderr, "Could not identify chip\n");
		return -1;
	}
	printf("Device: Atmel %s\n", chip->name);

	/* Get FU file (read mode) */
	if (!get_file_size(pc_fu_filename, &filesize)) {
		fprintf(stderr, "FU file not found\n");
		return -1;
	}

	/* Unlock Flash mem */
	struct _eefc_locks locks;
	if (!eefc_read_flash_info(fd, chip, &locks)) {
		fprintf(stderr, "Could not read flash information\n");
		return -1;
	}
	printf("Flash Size: %uKB\n", chip->flash_size);

	printf("Unlocking %d bytes at 0x%08x\n", filesize, chip->flash_addr);
	if (!eefc_unlock(fd, chip, &locks, 0, filesize)) {
		fprintf(stderr, "Could not unblock flash memory\n");
		return -1;
	}

	/* Write FU file in flash */
	printf("Writing %d bytes at 0x%08x from file '%s'\n", filesize, chip->flash_addr, pc_fu_filename);
	if (!write_flash(fd, chip, pc_fu_filename, 0, filesize)) {
		fprintf(stderr, "Could not write file in flash memory\n");
		return -1;
	}

	/* Verify FU file from flash */
//	printf("Verifying %d bytes at 0x%08x with file '%s'\n", filesize, chip->flash_addr, spuc_filename);
//	if (!verify_flash(fd, chip, spuc_filename, 0, filesize)) {
//		fprintf(stderr, "Error in verification process\n");
//		return -1;
//	}

	/* Set GPVN configuration */
	printf("Setting GPNVM%d\n", 1);
	if (!eefc_set_gpnvm(fd, chip, 1)) {
		fprintf(stderr, "Error in GPNVM bit setting\n");
		return -1;
	}

	/* Reset device */
	printf("Reset device\n");
	tools_plc_reset();

	/* Success */
	printf("Firmware Upgrade process success\n");
	return 0;

}

void fu_mng_process(void)
{



}
