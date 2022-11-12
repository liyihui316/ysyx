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
#include <stdlib.h>
#include <time.h>
#include <memory/paddr.h>
/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <regex.h>

enum {
  TK_NOTYPE = 256, TK_EQ, TK_UNEQ, TK_LE,
  TK_LBK, TK_RBK,
  TK_MUL, TK_DIV,
  TK_ADD, TK_SUB,
  TK_NUM, TK_HEX, TK_REG, TK_DEREF, TK_AND
  /* TODO: Add more token types */
};

static struct rule {
  const char *regex;
  int token_type;
} rules[] = {

  /* TODO: Add more rules.
   * Pay attention to the precedence level of different rules.
   */

  {" +", TK_NOTYPE},    // spaces
  {"\\+", TK_ADD},         // plus
  {"==", TK_EQ},        // equal
  {"-", TK_SUB},          //sub
  {"\\*", TK_MUL},         //multiply
  {"/", TK_DIV},          //divide
  {"\\(", TK_LBK},
  {"\\)", TK_RBK},
  //match hex first, because there' s 0 in hex, maybe causing match dec.
  {"0[xX][0-9a-fA-F]+", TK_HEX},
  {"[0-9]+", TK_NUM},
  {"!=", TK_UNEQ},
  {"<=", TK_LE},
  {"\\$[a-z0-9]+", TK_REG},
  {"&&", TK_AND}
};

#define NR_REGEX ARRLEN(rules)

static regex_t re[NR_REGEX] = {};

/* Rules are used for many times.
 * Therefore we compile them only once before any usage.
 */
bool check_parentheses(int p, int q);
int find_main_op(int p, int q);
void init_regex() {
  int i;
  char error_msg[128];
  int ret;

  for (i = 0; i < NR_REGEX; i ++) {
    ret = regcomp(&re[i], rules[i].regex, REG_EXTENDED);
    if (ret != 0) {
      regerror(ret, &re[i], error_msg, 128);
      panic("regex compilation failed: %s\n%s", error_msg, rules[i].regex);
    }
  }
}

typedef struct token {
  int type;
  char str[32];
} Token;

