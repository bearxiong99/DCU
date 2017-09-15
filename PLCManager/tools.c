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

static char syscmd_ppp_down[] =  "/etc/init.d/PLCpppUp stop";
static char syscmd_ppp_up[] =  "/etc/init.d/PLCpppUp start";
static char spuc_ppp_iface_file[] = "/sys/class/net/ppp0/statistics/rx_packets";

void tools_init(void)
{
	/* Configure ERASE pinout */
	GPIOExport(ERASE_GPIO_ID);
	GPIODirection(ERASE_GPIO_ID, GPIO_OUT);
	GPIOWrite(ERASE_GPIO_ID, ERASE_GPIO_DISABLE);
	usleep(500);
	/* Configure RESET */
	GPIOExport(RESET_GPIO_ID);
	GPIODirection(RESET_GPIO_ID, GPIO_OUT);

	GPIOWrite(RESET_GPIO_ID, RESET_GPIO_DISABLE);
	usleep(500);
}

void tools_plc_reset(void)
{
	GPIOWrite(RESET_GPIO_ID, RESET_GPIO_ENABLE);
	usleep(500);

	GPIOWrite(RESET_GPIO_ID, RESET_GPIO_DISABLE);
	usleep(5000);
}

void tools_plc_erase(void)
{
	/* Enable ERASE pin */
	GPIOWrite(ERASE_GPIO_ID, ERASE_GPIO_ENABLE);
	usleep(500);

	/* Enable RESET pin */
	GPIOWrite(RESET_GPIO_ID, RESET_GPIO_ENABLE);
	usleep(500);

	/* Disable RESET pin */
	GPIOWrite(RESET_GPIO_ID, RESET_GPIO_DISABLE);
	sleep(1);

	/* Disable ERASE pin */
	GPIOWrite(ERASE_GPIO_ID, ERASE_GPIO_DISABLE);
	usleep(500);

	/* Enable RESET pin */
	GPIOWrite(RESET_GPIO_ID, RESET_GPIO_ENABLE);
	usleep(500);

	/* Disable RESET pin */
	GPIOWrite(RESET_GPIO_ID, RESET_GPIO_DISABLE);
	sleep(1);

}

void tools_ppp0_down(void)
{
	system(syscmd_ppp_down);
	sleep(2);
}

void tools_ppp0_up(void)
{
	system(syscmd_ppp_up);
	sleep(5);
}

int tools_ppp0_check(void)
{
	struct stat dataFile;

	/* Check PPP0 interface stats file*/
	if (lstat (spuc_ppp_iface_file, &dataFile) == -1) {
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
