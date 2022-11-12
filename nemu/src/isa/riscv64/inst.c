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

#include "local-include/reg.h"
#include <cpu/cpu.h>
#include <cpu/ifetch.h>
#include <cpu/decode.h>

#define R(i) gpr(i)
#define Mr vaddr_read
#define Mw vaddr_write
//R(i)->src1 src2: unsigned 64
//imm: unsigned 64
enum {
  TYPE_I, TYPE_U, TYPE_S,
  TYPE_N, TYPE_J, TYPE_B, TYPE_R,// none
};
//get 64 bit imm
#define src1R() do { *src1 = R(rs1); } while (0)
#define src2R() do { *src2 = R(rs2); } while (0)
#define immI() do { *imm = SEXT(BITS(i, 31, 20), 12); } while(0)
#define immU() do { *imm = SEXT(BITS(i, 31, 12), 20) << 12; } while(0)
#define immS() do { *imm = (SEXT(BITS(i, 31, 25), 7) << 5) | BITS(i, 11, 7);} while(0)
#define immB() do { *imm = (SEXT(BITS(i, 31, 31), 1) << 12) | (BITS(i, 30, 25) << 5) | (BITS(i, 11, 8) << 1) | (BITS(i, 7, 7) << 11);}while(0)
#define immJ() do { *imm = (SEXT(BITS(i, 31, 31), 1) << 20) | (BITS(i, 30, 21) << 1) | (BITS(i, 20, 20) << 11) | (BITS(i, 19, 12) << 12);} while(0)

