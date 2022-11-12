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

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <assert.h>
#include <string.h>

// this should be enough
static char buf[65536] = {};
static int overflow = 0;
static int ind = 0;
static char code_buf[65536 + 128] = {}; // a little larger than `buf`
static char *code_format =
"#include <stdio.h>\n"
"int main() { "
"  unsigned result = %s; "
"  printf(\"%%d\", result); "
"  return 0; "
"}";
uint32_t choose(uint32_t n){
  uint32_t i;
  i = rand()%n;
  return i;
}
void gen_space(){
        int n = choose(3);
        for (int i=0;i<n;i++){
		/*if(ind==99)
			break;
                buf[ind]=' ';
                ind++;*/
		if(strlen(buf)+1>=65535) {overflow = -1; return ;}
		strcat(buf, " ");
        }
}
void gen_num(){
	//uint32_t guarantee unsigned int operations.
	//rand space?
	//long expr?
	//divide 0?
	char str[20];
	sprintf(str, "%u", choose(100));
	/*strncpy(buf+ind, str, 99-ind);
	if (strlen(str)>99-ind)
		ind = 99;
	else
		ind += strlen(str);*/
	if (strlen(buf)+strlen(str)>=65535) {overflow = -1; return ;}
	strcat(buf, str);
}
void gen(char c){
	/*if(ind<99){
	buf[ind]=c;
	ind++;}
*/
	char str[] = {c, '\0'};
	if(strlen(buf)+strlen(str)>=65535) {overflow = -1; return ;}
	strcat(buf, str);
}
void gen_rand_op(){
	//integer overflow cannot be considered, because prog will gen another expr automatically.
	char op_list[4] = {'+', '-', '*', '/'};
	int i = choose(4);
	/*if(ind<99){
	buf[ind]=op_list[i];
	ind++;}*/
	char str[] = {op_list[i], '\0'};
	if(strlen(buf)+strlen(str)>=65535) {overflow = -1; return ;}
	strcat(buf, str);
}
static void gen_rand_expr() {
	//if here is a "buf[0]='\0'", value in buf will be erased so there
	//is no left '('.
  do{
  switch(choose(3)){
        case 0: gen_num();/* printf("%d\n", overflow);*/ break;
        case 1: gen('('); /*printf("%d\n", overflow);*/gen_rand_expr(); /*printf("%d\n", overflow);*/gen(')'); /*printf("%d\n", overflow);*/ break;
        default: gen_rand_expr();/*printf("%d\n", overflow);*/ gen_rand_op();/*printf("%d\n", overflow);*/ gen_rand_expr();/*printf("%d\n", overflow);*/ break;
  }
  if (overflow<0){
  	buf[0] = '\0';
	overflow = 0;
  }
  }while(strlen(buf)==0);
}

int main(int argc, char *argv[]) {
  unsigned int seed = time(0);
  srand(seed);
  int loop = 1;
  if (argc > 1) {
    sscanf(argv[1], "%d", &loop);
  }
  int i;
  for (i = 0; i < loop; i ++) {
	  buf[0] = '\0';
    gen_rand_expr();
    sprintf(code_buf, code_format, buf);

    FILE *fp = fopen("/tmp/.code.c", "w");
    assert(fp != NULL);
    fputs(code_buf, fp);
    fclose(fp);

    nt ret = system("gcc -Wall -Werror /tmp/.code.c -o /tmp/.expr");
    if (ret != 0) {loop++; continue;}

    fp = popen("/tmp/.expr", "r");
    assert(fp != NULL);
    int result;
    fscanf(fp, "%d", &result);
    pclose(fp);

    printf("%d %s\n", result, buf);
  }
  return 0;
}
