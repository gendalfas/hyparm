#ifndef __INSTRUCTION_EMU__INTERPRETER__INTERNALS_H__
#define __INSTRUCTION_EMU__INTERPRETER__INTERNALS_H__

/*
 * Functionality specific to the interpreter.
 *
 * DO NOT use or include this file outside the interpreter.
 */


#include "common/compiler.h"
#include "common/debug.h"

#include "cpuArch/constants.h"

#include "guestManager/guestContext.h"


#define DEBUG_TRACE(what, context, instruction)                                                    \
  DEBUG(what, "%s: %#.8x @ %#.8x" EOL, __func__, context->R15, instruction);

#define TRACE(context, instruction)                                                                \
  printf("%s: %#.8x @ %#.8x" EOL, __func__, context->R15, instruction);


#define ARM_EXTRACT_CONDITION_CODE(instructionWord)         (instructionWord >> 28)
#define ARM_EXTRACT_REGISTER(instructionWord, position)     ((instructionWord >> position) & 0xF)
#define ARM_SET_REGISTER(instructionWord, position, value)                                         \
  ((instructionWord & ~(0xF << position)) | (value << position))

#define T16_EXTRACT_LOW_REGISTER(instructionWord, position) ((instructionWord >> position) & 0x7)


typedef enum
{
  ADD,
  ADC,
  SUB,
  SBC,
  RSB,
  RSC,
  AND,
  ORR,
  EOR,
  BIC,
  MOV,
  MVN,
  CMP,
  CMN,
  TST,
  TEQ,
  LSL,
  LSR,
  ASR,
  RRX,
  ROR
} OPTYPE;


u32int arithLogicOp(GCONTXT *context, u32int instr, OPTYPE opType, const char *instrString);

/* expand immediate12 field of instruction */
u32int armExpandImm12(u32int imm12) __constant__;

// take shift type field from instr, return shift type
u32int decodeShift(u32int instrShiftType) __constant__;

// take the imm5 shift amount and shift type field from instr
// returns shift type, and adjusts shift amount
u32int decodeShiftImmediate(u32int instrShiftType, u32int imm5, u32int *shamt);

/* a function to evaluate if a condition value is satisfied */
bool evaluateConditionCode(GCONTXT *context, u32int conditionCode);

/* function to load a register value from r0--r7 */
__macro__ u32int getLowGPRegister(const GCONTXT *context, u32int sourceRegister) __pure__;

/* function to load a register value from any register, evaluates modes. */
u32int getGPRegister(GCONTXT *context, u32int sourceRegister);

/* functions to get raw instruction pointer and PC value identical to native execution of guest */
__macro__ u32int getNativeInstructionPointer(const GCONTXT *context) __pure__;
__macro__ u32int getNativeProgramCounter(const GCONTXT *context) __pure__;
__macro__ u32int getVirtualInstructionPointer(const GCONTXT *context) __pure__;
__macro__ u32int getVirtualProgramCounter(const GCONTXT *context) __pure__;

void invalidDataProcTrap(GCONTXT *context, u32int instruction, const char *message)
  __attribute__((noinline,noreturn));

// rotate right function
u32int rorVal(u32int value, u32int ramt)  __constant__;

/* function to store a register value in r0--r7,r15 */
__macro__ void setLowGPRegister(GCONTXT *context, u32int destinationRegister, u32int value);

/* function to store a register value, evaluates modes. */
void setGPRegister(GCONTXT *context, u32int destinationRegister, u32int value);

// generic any type shift function, changes input_parameter(carryFlag) value
u32int shiftVal(u32int imm32, u8int shiftType, u32int shamt, u8int *carryFlag);

#ifdef CONFIG_GUEST_TEST
// function to evaluate breakpoint value in unittests
void evalBkptVal(GCONTXT *context, u32int value);
#endif


__macro__ u32int getLowGPRegister(const GCONTXT *context, u32int sourceRegister)
{
  return *(&context->R0 + sourceRegister);
}

__macro__ u32int getNativeInstructionPointer(const GCONTXT *context)
{
#ifdef CONFIG_BLOCK_COPY
  return context->PCOfLastInstruction;
#else
  return context->R15;
#endif
}

__macro__ u32int getNativeProgramCounter(const GCONTXT *context)
{
#ifdef CONFIG_THUMB2
    const u32int offset = 2 * ARM_INSTRUCTION_SIZE;
#else
    const u32int offset = 2 * (context->CPSR & PSR_T_BIT ? T16_INSTRUCTION_SIZE
                                                         : ARM_INSTRUCTION_SIZE);
#endif
    return getNativeInstructionPointer(context) + offset;
}

__macro__ u32int getVirtualInstructionPointer(const GCONTXT *context)
{
  return context->R15;
}

__macro__ u32int getVirtualProgramCounter(const GCONTXT *context)
{
#ifdef CONFIG_THUMB2
    const u32int offset = 2 * ARM_INSTRUCTION_SIZE;
#else
    const u32int offset = 2 * (context->CPSR & PSR_T_BIT ? T16_INSTRUCTION_SIZE
                                                         : ARM_INSTRUCTION_SIZE);
#endif
    return getVirtualInstructionPointer(context) + offset;
}

__macro__ void setLowGPRegister(GCONTXT *context, u32int destinationRegister, u32int value)
{
  *(&context->R0 + destinationRegister) = value;
}

#endif /* __INSTRUCTION_EMU__INTERPRETER__INTERNALS_H__ */