static void decode_operand(Decode *s, int *dest, word_t *src1, word_t *src2, word_t *imm, int type) {
  uint32_t i = s->isa.inst.val;
  int rd  = BITS(i, 11, 7);
  int rs1 = BITS(i, 19, 15);
  int rs2 = BITS(i, 24, 20);
  *dest = rd;
  switch (type) {
    case TYPE_I: src1R();          immI(); break;
    case TYPE_U:                   immU(); break;
    case TYPE_S: src1R(); src2R(); immS(); break;
    case TYPE_J:                   immJ(); break;
    case TYPE_B: src1R(); src2R(); immB(); break;
    case TYPE_R: src1R(); src2R();         break;
  }
}
int i=1;
static int decode_exec(Decode *s) {
  int dest = 0;
  word_t src1 = 0, src2 = 0, imm = 0;
  s->dnpc = s->snpc;

#define INSTPAT_INST(s) ((s)->isa.inst.val)
#define INSTPAT_MATCH(s, name, type, ... /* execute body */ ) { \
  decode_operand(s, &dest, &src1, &src2, &imm, concat(TYPE_, type)); \
  __VA_ARGS__ ; \
}

  printf("%0lx\n", s->pc);
  //printf("%0lx\n", R(10));
  INSTPAT_START();
  //printf("starting %d matching\n", i++);
  INSTPAT("??????? ????? ????? ??? ????? 00101 11", auipc  , U, R(dest) = s->pc + imm);
  INSTPAT("??????? ????? ????? 011 ????? 00000 11", ld     , I, R(dest) = Mr(src1 + imm, 8));
  INSTPAT("??????? ????? ????? 011 ????? 01000 11", sd     , S, Mw(src1 + imm, 8, src2));
  INSTPAT("0000000 00001 00000 000 00000 11100 11", ebreak , N, NEMUTRAP(s->pc, R(10))); // R(10) is $a0
  //TODO opcode in the string at last represents operations
  //op-type(I,U,R,S..) means what value to get.
  //different opcodes may have the same inst type, so have the same decode operation. Concrete execution is relative to funct, bust mostly is just addition.
  //if-else80 load-store60 mullonglong94 
  //pascal, wanshu keep looping
  //quicksort6c
  //recursion 1cc sum54 tolowercase84 unalign30
  INSTPAT("??????? ????? ????? 000 ????? 00100 11", addi   , I, R(dest) = src1+imm);
  INSTPAT("??????? ????? ????? ??? ????? 11011 11", jal    , J, s->dnpc = s->pc + imm; R(dest) = s->pc + 4);
  INSTPAT("??????? ????? ????? ??? ????? 11001 11", jalr   , I, s->dnpc = (src1+imm) & (uint64_t)(-2); R(dest) = s->pc + 4);
  INSTPAT("??????? ????? ????? 000 ????? 11000 11", beq    , B, if(src1==src2) s->dnpc = s->pc+imm);//
  INSTPAT("??????? ????? ????? 001 ????? 11000 11", bne    , B, if(src1!=src2) s->dnpc = s->pc+imm);//
  INSTPAT("??????? ????? ????? 101 ????? 11000 11", bge    , B, if((int64_t)src1>=(int64_t)src2) s->dnpc = s->pc+imm);//
  INSTPAT("??????? ????? ????? 111 ????? 11000 11", bgeu   , B, if(src1>=src2) s->dnpc = s->pc+imm);//
  INSTPAT("??????? ????? ????? 100 ????? 11000 11", blt   , B, if((int64_t)src1<(int64_t)src2) s->dnpc = s->pc+imm);//
  INSTPAT("??????? ????? ????? 110 ????? 11000 11", bltu   , B, if(src1<src2) s->dnpc = s->pc+imm);//
  INSTPAT("??????? ????? ????? ??? ????? 00110 11", addiw  , I, R(dest) = SEXT(BITS(src1 + imm, 31, 0), 32));
  INSTPAT("??????? ????? ????? ??? ????? 01110 11", addw   , R, R(dest) = SEXT(BITS(src1 + src2, 31, 0), 32));
  INSTPAT("??????? ????? ????? 010 ????? 00000 11", lw     , I, R(dest) = SEXT(Mr(src1+imm, 8), 32));
  INSTPAT("??????? ????? ????? 100 ????? 00000 11", lbu     , I, R(dest) = ZEXT(Mr(src1+imm, 2), 8));
  INSTPAT("0000000 ????? ????? 000 ????? 01100 11", add    , R, R(dest) = src1 + src2);
  INSTPAT("0100000 ????? ????? 000 ????? 01100 11", sub    , R, R(dest) = src1 - src2);
  INSTPAT("??????? ????? ????? 011 ????? 00100 11", sltiu  , I, R(dest) = (src1<imm));
  INSTPAT("??????? ????? ????? 011 ????? 01100 11", sltu   , R, R(dest) = (src1<src2));
  INSTPAT("??????? ????? ????? 001 ????? 01000 11", sh     , S, Mw(src1 + imm, 4, BITS(src2, 15, 0)));
  INSTPAT("??????? ????? ????? 000 ????? 01000 11", sb     , S, Mw(src1 + imm, 2, BITS(src2, 7, 0)));
  INSTPAT("??????? ????? ????? 010 ????? 01000 11", sw     , S, Mw(src1 + imm, 8, BITS(src2, 31, 0)));
  INSTPAT("??????? ????? ????? 011 ????? 01000 11", sd     , S, Mw(src1 + imm, 16, src2));
  INSTPAT("??????? ????? ????? 100 ????? 00100 11", xori   , I, R(dest) = src1 ^ imm);
  INSTPAT("??????? ????? ????? 111 ????? 00100 11", andi   , I, R(dest) = src1 & imm);
  INSTPAT("??????? ????? ????? 111 ????? 01100 11", and    , R, R(dest) = src1 & src2);
  //when shifting, for extends, uint64_t converts to int64_t.
  //when loading, u and not u have different extends ways.(zext sext)
  //when comparing, u and not u have different data types.(signed unsigned)
  INSTPAT("010000 ?????? ????? 101 ????? 00100 11", srai   , I, R(dest) = ((int64_t)(src1) >> BITS(imm, 4, 0)));
  INSTPAT("000000 ?????? ????? 101 ????? 00100 11", srli   , I, R(dest) = src1>>BITS(imm, 4, 0));
  INSTPAT("000000 ?????? ????? 001 ????? 00100 11", slli   , I, R(dest) = src1 << BITS(imm, 4, 0));
  INSTPAT("0000000 ????? ????? 001 ????? 01110 11", sllw   , R, R(dest) = src1 << src2);
  INSTPAT("0000000 ????? ????? 101 ????? 01110 11", srlw   , R, R(dest) = src1 >> src2);
  INSTPAT("0100000 ????? ????? 101 ????? 01110 11", sraw   , R, R(dest) = (uint64_t)((int64_t)(src1) >> src2));
  //
 //INV must be put in the end.
 //the same format(TYPE_X), different instructions, the same decoding(get src1, imm). addi, addiw
 //the same intruction, different formats, the same execution. addiw addw
 //some instructions are the same, although they have different names.
  INSTPAT("??????? ????? ????? ??? ????? ????? ??", inv    , N, INV(s->pc));
  INSTPAT_END();
//All imms are extended to unsigned 64 bits. when comparing, they're unsigned.
  R(0) = 0; // reset $zero to 0
  printf("get to the end and R(10): %lx\n", R(10));

  return 0;
}
int isa_exec_once(Decode *s) {
  s->isa.inst.val = inst_fetch(&s->snpc, 4);
  return decode_exec(s);
}
