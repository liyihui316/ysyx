#ifndef __WP_H__
#define __WP_H__
#include <common.h>
#include "sdb.h"

#define NR_WP 32
typedef struct watchpoint{
        int NO;
        int val;
	int pre_val;
        char expr[65536];
        struct watchpoint *next;
} WP;
static WP wp_pool[NR_WP]={};
static WP* head=NULL;
static WP* free_=NULL;
void init_wp_pool();
WP *new_wp();
void free_wp(WP *wp);
WP *find_wp(int No);
void print_wp_val();
void check_equal(bool *flag);
void change_expr(char *e);
#endif
