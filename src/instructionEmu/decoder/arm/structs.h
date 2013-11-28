#ifndef __INSTRUCTION_EMU__DECODER__ARM__STRUCTS_H__
#define __INSTRUCTION_EMU__DECODER__ARM__STRUCTS_H__

#include "common/types.h"

#define ADD_IMMEDIATE_BASE_VALUE    0x02800000
#define SUB_IMMEDIATE_BASE_VALUE    0x02400000
#define BRANCH_BASE_VALUE           0x0a000000
#define LDM_STM_BASE_VALUE          0x08000000
#define LDR_IMMEDIATE_BASE_VALUE    0x04100000
#define STR_IMMEDIATE_BASE_VALUE    0x04000000
#define MRC_BASE_VALUE              0x0e100010
#define MCR_BASE_VALUE              0x0e000010

typedef union
{
  struct armALUImmediateInstruction
  {
    u32int imm12 : 12;
    u32int Rd : 4;
    u32int Rn : 4;
    u32int S : 1;
    u32int pad0 : 4;
    u32int I : 1;
    u32int pad1 : 2;
    u32int cond : 4;
  } fields;
  u32int value;
} ARM_ALU_imm;


typedef union
{
  struct armALURegisterInstruction
  {
    u32int Rm : 4;
    u32int pad0 : 1;
    u32int type : 2;
    u32int imm5 : 5;
    u32int Rd : 4;
    u32int Rn : 4;
    u32int S : 1;
    u32int pad1 : 4;
    u32int I : 1;
    u32int pad2 : 2;
    u32int cond : 4;
  } fields;
  u32int value;
} ARM_ALU_reg;


typedef union
{
  struct armLdmStmInstruction
  {
    u32int register_list : 16;
    u32int Rn : 4;
    u32int L : 1;
    u32int W : 1;
    u32int S : 1;
    u32int U : 1;
    u32int P : 1;
    u32int pad : 3;
    u32int cond : 4;
  } fields;
  u32int value;
} ARM_ldm_stm;


typedef union
{
  struct armLdrStrImmediateInstruction
  {
    u32int imm12 : 12;
    u32int Rt : 4; // source
    u32int Rn : 4; // base
    u32int L : 1;
    u32int W : 1;
    u32int B : 1;
    u32int U : 1;
    u32int P : 1;
    u32int I : 1;
    u32int pad: 2;
    u32int cond : 4;
  } fields;
  u32int value;
} ARM_ldr_str_imm;


typedef union
{
  struct armLdrStrRegister
  {
    u32int Rm : 4; // source
    u32int pad0: 1;
    u32int type: 2;
    u32int imm5: 5;
    u32int Rt : 4; // source
    u32int Rn : 4; // base
    u32int L : 1;
    u32int W : 1;
    u32int B : 1;
    u32int U : 1;
    u32int P : 1;
    u32int I : 1;
    u32int pad: 2;
    u32int cond : 4;
  } fields;
  u32int value;
} ARM_ldr_str_reg;


typedef union
{
  struct armMrcInstruction
  {
    unsigned CRm : 4;
    unsigned pad0 : 1;
    unsigned opc2 : 3;
    unsigned coproc : 4;
    unsigned Rt : 4;
    unsigned CRn : 4;
    unsigned pad1 : 1;
    unsigned opc1 : 3;
    unsigned pad3 : 4;
    unsigned cc : 4;
  } fields;
  u32int value;
} ARMMrcInstruction;

typedef union
{
  struct armMcrInstruction
  {
    unsigned CRm : 4;
    unsigned pad0 : 1;
    unsigned opc2 : 3;
    unsigned coproc : 4;
    unsigned Rt : 4;
    unsigned CRn : 4;
    unsigned pad1 : 1;
    unsigned opc1 : 3;
    unsigned pad3 : 4;
    unsigned cc : 4;
  } fields;
  u32int value;
} ARMMcrInstruction;


// this union will hold possible encodings for instructions
// each struct must be 32 bits exactly
// instruction as a whole can be accesssed via 'raw' field
typedef union ARMInstruction
{
  struct Breakpoint
  {
    unsigned imm4:   4;
    unsigned pad0:   4;
    unsigned imm12: 12;
    unsigned pad1:   8;
    unsigned cc:     4;
  } bkpt;

  struct Branch
  {
    unsigned imm24: 24;
    unsigned pad0:   4;
    unsigned cc:     4;
  } branch;

  struct BxReg
  {
    unsigned Rm:     4;
    unsigned pad0:  24;
    unsigned cc:     4;
  } BxReg;

  struct Cps
  {
    unsigned mode:   5;
    unsigned pad0:   1;
    unsigned F:      1;
    unsigned I:      1;
    unsigned A:      1;
    unsigned pad1:   8;
    unsigned M:      1;
    unsigned imod:   2;
    unsigned pad2:  12;
  } cps;

  struct LoadImm
  {
    unsigned imm12: 12;
    unsigned Rt:     4;
    unsigned Rn:     4;
    unsigned pad0:   1;
    unsigned W:      1;
    unsigned pad1:   1;
    unsigned U:      1;
    unsigned P:      1;
    unsigned pad2:   3;
    unsigned cc:     4;
  } loadImm;

  struct LoadReg
  {
    unsigned Rm:     4;
    unsigned pad0:   1;
    unsigned type:   2;
    unsigned imm5:   5;
    unsigned Rt:     4;
    unsigned Rn:     4;
    unsigned pad1:   1;
    unsigned W:      1;
    unsigned pad2:   1;
    unsigned U:      1;
    unsigned P:      1;
    unsigned pad3:   3;
    unsigned cc:     4;
  } loadReg;

  struct Ldrex
  {
    unsigned pad0:  12;
    unsigned Rt:     4;
    unsigned Rn:     4;
    unsigned pad1:   8;
    unsigned cc:     4;
  } ldrex;

  struct Mcr
  {
    unsigned CRm:    4;
    unsigned pad0:   1;
    unsigned opc2:   3;
    unsigned coproc: 4;
    unsigned Rt:     4;
    unsigned CRn:    4;
    unsigned pad1:   1;
    unsigned opc1:   3;
    unsigned pad2:   4;
    unsigned cc:     4;
  } mcr;

  struct Mrs
  {
    unsigned pad0:  12;
    unsigned Rd:     4;
    unsigned pad1:   6;
    unsigned R:      1;
    unsigned pad2:   5;
    unsigned cc:     4;
  } mrs;

  struct MsrImm
  {
    unsigned imm12: 12;
    unsigned pad0:   4;
    unsigned mask:   4;
    unsigned pad1:   2;
    unsigned R:      1;
    unsigned pad2:   5;
    unsigned cc:     4;
  } msrImm;

  struct MsrReg
  {
    unsigned Rn:     4;
    unsigned pad0:  12;
    unsigned mask:   4;
    unsigned pad1:   2;
    unsigned R:      1;
    unsigned pad2:   5;
    unsigned cc:     4;
  } msrReg;

  struct Strex
  {
    unsigned Rt:     4;
    unsigned pad0:   8;
    unsigned Rd:     4;
    unsigned Rn:     4;
    unsigned pad1:   8;
    unsigned cc:     4;
  } strex;

  u32int raw;
} Instruction;

#endif /* __INSTRUCTION_EMU__DECODER__ARM__STRUCTS_H__ */
