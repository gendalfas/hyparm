#include "common/bit.h"
#include "common/stddef.h"

#include "cpuArch/constants.h"

#include "instructionEmu/interpreter/internals.h"


enum guestBreakPointValues
{
  BKPT_TEST_PASS = 0,
  BKPT_DUMP_ACTIVE_SPT = 0xFFFF
};


// take shift type field from instr, return shift type
static u32int decodeShift(u32int instrShiftType);

static u32int *getHighGPRegisterPointer(GCONTXT *context, u32int registerIndex);

// rotate right function
static u32int rorVal(u32int value, u32int ramt)  __constant__;


u32int arithLogicOp(GCONTXT *context, u32int instr, OPTYPE opType, const char *instrString)
{
  u32int nextPC = 0;
  u32int regDest = (instr & 0x0000F000) >> 12;

  if (regDest != GPR_PC)
  {
    invalidDataProcTrap(context, instr, instrString);
  }

#ifdef DATA_PROC_TRACE
  printf("%s: %#.8x @ %#.8x" EOL, instrString, instr, getNativeInstructionPointer(context));
#endif

  if (evaluateConditionCode(context, ARM_EXTRACT_CONDITION_CODE(instr)))
  {
    // set-flags case is tricky! depends on guest mode.
    u32int setFlags = (instr & 0x00100000); // S bit on intruction binary respresentation
    // source operand1
    u32int regSrc = (instr & 0x000F0000) >> 16;
    // source operand2 - register or immediate?
    u32int regOrImm = instr & 0x02000000; // 1 = imm, 0 = reg
    if (regOrImm != 0)
    {
      // if S bit is set, this is return from exception!
      if (setFlags != 0)
      {
        DIE_NOW(context, "arithmetic: imm case return from exception case unimplemented.\n");
      }

      // source operand2 immediate: pc = regSrc +/- ror(immediate)
      u32int imm12 = instr & 0x00000FFF;
      switch (opType)
      {
        case ADD:
        {
          nextPC = getGPRegister(context, regSrc) + armExpandImm12(imm12);
          break;
        }
        case SUB:
        {
          nextPC = getGPRegister(context, regSrc) - armExpandImm12(imm12);
          break;
        }
        default:
          DIE_NOW(context, "invalid arithLogicOp opType");
      }
    }
    else
    {
      // register case: pc = regSrc + shift(regSrc2)
      u32int regSrc2   =  instr & 0x0000000F;
      u32int shiftType = (instr & 0x00000060) >> 5;
      u32int shamt = 0;
      ASSERT((instr & 0x00000010) == 0, ERROR_UNPREDICTABLE_INSTRUCTION)
      // shift amount is an immediate field
      u32int imm5 = (instr & 0xF80) >> 7;
      u8int carryFlag = (context->CPSR & 0x20000000) >> 29;
      shiftType = decodeShiftImmediate(shiftType, imm5, &shamt);
      switch (opType)
      {
        case ADD:
        {
          // if S bit is set, this is return from exception!
          if (setFlags != 0)
          {
            DIE_NOW(context, "arithmetic: reg case return from exception case unimplemented.\n");
          }
          nextPC = getGPRegister(context, regSrc) + shiftVal(getGPRegister(context, regSrc2), shiftType, shamt, &carryFlag);
          break;
        }
        case SUB:
        {
          // if S bit is set, this is return from exception!
          if (setFlags != 0)
          {
            DIE_NOW(context, "arithmetic: reg case return from exception case unimplemented.\n");
          }
          nextPC = getGPRegister(context, regSrc) - shiftVal(getGPRegister(context, regSrc2), shiftType, shamt, &carryFlag);
          break;
        }
        case MOV:
        {
          // cant be shifted - mov shifted reg is a pseudo instr
          ASSERT(shamt == 0, ERROR_BAD_ARGUMENTS);
          nextPC = getGPRegister(context, regSrc2);
          break;
        }
        default:
          DIE_NOW(context, "invalid arithLogicOp opType");
      }
    }

    if (setFlags)
    {
      if (regDest == GPR_PC)
      {
        u32int modeToCopy = 0;
        // copy SPSR to CPSR
        switch (context->CPSR & PSR_MODE)
        {
          case PSR_FIQ_MODE:
          {
            modeToCopy = context->SPSR_FIQ;
            break;
          }
          case PSR_IRQ_MODE:
          {
            modeToCopy = context->SPSR_IRQ;
            break;
          }
          case PSR_SVC_MODE:
          {
            modeToCopy = context->SPSR_SVC;
            break;
          }
          case PSR_ABT_MODE:
          {
            modeToCopy = context->SPSR_ABT;
            break;
          }
          case PSR_UND_MODE:
          {
            modeToCopy = context->SPSR_UND;
            break;
          }
          default:
            DIE_NOW(context, "arithLogicOp: no SPSR for current guest mode");
        }
        if ((context->CPSR & PSR_MODE) != (modeToCopy & PSR_MODE))
        {
          guestChangeMode(modeToCopy & PSR_MODE);
        }
        context->CPSR = modeToCopy;

        // Align PC
        if (context->CPSR & PSR_T_BIT)
        {
          nextPC &= ~1;
        }
        else
        {
          nextPC &= ~3;
        }
      }
      else
      {
        // unimplemented setflags case
        DIE_NOW(context, ERROR_NOT_IMPLEMENTED);
      }
    }

    ASSERT((nextPC & 1) == 0, "Interworking branch not allowed");
    return nextPC;
  }
  else
  {
    return getNativeInstructionPointer(context) + ARM_INSTRUCTION_SIZE;
  }
}

