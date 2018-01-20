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

#include "gpio.h"
#include "tools.h"

static char syscmd_plc_down[] =  "/etc/init.d/PLCpppUp stop";
static char syscmd_plc_up[] =  "/etc/init.d/PLCpppUp start";
static char spuc_plc_iface_file[] = "/sys/class/net/ppp0/statistics/rx_packets";

static char syscmd_gprs_down[] =  "/etc/init.d/GPRSpppUp stop";
static char syscmd_gprs_up[] =  "/etc/init.d/GPRSpppUp start";
static char spuc_gprs_iface_file[] = "/sys/class/net/ppp1/statistics/rx_packets";

static const char spuc_fu_start_cmd[] =  "/home/cfg/fu_en";

static void _plc_init_pins(void)
{
	/* Configure PLC ERASE */
	GPIOExport(PLC_ERASE_GPIO_ID);
	GPIODirection(PLC_ERASE_GPIO_ID, GPIO_OUT);
	GPIOWrite(PLC_ERASE_GPIO_ID, PLC_ERASE_GPIO_DISABLE);
	usleep(500);

	/* Configure PLC RESET */
	GPIOExport(PLC_RESET_GPIO_ID);
	GPIODirection(PLC_RESET_GPIO_ID, GPIO_OUT);
	GPIOWrite(PLC_RESET_GPIO_ID, PLC_RESET_GPIO_DISABLE);
	usleep(500);
}

static void _gprs_init_pins(void)
{
	/* Configure GPRS SUPPLY */
	GPIOExport(GPRS_SUPPLY_GPIO_ID);
	GPIODirection(GPRS_SUPPLY_GPIO_ID, GPIO_OUT);
	GPIOWrite(GPRS_SUPPLY_GPIO_ID, GPRS_SUPPLY_GPIO_DISABLE);
	usleep(500);

	/* Configure GPRS DETECT */
	GPIOExport(GPRS_DETECT_GPIO_ID);
	GPIODirection(GPRS_DETECT_GPIO_ID, GPIO_IN);
	GPIOWrite(GPRS_DETECT_GPIO_ID, GPRS_DETECT_GPIO_DISABLE);
	usleep(500);

	/* Configure GPRS PWRKEY */
	GPIOExport(GPRS_PWRKEY_GPIO_ID);
	GPIODirection(GPRS_PWRKEY_GPIO_ID, GPIO_OUT);
	GPIOWrite(GPRS_PWRKEY_GPIO_ID, GPRS_PWRKEY_GPIO_DISABLE);
	usleep(500);
}

void tools_init(void)
{
	/* Configure PLC pinout */
	_plc_init_pins();

	/* Configure GPRS pinout */
	_gprs_init_pins();
}

void tools_plc_reset(void)
{
	GPIOWrite(PLC_RESET_GPIO_ID, PLC_RESET_GPIO_ENABLE);
	usleep(500);

	GPIOWrite(PLC_RESET_GPIO_ID, PLC_RESET_GPIO_DISABLE);
	usleep(5000);
}

void tools_plc_erase(void)
{
	/* Enable ERASE pin */
	GPIOWrite(PLC_ERASE_GPIO_ID, PLC_ERASE_GPIO_ENABLE);
	usleep(500);

	/* Enable RESET pin */
	GPIOWrite(PLC_RESET_GPIO_ID, PLC_RESET_GPIO_ENABLE);
	usleep(500);

	/* Disable RESET pin */
	GPIOWrite(PLC_RESET_GPIO_ID, PLC_RESET_GPIO_DISABLE);
	sleep(1);

	/* Disable ERASE pin */
	GPIOWrite(PLC_ERASE_GPIO_ID, PLC_ERASE_GPIO_DISABLE);
	usleep(500);

	/* Enable RESET pin */
	GPIOWrite(PLC_RESET_GPIO_ID, PLC_RESET_GPIO_ENABLE);
	usleep(500);

	/* Disable RESET pin */
	GPIOWrite(PLC_RESET_GPIO_ID, PLC_RESET_GPIO_DISABLE);
	sleep(1);

}

