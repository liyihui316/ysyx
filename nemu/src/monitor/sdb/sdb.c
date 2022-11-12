/***************************************************************************************
* Copyright (c) 2014-2022 Zihao Yu, Nanjing University
*
* NEMU is licensed under Mulan PSL v2.
* You can use this software according to the terms and conditions of the Mulan PSL v2.
* You may obtain a copy of Mulan PSL v2 at:
*          http://license.coscl.org.cn/MulanPSL2
*
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
* EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
* MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
*
* See the Mulan PSL v2 for more details.
***************************************************************************************/

#include <isa.h>
#include <cpu/cpu.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <memory/paddr.h>
#include "sdb.h"
#include "wp.h"
static int is_batch_mode = false;

void init_regex();
void init_wp_pool();


/* We use the `readline' library to provide more flexibility to read from stdin. */
static char* rl_gets() {
	//string to save each line to read
  static char *line_read = NULL;
//line to read is already uesd, so it needs to be free.
//free the memory, and the pointer needs to be NULL.
  if (line_read) {
    free(line_read);
    line_read = NULL;
  }

  line_read = readline("(nemu) ");

  if (line_read && *line_read) {
    add_history(line_read);
  }

  return line_read;
}

static int cmd_si(char *args) {
	int n;
	sscanf(args, "%d", &n);
	cpu_exec(n);
	return 0;
}

static int cmd_info(char *args){
	if (strcmp(args, "r") == 0)
		isa_reg_display();
	else if (strcmp(args, "w")==0)
		print_wp_val();
	else
		printf("unknown command\n");
	return 0;
}

static int cmd_c(char *args) {
  cpu_exec(-1);
  return 0;
}


static int cmd_q(char *args) {
  return -1;
}

static int cmd_help(char *args);

static int cmd_x(char *args){
	char *arg1 = strtok(NULL, " ");
	if(arg1 == NULL){
		printf("print the start address and the nums of data.\n");
		return 0;
	}
	int N;
	sscanf(arg1, "%d", &N);
	char *arg2 = strtok(NULL, " ");
	if(arg2 == NULL){
		printf("print the start address.\n");
		return 0;
	}
	paddr_t addr;
	sscanf(arg2, "%x", &addr);
	//uint8_t *haddr = guest_to_host(addr);
	printf("%x: ", addr);
	for (int i=0;i<N;i++){
		printf("%08lx ", paddr_read(addr+4*i, 4));
	}
	printf("\n");
	return 0;
}
static int cmd_p(char* args){
	bool success, flag;
	printf("%ld\n", expr(args, &success));
	//save expr, chaeck_equal() save value.
	change_expr(args);
	return 0;
}
static int cmd_w(char *args){
	WP *wp = new_wp();
	if(wp==NULL) {printf("no space for more wp.\n"); return 0;}
	bool success;
	strcpy(wp->expr, args);
	wp->val = expr(args, &success);
	if(!success) {printf("can't compute the value of expr.\n"); return 0;}
	printf("wp NO.%d: val:%d\n", wp->NO, wp->val);
	return 0;
}
static int cmd_d(char *args){
	int no;
	sscanf(args, "%d", &no);
	WP *free_node = find_wp(no);
	if(free_node==NULL){
		printf("can't find this wp in head linked list.\n");
		return -1;
	}
	free_wp(free_node);
	return 0;
}
static struct {
  const char *name;
  const char *description;
  int (*handler) (char *);
} cmd_table [] = {
  { "help", "Display information about all supported commands", cmd_help },
  { "c", "Continue the execution of the program", cmd_c },
  { "q", "Exit NEMU", cmd_q },
  { "si", "input steps to execute the program", cmd_si},
  { "info", "print the value of registers or the watchpoints", cmd_info},
  { "x", "continuously output N 4-byte data from start address", cmd_x},
  { "p", "calculate the value of expression", cmd_p},
  { "d", "delete wp node.", cmd_d},
  { "w", "create a new wp.", cmd_w}
  /* TODO: Add more commands */

};

#define NR_CMD ARRLEN(cmd_table)

static int cmd_help(char *args) {
  /* extract the first argument */
  char *arg = strtok(NULL, " ");
  int i;

  if (arg == NULL) {
    /* no argument given */
    for (i = 0; i < NR_CMD; i ++) {
      printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
    }
  }
  else {
    for (i = 0; i < NR_CMD; i ++) {
      if (strcmp(arg, cmd_table[i].name) == 0) {
        printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
        return 0;
      }
    }
    printf("Unknown command '%s'\n", arg);
  }
  return 0;
}

void sdb_set_batch_mode() {
  is_batch_mode = true;
}

void sdb_mainloop() {
  if (is_batch_mode) {
    cmd_c(NULL);
    return;
  }

  for (char *str; (str = rl_gets()) != NULL; ) {
    char *str_end = str + strlen(str);

    /* extract the first token as the command */
    char *cmd = strtok(str, " ");
    if (cmd == NULL) { continue; }
    /* treat the remaining string as the arguments,
     * which may need further parsing
     */
    //only cmd, no args.
    char *args = cmd + strlen(cmd) + 1;
    if (args >= str_end) {
      args = NULL;
    }

#ifdef CONFIG_DEVICE
    extern void sdl_clear_event_queue();
    sdl_clear_event_queue();
#endif

    int i;
    for (i = 0; i < NR_CMD; i ++) {
      if (strcmp(cmd, cmd_table[i].name) == 0) {
	      //cmd_help, cmd_c, cmd_q return negative value.
        if (cmd_table[i].handler(args) < 0) { return; }
        break;
      }
    }

    if (i == NR_CMD) { printf("Unknown command '%s'\n", cmd); }
  }
}

void init_sdb() {
  /* Compile the regular expressions. */
  init_regex();

  /* Initialize the watchpoint pool. */
  init_wp_pool();
}
