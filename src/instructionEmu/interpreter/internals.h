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


#define ARM_EXTRACT_REGISTER(instructionWord, position)     ((instructionWord >> position) & 0xF)
#define ARM_SET_REGISTER(instructionWord, position, value)                                         \
  ((instructionWord & ~(0xF << position)) | (value << position))

#define T16_EXTRACT_LOW_REGISTER(instructionWord, position) ((instructionWord >> position) & 0x7)

/* expand immediate12 field of instruction */
u32int armExpandImm12(u32int imm12) __constant__; // STARFIX: this will get removed

// take the imm5 shift amount and shift type field from instr
// returns shift type, and adjusts shift amount
u32int decodeShiftImmediate(u32int instrShiftType, u32int imm5, u32int *shamt);


// function to evaluate breakpoint value in unittests
void evaluateBreakpointValue(GCONTXT *context, u32int value);


/* a function to evaluate if a condition value is satisfied */
bool evaluateConditionCode(GCONTXT *context, ConditionCode conditionCode);


/* function to load a register value from r0--r7 */
__macro__ u32int getLowGPRegister(const GCONTXT *context, u32int sourceRegister) __pure__;


/* function to load a register value from any register, evaluates modes. */
u32int getGPRegister(GCONTXT *context, u32int sourceRegister);


void invalidDataProcTrap(GCONTXT *context, u32int instruction, const char *message)
  __attribute__((noinline,noreturn));

/* functions to get raw instruction pointer and PC value identical to native execution of guest */
__macro__ u32int PC(const GCONTXT *context) __pure__;

/* function to store a register value in r0--r7,r15 */
__macro__ void setLowGPRegister(GCONTXT *context, u32int destinationRegister, u32int value);


/* function to store a register value, evaluates modes. */
void setGPRegister(GCONTXT *context, u32int destinationRegister, u32int value);


// generic any type shift function
u32int shiftVal(u32int imm32, u8int shiftType, u32int shamt);


/********************** inlines ***********************************************/
__macro__ u32int getLowGPRegister(const GCONTXT *context, u32int sourceRegister)
{
  return *(&context->R0 + sourceRegister);
}


__macro__ u32int PC(const GCONTXT *context)
{
#ifndef CONFIG_THUMB2
    return context->R15 + 2 * ARM_INSTRUCTION_SIZE;
#else
    return 2 * (context->CPSR.bits.T ? T16_INSTRUCTION_SIZE : ARM_INSTRUCTION_SIZE);
#endif
}

__macro__ void setLowGPRegister(GCONTXT *context, u32int destinationRegister, u32int value)
{
  *(&context->R0 + destinationRegister) = value;
}


#endif /* __INSTRUCTION_EMU__INTERPRETER__INTERNALS_H__ */
