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

#include "sdb.h"
#include "wp.h"

void init_wp_pool() {
  int i;
  for (i = 0; i < NR_WP; i ++) {
    wp_pool[i].NO = i;
    wp_pool[i].val = 0;
    wp_pool[i].pre_val = 0;
    wp_pool[i].expr[0] = '\0';
    wp_pool[i].next = (i == NR_WP - 1 ? NULL : &wp_pool[i + 1]);
  }

  head = NULL;
  free_ = wp_pool;
}

/* TODO: Implement the functionality of watchpoint */
WP* new_wp(){
	//to head tail
	//expr of new needs to be initialized.
	if(free_ == NULL) return NULL;
	WP *new = free_;
	free_ = new->next;
	new->next = NULL;
	if(head == NULL) head = new;
	else {
		WP *h = head;
		while(h->next!=NULL){
			h = h->next;
		}
		h->next = new;
	}
	return new;
}
//give back watch point to linked-list free_.
void free_wp(WP *wp){
	//to free_ head
	if(head==wp) head = wp->next;
	else {
		WP *h=head;
		while(h!=NULL&&h->next!=wp){
			h = h->next;
		}
		if(h == NULL)
			assert(0);
		h->next = h->next->next;
	}
	wp->next = free_;
	free_ = wp;
	wp->val = 0;
	wp->expr[0] = '\0';
}
WP *find_wp(int no){
	for (WP *h=head;h!=NULL;h=h->next){
		if(no==h->NO)
			return h;
	}
	return NULL;
}
void print_wp_val(){
	for (WP *h=head;h!=NULL;h=h->next){
		printf("NO.%d: %s, %d\n", h->NO, h->expr, h->val);
	}
}
/*void save_wp_state(char *expr){
	for(WP *h=head;h!=NULL;h=h->next){
		if(strlen(h->next->next)==0){
			bool success;
			int val;
		if(val != h->val){
			printf("NO%d watchpoint, old value:%d, new value:%d\n", h->NO, h->val, val);
			h->val = val;
			strcpy(h->expr, expr);
			*flag = true;
		}
		}
	}
}*/
void check_equal(bool *flag){
	for(WP *h=head;h!=NULL;h=h->next){
		if(h->next==NULL){
		bool success;
		int val;
	        val = expr(h->expr, &success);
		if(val != h->val){
			printf("NO.%d watchpoint, old value:%d, new value:%d\n", h->NO, h->val, val);
			h->val = val;
			*flag = true;
			return ;
		}}
	}
	*flag = false;
}
void change_expr(char *e){
	for (WP *h=head; h!=NULL; h=h->next){
		if(h->next==NULL){
			strcpy(h->expr, e);
			break;
		}
	}
}
