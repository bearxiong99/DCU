/*
 * Copyright (c) 2015-2016, Atmel Corporation.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU General Public License,
 * version 2, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>

#include "comm.h"
#include "utils.h"

#define X_ACK 0x06
#define X_NAK 0x15
#define X_EOF 0x04
#define X_HEADSIZE 3
#define X_CHKSIZE 2

static bool configure_tty(int fd, int speed)
{
	struct termios tty;

	memset(&tty, 0, sizeof(tty));

	if (tcgetattr(fd, &tty) != 0) {
		perror("error from tcgetattr: ");
		return false;
	}

	if (speed) {
		cfsetospeed(&tty, speed);
		cfsetispeed(&tty, speed);
	}

	tty.c_cflag &= ~(CSIZE | PARENB | PARODD | CSTOPB | CRTSCTS);
	tty.c_cflag |= CS8 | CLOCAL | CREAD;
	tty.c_lflag = 0;
	tty.c_oflag = 0;
	tty.c_cc[VMIN] = 1;
	tty.c_cc[VTIME] = 5;
	tty.c_iflag &= ~(ICRNL | IGNBRK | IXON | IXOFF | IXANY);

	if (tcsetattr(fd, TCSANOW, &tty) != 0) {
		perror("error from tcsetattr: ");
		return false;
	}

	return true;
}

//static bool switch_to_binary(int fd)
//{
//	char cmd[] = "N#";
//	int rsize = 0;
//	fd_set set;
//	struct timeval timeout;
//	int rv;
//
//	FD_ZERO(&set); /* clear the set */
//	FD_SET(fd, &set); /* add our file descriptor to the set */
//
//	timeout.tv_sec = 0;
//	timeout.tv_usec = 500000;
//
//	if (write(fd, cmd, strlen(cmd)) != strlen(cmd))
//		return false;
//
//	//return read(fd, cmd, 2) == 2;
//	rv = select(fd + 1, &set, NULL, NULL, &timeout);
//	if(rv == -1)
//		perror("Select error in switch to binary\n"); /* an error accured */
//	else if(rv == 0)
//		printf("Timeout in switch to binary\n"); /* a timeout occured */
//	else
//		rsize = read(fd, cmd, 2 ); /* there was data to read */
//
//	if (rsize == 2)
//		return true;
//	else
//		return false;
//}

static bool switch_to_binary(int fd)
{
	char cmd[] = "N#";
	ssize_t rd_size;

	int flags = fcntl(fd, F_GETFL, 0);
	fcntl(fd, F_SETFL, flags | O_NONBLOCK);

	rd_size = 0;
	while (rd_size != 2) {

		if (write(fd, cmd, strlen(cmd)) != strlen(cmd))
			return false;
		sleep(1);

		rd_size = read(fd, cmd, 2);
		sleep(1);
	}

	/* restore flags */
	fcntl(fd, F_SETFL, flags);

	return true;

//	if (write(fd, cmd, strlen(cmd)) != strlen(cmd))
//		return false;
//	return read(fd, cmd, 2) == 2;
}

int samba_open(const char* device)
{
	int fd = open(device, O_RDWR | O_NOCTTY | O_SYNC);
	if (fd < 0) {
		perror("Could not open device\n");
		return -1;
	}

	if (!configure_tty(fd, B115200)) {
		close(fd);
		perror("Could not configure TTY\n");
		return -1;
	}

	if (!switch_to_binary(fd)) {
		close(fd);
		perror("Could not switch to binary\n");
		return -1;
	}

	return fd;
}

void samba_close(int fd)
{
	close(fd);
}

bool samba_read_word(int fd, uint32_t addr, uint32_t* value)
{
	char cmd[12];
	int rsize = 0;

	fd_set set;
	struct timeval timeout;
	int rv;

	FD_ZERO(&set); /* clear the set */
	FD_SET(fd, &set); /* add our file descriptor to the set */

	timeout.tv_sec = 0;
	timeout.tv_usec = 500000;

	snprintf(cmd, sizeof(cmd), "w%08x,#", addr);
	if (write(fd, cmd, strlen(cmd)) != strlen(cmd))
		return false;

	//return read(fd, value, 4) == 4;
	rv = select(fd + 1, &set, NULL, NULL, &timeout);
	if(rv == -1)
		perror("Select error\n"); /* an error accured */
	else if(rv == 0)
		printf("Timeout on read word\n"); /* a timeout occured */
	else
		rsize = read(fd, value, 4); /* there was data to read */

	if (rsize == 4)
		return true;
	else
		return true;
}

bool samba_write_word(int fd, uint32_t addr, uint32_t value)
{
	char cmd[20];
	snprintf(cmd, sizeof(cmd), "W%08x,%08x#", addr, value);
	return write(fd, cmd, strlen(cmd)) == strlen(cmd);
}

bool samba_read(int fd, uint8_t* buffer, uint32_t addr, uint32_t size)
{
	char cmd[20];
    uint8_t answer;
	uint8_t xbuf[X_HEADSIZE];

	while (size > 0) {
		uint32_t count = MIN(size, 1024);
		// workaround for bug when size is exactly 512
		if (count == 512)
			count = 1;
		snprintf(cmd, sizeof(cmd), "R%08x,%08x#", addr, count);
		if (write(fd, cmd, strlen(cmd)) != strlen(cmd))
			return false;

		// XMODEM protocol: send 'C' to start transfer
		if (write(fd, "C", 1) != 1)
			return -errno;

		// XMODEM protocol: reads header
		if (read(fd, &xbuf, X_HEADSIZE) != X_HEADSIZE)
			return -errno;

		// XMODEM protocol: receive data block
		if (read(fd, buffer, count) != count)
			return false;

		// XMODEM protocol: reads checksum
		if (read(fd, &xbuf, X_CHKSIZE) != X_CHKSIZE)
			return -errno;

		// XMODEM protocol: if received the whole block, send ACK, otherwise NACK
		if (count == size)
			answer = X_ACK;
		else
			answer = X_NAK;
        if (write(fd, &answer, sizeof(answer)) != sizeof(answer))
        	return -errno;

		// XMODEM protocol: reads EOF
		if (read(fd, &xbuf, 1) != 1)
			return -errno;

		// XMODEM protocol: if EOF, send ACK, otherwise NACK
		if (xbuf[0] == X_EOF)
			answer = X_ACK;
		else
			answer = X_NAK;
        if (write(fd, &answer, sizeof(answer)) != sizeof(answer))
        	return -errno;

		addr += count;
		buffer += count;
		size -= count;
	}
	return true;
}

bool samba_write(int fd, uint8_t* buffer, uint32_t addr, uint32_t size)
{
	char cmd[20];
	while (size > 0) {
		uint32_t count = MIN(size, 1024);
		// workaround for bug when size is exactly 512
		if (count == 512)
			count = 1;
		snprintf(cmd, sizeof(cmd), "S%08x,%08x#", addr, count);
		if (write(fd, cmd, strlen(cmd)) != strlen(cmd))
			return false;
		if (write(fd, buffer, count) != count)
			return false;
		buffer += count;
		size -= count;
	}
	return true;
}
