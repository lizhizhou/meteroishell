/*
 * shell.c
 *
 *  Created on: May 10, 2013
 *      Author: lizhizhou
 */
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <pthread.h>
#include "../hardware/platform.h"
#include "../hardware/PIO26.h"
#include "../hardware/shield_ctrl.h"
#include "../hardware/FPGA.h"
#include "../hardware/AM2301.h"
#include "../hardware/sht1x.h"
#include "../hardware/position_sensor.h"
#include "../hardware/step_motor.h"
#include "../hardware/brush_motor.h"
#include "../hardware/fan_motor.h"
#include "../hardware/unit_test.h"
#include "../tcc/libtcc.h"
#include "debug.h"

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

typedef int (*shell_cmd_func)(int argc, char *argv[]);
typedef struct shell_cmd_t {
    char *name;
    char *help;
    shell_cmd_func func;
} shell_cmd_func_t;
extern shell_cmd_func_t shell_cmd_func_list[];

int unit_test(int argc,char* argv[])
{
	// Unit_test
	int i;
	printf("1:AM2301_test\n");
	printf("2:syringe_test\n");
	printf("3:step_motmor_test\n");
	printf("4:brush_motor_test\n");
	printf("5:microscope_test\n");
	printf("6:wheel_plate_test\n");
	printf("7:PID_test\n");
	printf("8:PIO_test\n");
	printf("Input choose\n");
	scanf("%d",&i);
	switch(i) {
		case 1:
			AM2301_test();
			break;
		case 2:
			break;
		case 3:
			step_motmor_test();
			break;
		case 4:
			brush_motor_test();
			break;
		case 5:
			break;
		case 6:
			break;
		case 7:
			PID_test();
			break;
		case 8:
			PIO_test();
			break;
	}
	return (true);
}

int cli_tcc(int argc, char **argv)
{
    TCCState *s;
    int (*func)(int argc_f, char* argv_f[]);
    char filename[128];

	unsigned long led0 = LED_0;
	unsigned long led1 = LED_1;
	unsigned long led2 = LED_2;
	unsigned long led3 = LED_3;

    s = tcc_new();
    if (!s) {
    	printf("Could not create tcc state\n");
        return 0;
    }

    /* MUST BE CALLED before any compilation */
    tcc_set_output_type(s, TCC_OUTPUT_MEMORY);

    /* as a test, we add a symbol that the compiled program can use.
       You may also open a dll with tcc_add_dll() and use symbols from that */
	ADD_FUNCTION(s, set_rgb_led);
	ADD_FUNCTION(s, printf);
	ADD_VAR(s,  led0);
	ADD_VAR(s,  led1);
	ADD_VAR(s,  led2);
	ADD_VAR(s,  led3);

    sscanf(argv[0],"%s", filename);
    printf("open file %s", filename);
    if (argc > 0)
    {
        if(tcc_add_file(s, filename) == -1)
        {
            printf("compile error\n");
            return (false);
        }
    }
    else
    {
        printf("Usage: tcc file\n");
        return (false);
    }
    printf("compile ok \n");
    /* relocate the code */
    if (tcc_relocate(s, TCC_RELOCATE_AUTO) < 0)
        return (false);

    /* get entry symbol */
    func = tcc_get_symbol(s, "main");
    if (!func) {
        return (false);
    }
    printf("function %p \n", func);
    /* run the code */
    func(argc--, argv++);

    /* delete the state */
    tcc_delete(s);

    return (true);
}


static int cli_help (int argc, char *argv[])
{
	shell_cmd_func_t *func = shell_cmd_func_list;
    printf("Help Manual!\n");

    while (func->name) {
        printf("%s: %s\n", func->name, func->help);
        func++;
    };
    return (true);
}

int cli_debug(int argc,char* argv[])
{
    char buf[255];
    if(argc < 1) {
        printf("Error command");
        return (false);
    }
    sscanf(argv[0], "%s", buf);
    if(!strcmp(buf,"on"))
    	debug_flag = 1;
    else if(!strcmp(buf,"off"))
    	debug_flag = 0;
    return (true);
}

shell_cmd_func_t shell_cmd_func_list[] = {
	{"tcc",       "Tiny C compile",                    cli_tcc},
	{"debug",     "on/off the debug log",              cli_debug},
	{"help",      "Print Help Manual",                 cli_help},
    {"ut",        "Unit test of the system",           unit_test},
    {NULL, NULL, NULL}
};

#define CLI_BUFFER_SIZE 1024
#define ARG_LIST_SIZE 20
static int cmd_distribution (int argc, char *argv[])
{
	shell_cmd_func_t *func = shell_cmd_func_list;
    char keyword[CLI_BUFFER_SIZE];
    char *arg = argv[0];
    int  state;
    //find first argument
    sscanf(arg,"%1024s",keyword);

    while (func->name) {
        if (!strncasecmp(func->name, keyword, CLI_BUFFER_SIZE)) {
            if (func->func) {
                state = ((func->func)(--argc, ++argv));
            }
            break;
        }
        func++;
    };

    if (!func->name) {
        printf("%s: Unsupported command!\n", arg);
    }

    if (state == 0) {
        printf("%s: argument error!\n", arg);
    }
    return (true);
}

static int parser_cli(char* buffer, char* argv[])
{
    int i = 0;
    if (strlen(buffer) == 0)
        return (0);
    argv[i] = buffer;
    i++;
    while(*buffer!='\0')
    {
        buffer++;
        if(isspace(*buffer)) {
            while(*buffer!='\0') {
                buffer++;
                if(!isspace(*buffer) && *buffer !='\0'){
                    argv[i] = buffer;
                    i++;
                    break;
                }
            }
        }
    }
    return (i);
}

int gets_s(char* buffer, int buf_size)
{
    int i;
    for (i = 0; i < buf_size; i++) {
        *buffer = getchar();
        if (*buffer == '\n')
            break;
        buffer++;
    }
    *buffer = '\0';
    return (i);
}

int cli(void) {
    char buffer[CLI_BUFFER_SIZE];
    char *argv[ARG_LIST_SIZE];

    while(1)
    {
        int argc, i=0;
        bzero(buffer, sizeof(buffer));
        printf("Meteroi shell>");
        gets_s(buffer, sizeof(buffer));
        argc = parser_cli(buffer,argv);
        if (argc == 0)
            continue;
        debuginf("argc=%d\n",argc);
        while(i < argc ) {
            char cmd_name[256];
            sscanf(argv[i],"%256s ", cmd_name);
            debuginf("arg[%d] = %s\n", i, cmd_name);
            i++;
        }
        if(strcmp(buffer,"exit")==0)
        {
            break;
        } else {
        	//pthread_mutex_lock(&mutex);
            cmd_distribution(argc, argv);
            //pthread_mutex_unlock(&mutex);
        }
    }
    return (true);
}