static Token tokens[65536] __attribute__((used)) = {};
#define TOKEN_SIZE sizeof(tokens)/sizeof(tokens[0])
static int nr_token __attribute__((used))  = 0;
static int eval(int p, int q){
//	printf("%d %d\n", p, q);
	if(p > q){
		assert(0);
	}
	else if (p == q){
		//hex, reg and dec
		if (tokens[p].type == TK_HEX){
			char split_str[] = {tokens[p].str[1], '\0'};
			strtok(tokens[p].str, split_str);
			char *hex_str = strtok(NULL, "");
			//printf("%s\n", hex_str);
			uint32_t hex;
			sscanf(hex_str, "%x", &hex);
			return hex;
			//save data in hex format, just print it in dec format.
		}
		if(tokens[p].type == TK_REG){
			bool success;
			//char split_str[] = {tokens[p].str[0], '\0'};
			//strtok(tokens[p].str, split_str);
			char final_str[10]={'\0'};
			strncpy(final_str, tokens[p].str+1, strlen(tokens[p].str)-1);
			printf("%s\n", final_str);
			uint32_t regval = isa_reg_str2val(final_str, &success);
			assert(success);
			return regval;
		}
		uint32_t a;
		sscanf(tokens[p].str, "%d", &a);
		return a;
	}
	else if (check_parentheses(p, q) == true){
		return eval(p+1, q-1);
	}
	else {
		int op = find_main_op(p, q);
		int val1, val2;
		if (tokens[op].type != TK_DEREF)
		val1 = eval(p, op-1);
		else
			val1 = 0;
//		printf("val1:%d\n", val1);	
		val2 = eval(op+1, q);
//		printf("val2:%d\n", val2);
//		printf("op:%d\n", tokens[op].type);

		switch(tokens[op].type){
			case TK_DEREF: uint32_t raddr_start = rand()%100+0x80000000;
				       uint32_t raddr = paddr_read(raddr_start, 4);
				       return raddr;
			case TK_ADD: return (val1+val2);
			case TK_SUB: return (val1-val2);
			case TK_MUL: return (val1*val2);
			case TK_DIV: return (val1/val2);
			case TK_EQ: return (val1==val2);
			case TK_UNEQ: return (val1!=val2);
			case TK_LE: return (val1<=val2);
			case TK_AND: return (val1&&val2);
			//return a random address.
			default: assert(0);
		}
	}
}
int find_main_op(int p, int q){
	int in_bk=0;
	int i;
	int flag = 0;
	int aflag = 0;
	int leflag = 0;
	int eflag = 0;
	int nflag = 0;
	int mdflag = 0;
	for (i=p;i<=q;i++){
		if(tokens[i].type==TK_NUM||tokens[i].type==TK_HEX||tokens[i].type==TK_REG)
			continue;
		//here must legal expression with brakets.
		if(tokens[i].type==TK_LBK){
			in_bk++;
			continue;
		}
		if(tokens[i].type==TK_RBK){
			in_bk--;
			continue;
		}
		if(in_bk>0) continue;
		//eq, uneq, le, and, deref are all including priority judge.
		if(tokens[i].type == TK_AND){
			flag = i;
			nflag = 1;		
		}
		if ((tokens[i].type == TK_EQ || tokens[i].type == TK_UNEQ || tokens[i].type == TK_LE)&&(!nflag)){
			flag = i;
			eflag = 1;	
		}
		if (tokens[i].type == TK_LE&&((!nflag)&&(!eflag))){
			flag = i;
			leflag = 1;
		}
		if ((tokens[i].type == TK_ADD || tokens[i].type == TK_SUB)&&((!nflag)&&(!eflag)&&(!leflag))){
			flag = i;
			aflag=1;
		}
		if ((tokens[i].type == TK_MUL || tokens[i].type == TK_DIV) && ((!nflag)&&(!eflag)&&(!leflag)&&(!aflag))){
			flag = i;
			mdflag = 1;
		}
		//while dereferencing, find the dominant op.-> eval() ->expr()
		if(tokens[i].type == TK_DEREF && ((!nflag)&&(!eflag)&&(!leflag)&&(!aflag)&&(!mdflag))){
			flag = i;
		}
	}
	i = flag;
	return i;
}
bool check_parentheses(int p, int q){
	int match = 0;
	int m_eq0 = 0;
	for (int i=p;i<=q;i++){
		if(tokens[i].type == TK_LBK){
			match = match+1;
		}
		if(tokens[i].type == TK_RBK){
			match = match-1;
		}
		if(match == 0 && i != q) m_eq0 = 1;
		if(match<0) assert(0);
	}
	if(match>0)
		assert(0);
	if(match==0 && m_eq0==1) 
		return false;
	return true;
}
/*	int i;
	if(tokens[p].type=='(' && tokens[q].type==')'){
		int lc=0, rc=0;
		for(i=p+1;i<q;i++){
			if(tokens[i].type=='(') lc++;
			if(tokens[i].type==')') rc++;
			if(rc>lc) return false;
		}
		if(rc==lc) return true;
	}
	return false;
}*/
/*uint32_t choose(uint32_t n){
	srand((unsigned int)time(NULL));
	uint32_t t = rand()%3;
	return t;
}
void gen_rand_expr(){
	switch(choose(3)){
		case 0:gen_num(); break;
		case 1: gen('('); gen_rand_expr(); gen(')'); break;
		default: gen_rand_expr(); gen_rand_op(); gen_rand_expr(); break;
	}
}*/
static bool make_token(char *e) {
  int position = 0;
  int i;
  regmatch_t pmatch;

  nr_token = 0;
  for (int i=0;i<TOKEN_SIZE;i++) memset(tokens[i].str, 0, sizeof(tokens[i].str)/sizeof(tokens[i].str[0]));
  while (e[position] != '\0') {
    /* Try all rules one by one. */
    for (i = 0; i < NR_REGEX; i ++) {
	    //position is matched right now, search all regex to match it.
	    //one each time.
	    //pmatc.rm_so == 0 means the matched str is right this position, not brfore or after.
      if (regexec(&re[i], e + position, 1, &pmatch, 0) == 0 && pmatch.rm_so == 0) {
        char *substr_start = e + position;
        int substr_len = pmatch.rm_eo;

        //printf("match rules[%d] = \"%s\" at position %d with len %d: %.*s\n",
          //  i, rules[i].regex, position, substr_len, substr_len, substr_start);

        position += substr_len;
	tokens[nr_token].type = rules[i].token_type ;
	strncpy(tokens[nr_token].str, substr_start, substr_len);
	nr_token++;
	//printf("%d\n", nr_token);
        /* TODO: Now a new token is recognized with rules[i]. Add codes
         * to record the token in the array `tokens'. For certain types
        * of tokens, some extra actions should be performed.
         */

	/*tokens[i].type = rules[i].token_type ;
	switch(rules[i].token_type){
		case TK_NOTYPE: 
		default:	
	}*/

        break;
      }
    }

    if (i == NR_REGEX) {
      printf("no match at position %d\n%s\n%*.s^\n", position, e, position, "");
      return false;
    }
  }

  return true;
}


word_t expr(char *e, bool *success) {
  if (!make_token(e)) {
    *success = false;
    return 0;
  }
  else *success = true;
 // printf("%d\n", eval(0, nr_token-1));
  // TODO: Insert codes to evaluate the expression.
  //printf("nr_token:%d\n", nr_token);
  for (int i=0;i<nr_token;i++){
  	if (tokens[i].type==TK_MUL && (i==0||(tokens[i-1].type!=TK_RBK&&tokens[i-1].type!=TK_NUM&&tokens[i-1].type!=TK_HEX&&tokens[i-1].type!=TK_REG))){
		tokens[i].type = TK_DEREF;
	}
  }
  return eval(0, nr_token-1);
}
