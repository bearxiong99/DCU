#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "dlms_emu_report.h"

static char spuc_conf_name[20];
static char spuc_tmp_name[50];
static char spuc_file_name[50];

void dlms_emu_report_init(void)
{
	sprintf(spuc_conf_name, "top_name");
}

void dlms_emu_report_start_cycle(void)
{
	int csv_fd;
	char puc_ln_buf[100];
	int i_ln_len;

	time_t t = time(NULL);
	struct tm *tm = localtime(&t);

	sprintf(spuc_tmp_name, "/tmp/%s_%u%02u%02u_%02u%02u%02u.csv", spuc_conf_name,
				(tm->tm_year + 1900), tm->tm_mon, tm->tm_mday,tm->tm_hour, tm->tm_min, tm->tm_sec);
	sprintf(spuc_file_name, "/home/ftp/Cycles/%s_%u%02u%02u_%02u%02u%02u.csv", spuc_conf_name,
			(tm->tm_year + 1900), tm->tm_mon, tm->tm_mday,tm->tm_hour, tm->tm_min, tm->tm_sec);
	csv_fd = open(spuc_tmp_name, O_RDWR|O_CREAT|O_APPEND, S_IROTH|S_IWOTH|S_IXOTH);

	if (csv_fd != -1) {
		i_ln_len = sprintf(puc_ln_buf, "starting cycle %s at %s\r\n", spuc_conf_name, asctime(tm));
		write(csv_fd, puc_ln_buf, i_ln_len);
		i_ln_len = sprintf(puc_ln_buf, "time;dev_sn;operation;element_id;result\r\n");
		write(csv_fd, puc_ln_buf, i_ln_len);
		close(csv_fd);
	}
}


void dlms_emu_report_node_cycle(dlms_report_info_t *x_node_info)
{
	int csv_fd;
	char puc_ln_buf[100];
	int i_ln_len;

	time_t t = time(NULL);
	struct tm *tm = localtime(&t);

	csv_fd = open(spuc_tmp_name, O_RDWR|O_CREAT|O_APPEND, S_IROTH|S_IWOTH|S_IXOTH);

	if (csv_fd != -1) {
		i_ln_len = sprintf(puc_ln_buf, "%u/%02u/%02u_%02u:%02u:%02u;%s;%s;%u;%s\r\n",
				(tm->tm_year + 1900), tm->tm_mon, tm->tm_mday,tm->tm_hour, tm->tm_min, tm->tm_sec,
				x_node_info->pc_dev_sn, x_node_info->pc_operation, x_node_info->i_element_id, x_node_info->pc_result);
		write(csv_fd, puc_ln_buf, i_ln_len);
		close(csv_fd);
	}
}


void dlms_emu_report_end_cycle(void)
{
	int csv_fd, csv_fd2;
	char puc_ln_buf[100];
	int i_ln_len;
	char c;

	time_t t = time(NULL);
	struct tm *tm = localtime(&t);

	csv_fd = open(spuc_tmp_name, O_RDWR|O_CREAT|O_APPEND, S_IROTH|S_IWOTH|S_IXOTH);

	if (csv_fd != -1) {
		i_ln_len = sprintf(puc_ln_buf, "ending cycle %s at %s\r\n", spuc_conf_name, asctime(tm));
		write(csv_fd, puc_ln_buf, i_ln_len);
		close(csv_fd);

		/* Copy temporal file to FTP folder */
		csv_fd = open(spuc_tmp_name, O_RDONLY);
		csv_fd2 = open(spuc_file_name, O_RDWR|O_CREAT|O_APPEND, S_IROTH|S_IWOTH|S_IXOTH);
		while(read(csv_fd, &c, sizeof(c) != 0)){
			write(csv_fd2, &c ,sizeof(c));
		}
		close(csv_fd);
		close(csv_fd2);

		remove(spuc_tmp_name);
		chmod(spuc_file_name, 0777);
	}
}


