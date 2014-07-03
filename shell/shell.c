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
#include "../api/lophilo.h"
#include "debug.h"

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

typedef int (*shell_cmd_func)(int argc, char *argv[]);
typedef struct shell_cmd_t {
    char *name;
    char *help;
    shell_cmd_func func;
} shell_cmd_func_t;
extern shell_cmd_func_t shell_cmd_func_list[];

int set_led(int argc, char **argv)
{
	int id,r,g,b;
	sscanf(argv[0], "%d",&id);
	sscanf(argv[1], "%d",&r);
	sscanf(argv[2], "%d",&g);
	sscanf(argv[3], "%d",&b);
	debuginf("id=%d, r=%d g=%d b=%d\n", id, r, g, b);
	led(id, (char)r,(char)g,(char)b);
	return (true);
}

int cli_tcc(int argc, char **argv)
{
    TCCState *s;
    int (*func)(int argc_f, char* argv_f[]);
    char filename[128];
    int ret;

    s = tcc_new();
    if (!s) {
    	printf("Could not create tcc state\n");
        return (false);
    }

    /* MUST BE CALLED before any compilation */
    tcc_set_output_type(s, TCC_OUTPUT_MEMORY);

    /* as a test, we add a symbol that the compiled program can use.
       You may also open a dll with tcc_add_dll() and use symbols from that */
	ADD_FUNCTION(s, led);
	ADD_FUNCTION(s, printf);
	ADD_FUNCTION(s, sleep);
	ADD_FUNCTION(s, usleep);

    sscanf(argv[0],"%s", filename);
    debuginf("open file %s\n", filename);
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
    /* relocate the code */
    if (tcc_relocate(s, TCC_RELOCATE_AUTO) < 0)
        return (false);

    usleep(1000*100); // wait for ready

    /* get entry symbol */
    func = tcc_get_symbol(s, "main");
    if (!func) {
        return (false);
    }
    /* run the code */
    ret = func(argc--, argv++);
    debuginf("ret=%d\n", ret);

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
	{"led",       "Set led",                           set_led},
	{"tcc",       "Tiny C compile",                    cli_tcc},
	{"debug",     "on/off the debug log",              cli_debug},
	{"help",      "Print Help Manual",                 cli_help},
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
        fflush(stdout);
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