/* expand immediate12 field of instruction */
u32int armExpandImm12(u32int imm12)
{
  // <7:0> immediate value, zero extended to 32
  u32int immVal = imm12 & 0xFF;
  // <11:8>*2 is the rotate amount
  u32int shamt = ((imm12 & 0xF00) >> 8) * 2;
  // rotate right!
  return rorVal(immVal, shamt);
}

// take shift type field from instr, return shift type
static u32int decodeShift(u32int instrShiftType)
{
  instrShiftType = instrShiftType & 0x3;
  switch (instrShiftType)
  {
    case 0:
      return SHIFT_TYPE_LSL;
    case 1:
      return SHIFT_TYPE_LSR;
    case 2:
      return SHIFT_TYPE_ASR;
    default:
      return SHIFT_TYPE_ROR;
  } // switch ends
}

// take the imm5 shift amount and shift type field from instr
// returns shift type, and adjusts shift amount
u32int decodeShiftImmediate(u32int instrShiftType, u32int imm5, u32int * shamt)
{
  instrShiftType = instrShiftType & 0x3;
  *shamt = imm5 & 0x1F;
  switch (instrShiftType)
  {
    case SHIFT_TYPE_LSR:
    case SHIFT_TYPE_ASR:
      if (*shamt == 0)
      {
        *shamt = 32;
      }
    case SHIFT_TYPE_LSL:
      return instrShiftType;
    case SHIFT_TYPE_RRX:
      if (*shamt == 0)
      {
        *shamt = 1;
        return SHIFT_TYPE_RRX;
      }
      return SHIFT_TYPE_ROR;
    default:
      DIE_NOW(NULL, "decodeShiftImmediate: voodoo dolls everywhere!");
  }
}

/*
 * This function is used in unit tests. It evaluates the value passed to the BKPT instruction.
 * Current values:
 * 0      pass
 * 0xFFFF print active shadow pagetable
 * other  fail
 */
void evaluateBreakpointValue(GCONTXT *context, u32int value)
{
  switch (value)
  {
    case BKPT_TEST_PASS:
    {
      DIE_NOW(context, "test passed");
    }
    case BKPT_DUMP_ACTIVE_SPT:
    {
      dumpTranslationTable(context->pageTables->shadowActive);
      break;
    }
    default:
    {
      printf("Breakpoint value %#.8x" EOL, value);
      DIE_NOW(context, "test failed");
    }
  }
  return;
}

bool evaluateConditionCode(GCONTXT *context, u32int conditionCode)
{
  switch (conditionCode)
  {
    case CC_EQ:
      return context->CPSR & PSR_CC_FLAG_Z_BIT;
    case CC_NE:
      return !(context->CPSR & PSR_CC_FLAG_Z_BIT);
    case CC_HS:
      return context->CPSR & PSR_CC_FLAG_C_BIT;
    case CC_LO:
      return !(context->CPSR & PSR_CC_FLAG_C_BIT);
    case CC_MI:
      return context->CPSR & PSR_CC_FLAG_N_BIT;
    case CC_PL:
      return !(context->CPSR & PSR_CC_FLAG_N_BIT);
    case CC_VS:
      return context->CPSR & PSR_CC_FLAG_V_BIT;
    case CC_VC:
      return !(context->CPSR & PSR_CC_FLAG_V_BIT);
    case CC_HI:
      return (context->CPSR & PSR_CC_FLAG_C_BIT) && !(context->CPSR & PSR_CC_FLAG_Z_BIT);
    case CC_LS:
      return !(context->CPSR & PSR_CC_FLAG_C_BIT) || (context->CPSR & PSR_CC_FLAG_Z_BIT);
    case CC_GE:
      return testBitsEqual(context->CPSR, PSR_CC_FLAG_N_BIT, PSR_CC_FLAG_V_BIT);
    case CC_LT:
      return testBitsNotEqual(context->CPSR, PSR_CC_FLAG_N_BIT, PSR_CC_FLAG_V_BIT);
    case CC_GT:
      return !(context->CPSR & PSR_CC_FLAG_Z_BIT)
          && testBitsEqual(context->CPSR, PSR_CC_FLAG_N_BIT, PSR_CC_FLAG_V_BIT);
    case CC_LE:
      return (context->CPSR & PSR_CC_FLAG_Z_BIT)
          || testBitsNotEqual(context->CPSR, PSR_CC_FLAG_N_BIT, PSR_CC_FLAG_V_BIT);
    case CC_AL:
      return TRUE;
    default:
      return FALSE;
  }
}