void tools_plc_down(void)
{
	int res;

	res = system(syscmd_plc_down);
	printf("plc iface down res: %u\r\n", res);
	sleep(2);
}

void tools_plc_up(void)
{
	int res;

	res = system(syscmd_plc_up);
	printf("plc iface up res: %u\r\n", res);
	sleep(5);
}

int tools_plc_check(void)
{
	struct stat dataFile;

	/* Check PPP0 interface stats file*/
	if (lstat (spuc_plc_iface_file, &dataFile) == -1) {
			return -1;
	}

	return 0;
}

void tools_gprs_enable(void)
{
	/* Provide supply */
	GPIOWrite(GPRS_SUPPLY_GPIO_ID, GPRS_SUPPLY_GPIO_ENABLE);
	sleep(1);

	/* Reset */
	GPIOWrite(GPRS_PWRKEY_GPIO_ID, GPRS_DETECT_GPIO_ENABLE);
	sleep(1);
	GPIOWrite(GPRS_PWRKEY_GPIO_ID, GPRS_DETECT_GPIO_DISABLE);
}

void tools_gprs_disable(void)
{
	/* Disable supply */
	GPIOWrite(GPRS_SUPPLY_GPIO_ID, GPRS_SUPPLY_GPIO_DISABLE);
	sleep(1);
}

void tools_gprs_reset(void)
{
	GPIOWrite(GPRS_PWRKEY_GPIO_ID, GPRS_DETECT_GPIO_ENABLE);
	sleep(1);
	GPIOWrite(GPRS_PWRKEY_GPIO_ID, GPRS_DETECT_GPIO_DISABLE);
}

bool tools_gprs_detect(void)
{
	/* Check GPRS detect pin */
	if (GPIORead(GPRS_DETECT_GPIO_ID) == GPRS_DETECT_GPIO_ENABLE) {
			return true;
	}

	return false;
}

void tools_gprs_down(void)
{
	int res;

	res = system(syscmd_gprs_down);
	printf("gprs iface down res: %u\r\n", res);
	sleep(2);
}

void tools_gprs_up(void)
{
	int res;

	res = system(syscmd_gprs_up);
	printf("gprs iface up res: %u\r\n", res);
	sleep(5);
}

int tools_gprs_check(void)
{
	struct stat dataFile;

	/* Check PPP1 interface stats file*/
	if (lstat (spuc_gprs_iface_file, &dataFile) == -1) {
			return -1;
	}

	return 0;
}

uint16_t tools_extract_u16(void *vptr_value) {
	uint16_t us_val_swap;
	uint8_t uc_val_tmp;

	uc_val_tmp = *(uint8_t *)vptr_value;
	us_val_swap = (uint16_t)uc_val_tmp;

	uc_val_tmp = *((uint8_t *)vptr_value + 1);
	us_val_swap += ((uint16_t)uc_val_tmp) << 8;

	return us_val_swap;
}

int tools_get_timestamp_ms(void)
{
    struct timeval te;
    int ms;

    gettimeofday(&te, NULL);
    ms = te.tv_sec*1000 + te.tv_usec/1000;

    return ms;
}

int tools_fu_start_check(char *pc_fu_filename)
{
	struct stat dataFile;
	int i_size_fd;
	int fd;

	/* Check if FU start cmd file exists */
	if (lstat (spuc_fu_start_cmd, &dataFile) == -1) {
			return 0;
	}

	/* Extract the name of file to use in firmware upgrade process */
	fd = open(spuc_fu_start_cmd, O_RDWR, S_IROTH);
	i_size_fd = read(fd, pc_fu_filename, 200);
	close(fd);
	remove(pc_fu_filename);

	return i_size_fd;
}
