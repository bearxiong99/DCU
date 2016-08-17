#include "app_debug.h"
#include <stdio.h>
#include <stdarg.h>
#include <fcntl.h>
#include <time.h>
#include <unistd.h>

#ifdef DEBUG_ENABLE
static char spuc_dbg_name[50];
static char spuc_dbg_buf[500];
static int si_log_fd;

void PRINTF_INIT(void) {
	int i_ln_len;
	time_t t = time(NULL);
	struct tm *tm = localtime(&t);

	i_ln_len = sprintf(spuc_dbg_name, "/tmp/PLC_Dbg_%u%02u%02u_%02u%02u%02u.log",
			(tm->tm_year + 1900), tm->tm_mon, tm->tm_mday,tm->tm_hour, tm->tm_min, tm->tm_sec);
	si_log_fd = open(spuc_dbg_name, O_RDWR|O_CREAT|O_APPEND, S_IROTH|S_IWOTH|S_IXOTH);
	if (i_ln_len > 0) {
		i_ln_len = sprintf(spuc_dbg_buf, "Start DCU debugging...\r\n");
		write(si_log_fd, spuc_dbg_buf, i_ln_len);
	}

}

void PRINTF(const char * format, ...) {
	int i_ln_len;

	va_list argptr;
	va_start(argptr,format);
	i_ln_len = vsprintf(spuc_dbg_buf, format, argptr);
	va_end(argptr);

	if (i_ln_len > 0) {
		write(si_log_fd, spuc_dbg_buf, i_ln_len);
	}
}

#else
void PRINTF_INIT(void) {

}

void PRINTF(const char * format, ...) {

}

#endif