void invalidDataProcTrap(GCONTXT *context, u32int instruction, const char *message)
{
  printf("%#.8x @ %#.8x should not have trapped!" EOL, instruction, getNativeInstructionPointer(context));
  DIE_NOW(context, message);
}

u32int getGPRegister(GCONTXT *context, u32int sourceRegister)
{
  if (sourceRegister < 8)
  {
    return getLowGPRegister(context, sourceRegister);
  }

  if (sourceRegister == GPR_PC)
  {
    return getNativeProgramCounter(context);
  }

  return *(getHighGPRegisterPointer(context, sourceRegister));
}

static u32int *getHighGPRegisterPointer(GCONTXT *context, u32int registerIndex)
{
  const u32int guestMode = context->CPSR & PSR_MODE;
  if (registerIndex <= 12)
  {
    return ((guestMode == PSR_FIQ_MODE ? &context->R8_FIQ : &context->R8) + registerIndex - 8);
  }

  switch (guestMode)
  {
    case PSR_USR_MODE:
    case PSR_SYS_MODE:
      return registerIndex == 13 ? &context->R13_USR : &context->R14_USR;
    case PSR_FIQ_MODE:
      return registerIndex == 13 ? &context->R13_FIQ : &context->R14_FIQ;
    case PSR_IRQ_MODE:
      return registerIndex == 13 ? &context->R13_IRQ : &context->R14_IRQ;
    case PSR_SVC_MODE:
      return registerIndex == 13 ? &context->R13_SVC : &context->R14_SVC;
    case PSR_ABT_MODE:
      return registerIndex == 13 ? &context->R13_ABT : &context->R14_ABT;
    case PSR_UND_MODE:
      return registerIndex == 13 ? &context->R13_UND : &context->R14_UND;
    default:
      DIE_NOW(context, ERROR_NOT_IMPLEMENTED);
  } // switch ends
}

void setGPRegister(GCONTXT *context, u32int destinationRegister, u32int value)
{
  if (destinationRegister < 8 || destinationRegister == GPR_PC)
  {
    setLowGPRegister(context, destinationRegister, value);
  }
  else
  {
    *(getHighGPRegisterPointer(context, destinationRegister)) = value;
  }
}

// rotate right function
u32int rorVal(u32int value, u32int ramt)
{
  u32int retVal = value;
  u32int leftToRotate = ramt;
  while (leftToRotate > 0)
  {
    u32int rightMostBit = retVal & 0x1;
    retVal >>= 1;
    if (rightMostBit == 1)
    {
      retVal |= 0x80000000;
    }
    else
    {
      retVal &= 0x7FFFFFFF;
    }
    leftToRotate--;
  }
  return retVal;
}

// generic any type shift function, changes input_parameter(carryFlag) value
u32int shiftVal(u32int value, u8int shiftType, u32int shamt, u8int * carryFlag)
{
  // RRX can only shift right by 1
  ASSERT((shiftType != SHIFT_TYPE_RRX) || (shamt == 1), "type rrx, but shamt not 1!");

  u32int retVal = 0;
  if (shamt == 0)
  {
    return value;
  }
  else
  {
     switch(shiftType)
     {
       case SHIFT_TYPE_ROR:
         // ror doesnt adjust carry flag!
         retVal = rorVal(value, shamt);
         break;
       case SHIFT_TYPE_LSL:
         retVal = value << shamt;
         break;
       case SHIFT_TYPE_LSR:
         retVal = value >> shamt;
         break;
       case SHIFT_TYPE_ASR:
       case SHIFT_TYPE_RRX:
       default:
        DIE_NOW(NULL, ERROR_NOT_IMPLEMENTED);
     } // switch
  } // else
  return retVal;
}
