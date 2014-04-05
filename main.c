#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <pthread.h>
#include <sched.h>
#include "hardware/platform.h"
#include "shell/debug.h"
#include "shell/shell.h"
#include "shell/exit.h"
#include "tcc/libtcc.h"
int main(int argn, char* argv[])
{
//	struct sched_param param;
//	param.sched_priority = 1;
//	sched_setscheduler( 0, SCHED_FIFO, &param);
	init_exit_hanedle();
	if (!fpga_open()) {
		printf("FPGA open error\n");
		exit(1);
	}
	shield_ctrl_init();
	if(argn == 0)
		cli();
	else
	{
		FILE* file;
	}
	fpga_close();
	return 0;
}
