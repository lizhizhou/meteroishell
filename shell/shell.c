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
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include <unistd.h>
#include "debug.h"
#include "linenoise.h"
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

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

typedef int (*shell_cmd_func)(int argc, char *argv[]);
typedef struct shell_cmd_t {
    char *name;
    char *help;
    shell_cmd_func func;
} shell_cmd_func_t;
extern shell_cmd_func_t shell_cmd_func_list[];

int cli_led(int argc, char **argv)
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

int cli_fpga(int argc, char **argv)
{
	char file_path[255];
	sscanf(argv[0], "%s", file_path);
	fpga_download(file_path);
	return (true);
}

int cli_sh(int argc, char **argv)
{
    pid_t pid;
    int status;
    char* cmdstring = argv[0];

    if((pid = fork())<0){
        status = -1;
    }
    else if(pid == 0){
        execl("/bin/sh", "sh", "-c", cmdstring, (char *)0);
        _exit(127); //子进程正常执行则不会执行此语句
    }
    else
      {
       while(waitpid(pid, &status, 0) < 0){
          if(errno != EINTR)
            {
             status = -1;
             break;
            }
          }
      }
    return status;
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
	{"led",       "Set led",                           cli_led},
	{"fpga",      "FGPA downlaoder",                   cli_fpga},
	{"tcc",       "Tiny C compile",                    cli_tcc},
	{"sh",        "Run linux shell command",           cli_sh},
	{"debug",     "on/off the debug log",              cli_debug},
	{"help",      "Print Help Manual",                 cli_help},
    {NULL, NULL, NULL}
};

void completion(const char *buf, linenoiseCompletions *lc) {
	shell_cmd_func_t *func = shell_cmd_func_list;
    while (func->name) {
		if (buf[0] == func->name[0]) {
			linenoiseAddCompletion(lc, func->name);
            break;
		}
        func++;
    }
}

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

int cli(void) {
    char *line;
    char *argv[ARG_LIST_SIZE];
    int argc, i=0;

    /* Set the completion callback. This will be called every time the
     * user uses the <tab> key. */
    linenoiseSetCompletionCallback(completion);

    /* Load history from file. The history file is just a plain text file
     * where entries are separated by newlines. */
    linenoiseHistoryLoad("history.txt"); /* Load the history at startup */

    /* Now this is the main loop of the typical linenoise-based application.
     * The call to linenoise() will block as long as the user types something
     * and presses enter.
     *
     * The typed string is returned as a malloc() allocated string by
     * linenoise, so the user needs to free() it. */
    while((line = linenoise("Meteroi shell>")) != NULL) {
        /* Do something with the string. */
        if (line[0] != '\0' && line[0] != '/') {
        	debuginf("echo: '%s'\n", line);
            argc = parser_cli(line,argv);
            if (argc == 0)
                continue;
            debuginf("argc=%d\n",argc);
            while(i < argc ) {
                char cmd_name[256];
                sscanf(argv[i],"%256s ", cmd_name);
                debuginf("arg[%d] = %s\n", i, cmd_name);
                i++;
            }
            if(strcmp(line,"exit")==0)
            {
                break;
            } else {
            	//pthread_mutex_lock(&mutex);
                cmd_distribution(argc, argv);
                //pthread_mutex_unlock(&mutex);
            }
            linenoiseHistoryAdd(line); /* Add to the history. */
            linenoiseHistorySave("history.txt"); /* Save the history on disk. */
        } else if (!strncmp(line,"/historylen",11)) {
            /* The "/historylen" command will change the history len. */
            int len = atoi(line+11);
            linenoiseHistorySetMaxLen(len);
        } else if (line[0] == '/') {
            printf("Unreconized command: %s\n", line);
        }
        free(line);
    }
    return (true);
}

