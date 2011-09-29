#include "common/bit.h"
#include "common/debug.h"

#include "cpuArch/constants.h"

#include "instructionEmu/commonInstrFunctions.h"

#include "instructionEmu/interpreter/loadInstructions.h"


u32int armLdrInstruction(GCONTXT *context, u32int instruction)
{
  if (!evaluateConditionCode(context, ARM_EXTRACT_CONDITION_CODE(instruction)))
  {
    return context->R15 + ARM_INSTRUCTION_SIZE;
  }

  u32int regOrImm = instruction & 0x02000000; // 1 = reg, 0 = imm
  u32int preOrPost = instruction & 0x01000000; // 1 = pre, 0 = post
  u32int incOrDec = instruction & 0x00800000; // 1 = inc, 0 = dec
  u32int writeBack = instruction & 0x00200000; // 1 = writeBack indexing, 0 = no writeback
  u32int regSrc = (instruction & 0x000F0000) >> 16; // Base Load address
  u32int regDst = (instruction & 0x0000F000) >> 12; // Destination - load to this
  u32int offsetAddress = 0;
  u32int baseAddress = 0;

  if (!regOrImm)
  {
    // immediate case
    u32int imm32 = instruction & 0x00000FFF;
    baseAddress = loadGuestGPR(regSrc, context);
    if (regSrc == GPR_PC)
    {
      baseAddress += 8;
    }

    // offsetAddress = if increment then base + imm32 else base - imm32
    if (incOrDec)
    {
      offsetAddress = baseAddress + imm32;
    }
    else
    {
      offsetAddress = baseAddress - imm32;
    }
  } // Immediate case ends
  else
  {
    // register case
    u32int regSrc2 = instruction & 0x0000000F;
    baseAddress = loadGuestGPR(regSrc, context);
    if (regSrc == GPR_PC)
    {
      baseAddress += 8;
    }
    // regSrc2 == PC then UNPREDICTABLE
    if (regSrc2 == GPR_PC)
    {
      DIE_NOW(0, "LDR reg Rm == PC UNPREDICTABLE case!");
    }
    u32int offsetRegisterValue = loadGuestGPR(regSrc2, context);

    // (shift_t, shift_n) = DecodeImmShift(type, imm5)
    u32int shiftAmount = 0;
    u32int shiftType = decodeShiftImmediate(((instruction & 0x060) >> 5),
    ((instruction & 0xF80)>>7), &shiftAmount);
    u8int carryFlag = (context->CPSR & 0x20000000) >> 29;

    // offset = Shift(offsetRegisterValue, shiftType, shitAmount, cFlag);
    u32int offset = shiftVal(offsetRegisterValue, shiftType, shiftAmount, &carryFlag);

    // if increment then base + offset else base - offset
    if (incOrDec)
    {
      // increment
      offsetAddress = baseAddress + offset;
    }
    else
    {
      // decrement
      offsetAddress = baseAddress - offset;
    }
  } // Register case ends

  u32int address = 0;
  // if preIndex then use offsetAddress else baseAddress
  if (preOrPost)
  {
    address = offsetAddress;
  }
  else
  {
    address = baseAddress;
  }

  if ((address & 0x3) != 0x0)
  {
    DIE_NOW(context, "LDR Rd [Rn, Rm/#imm] unaligned address!");
  }

  // P = 0 and W == 1 then LDR as if user mode
  if ((preOrPost == 0) && (writeBack != 0))
  {
    bool abort = shouldDataAbort(FALSE, FALSE, address);
    if (abort)
    {
      return context->R15;
    }
  }

  // DO the actual load from memory
  u32int valueLoaded = context->hardwareLibrary->loadFunction(context->hardwareLibrary, WORD, address);

  // LDR loading to PC should load a word-aligned value
  if ((regDst == 15) && ((valueLoaded & 0x3) != 0))
  {
    printf("LDR: regDst = %x, load from addr %#.8x" EOL, regDst, valueLoaded);
    DIE_NOW(context, "LDR Rd [Rn, Rm/#imm] load unaligned value to PC!");
  }
  // put loaded val to reg
  storeGuestGPR(regDst, valueLoaded, context);

  // wback = (P = 0) or (W = 1)
  bool wback = !preOrPost || writeBack;
  if (wback)
  {
    // if Rn == Rt then UNPREDICTABLE
    if (regDst == regSrc)
    {
      DIE_NOW(0, "LDR writeback UNPREDICTABLE case!");
    }
    // Rn = offsetAddr;
    storeGuestGPR(regSrc, offsetAddress, context);
  }
  if (regDst == GPR_PC)
  {
    return context->R15;
  }
  else
  {
    return context->R15 + ARM_INSTRUCTION_SIZE;
  }
}

u32int armLdrbInstruction(GCONTXT *context, u32int instruction)
{
  if (!evaluateConditionCode(context, ARM_EXTRACT_CONDITION_CODE(instruction)))
  {
    return context->R15 + ARM_INSTRUCTION_SIZE;
  }

  u32int offset = 0;
  u32int offsetAddress = 0;

  u32int condcode = (instruction & 0xF0000000) >> 28;
  u32int regOrImm = instruction & 0x02000000; // 1 = reg, 0 = imm
  u32int preOrPost = instruction & 0x01000000; // 1 = pre, 0 = post
  u32int incOrDec = instruction & 0x00800000; // 1 = inc, 0 = dec
  u32int writeBack = instruction & 0x00200000; // 1 = writeBack indexing, 0 = no writeback
  u32int regSrc = (instruction & 0x000F0000) >> 16; // Base Load address
  u32int regDst = (instruction & 0x0000F000) >> 12; // Destination - load to this

  u32int baseAddress = loadGuestGPR(regSrc, context);

  if (regDst == GPR_PC)
  {
    DIE_NOW(0, "LDRB: cannot load a single byte into PC!");
  }

  if (!regOrImm)
  {
    if (regSrc == GPR_PC)
    {
      DIE_NOW(0, "check LDRB literal");
    }
    // immediate case
    offset = instruction & 0x00000FFF;

  } // Immediate case ends
  else
  {
    // register case
    u32int regSrc2 = instruction & 0x0000000F;
    if (regSrc2 == 15)
    {
      DIE_NOW(0, "LDRB reg Rm == PC UNPREDICTABLE case!");
    }
    u32int offsetRegisterValue = loadGuestGPR(regSrc2, context);

    // (shift_t, shift_n) = DecodeImmShift(type, imm5)
    u32int shiftAmount = 0;
    u32int shiftType = decodeShiftImmediate(((instruction & 0x060) >> 5),
    ((instruction & 0xF80)>>7), &shiftAmount);
    u8int carryFlag = (context->CPSR & 0x20000000) >> 29;

    // offset = Shift(offsetRegisterValue, shiftType, shitAmount, cFlag);
    offset = shiftVal(offsetRegisterValue, shiftType, shiftAmount, &carryFlag);

  } // Register case ends

  // if increment then base + offset else base - offset
  if (incOrDec)
  {
    // increment
    offsetAddress = baseAddress + offset;
  }
  else
  {
    // decrement
    offsetAddress = baseAddress - offset;
  }

  u32int address = 0;
  // if preIndex then use offsetAddress else baseAddress
  if (preOrPost)
  {
    address = offsetAddress;
  }
  else
  {
    address = baseAddress;
  }

  // P = 0 and W == 1 then LDRB as if user mode
  if (!preOrPost && writeBack && shouldDataAbort(FALSE, FALSE, address))
  {
    return context->R15;
  }

  // DO the actual load from memory
  u32int valueLoaded = context->hardwareLibrary->loadFunction(context->hardwareLibrary, BYTE, address) & 0xFF;

  // put loaded val to reg
  storeGuestGPR(regDst, valueLoaded, context);

  // wback = (P = 0) or (W = 1)
  bool wback = !preOrPost || writeBack;
  if (wback)
  {
    // if Rn == Rt then UNPREDICTABLE
    if (regDst == regSrc)
    {
      DIE_NOW(0, "LDRB writeback UNPREDICTABLE case!");
    }
    // Rn = offsetAddr;
    storeGuestGPR(regSrc, offsetAddress, context);
  }
  return context->R15 + ARM_INSTRUCTION_SIZE;
}

u32int armLdrhInstruction(GCONTXT *context, u32int instruction)
{
  if (!evaluateConditionCode(context, ARM_EXTRACT_CONDITION_CODE(instruction)))
  {
    return context->R15 + ARM_INSTRUCTION_SIZE;
  }

  u32int preOrPost = instruction & 0x01000000; // 1 = pre, 0 = post
  u32int incOrDec = instruction & 0x00800000; // 1 = inc, 0 = dec
  u32int regOrImm = instruction & 0x00400000; // 1 = reg, 0 = imm
  u32int writeBack = instruction & 0x00200000; // 1 = writeBack indexing, 0 = no writeback
  u32int regSrc = (instruction & 0x000F0000) >> 16; // Source value from this register...
  u32int regDst = (instruction & 0x0000F000) >> 12; // Destination address

  // P = 0 and W == 1 then LDRHT (as if user mode)
  if (!preOrPost && writeBack)
  {
    DIE_NOW(0, "LDRH as user mode unimplemented.");
  }
  if (regDst == GPR_PC)
  {
    // cannot load halfword into PC!!
    DIE_NOW(0, "LDRH Rd=PC UNPREDICTABLE case.");
  }

  u32int baseAddress = loadGuestGPR(regSrc, context);
  ;
  u32int offsetAddress;
  u32int address;

  if (regOrImm != 0)
  {
    // immediate case
    u32int imm4Top = instruction & 0x00000F00;
    u32int imm4Bottom = instruction & 0x0000000F;
    u32int imm32 = (imm4Top >> 4) | imm4Bottom; // imm field to +/- offset

    // offsetAddress = if increment then base + imm32 else base - imm32
    if (incOrDec != 0)
    {
      offsetAddress = baseAddress + imm32;
    }
    else
    {
      offsetAddress = baseAddress - imm32;
    }

    // if preIndex then use offsetAddress else baseAddress
    if (preOrPost != 0)
    {
      address = offsetAddress;
    }
    else
    {
      address = baseAddress;
    }
    if ((address & 0x1) == 0x1)
    {
      DIE_NOW(0, "LDRH: load address unaligned.");
    }
  } // immediate case done
  else
  {
    // register case
    u32int regSrc2 = instruction & 0x0000000F;
    if (regSrc2 == 15)
    {
      DIE_NOW(0, "LDRH reg Rm == PC UNPREDICTABLE case!");
    }
    u32int offsetRegisterValue = loadGuestGPR(regSrc2, context);

    // (shift_t, shift_n) = (SRType_LSL, 0);
    u32int shiftAmount = 0;
    u32int shiftType = decodeShiftImmediate(0, 0, &shiftAmount);
    u8int carryFlag = (context->CPSR & 0x20000000) >> 29;

    // offset = Shift(offsetRegisterValue, shiftType, shitAmount, cFlag);
    u32int offset = shiftVal(offsetRegisterValue, shiftType, shiftAmount, &carryFlag);

    // if increment then base + offset else base - offset
    if (incOrDec != 0)
    {
      // increment
      offsetAddress = baseAddress + offset;
    }
    else
    {
      // decrement
      offsetAddress = baseAddress - offset;
    }

    // if preIndex then use offsetAddress else baseAddress
    if (preOrPost != 0)
    {
      address = offsetAddress;
    }
    else
    {
      address = baseAddress;
    }
    if ((address & 0x1) == 0x1)
    {
      DIE_NOW(0, "LDRH: load address unaligned.");
    }
  } // reg case done

  u32int valueLoaded = context->hardwareLibrary->loadFunction(context->hardwareLibrary, HALFWORD, address);

  // put loaded val to reg
  storeGuestGPR(regDst, valueLoaded, context);

#ifdef DATA_MOVE_TRACE
  printf("ldrhInstr: %#.8x @ PC = %#.8x R[%x]=%x" EOL, instr, context->R15, regDst, valueLoaded);
#endif

  // wback = (P = 0) or (W = 1)
  bool wback = (preOrPost == 0) || (writeBack != 0);
  if (wback)
  {
    //if Rn == PC || Rn == Rt || Rn == Rm) then UNPREDICTABLE;
    if (regDst == regSrc)
    {
      DIE_NOW(0, "LDRH writeback UNPREDICTABLE case!");
    }
    // Rn = offsetAddr;
    storeGuestGPR(regSrc, offsetAddress, context);
  }

  return context->R15 + ARM_INSTRUCTION_SIZE;
}

u32int armLdrdInstruction(GCONTXT *context, u32int instruction)
{
  if (!evaluateConditionCode(context, ARM_EXTRACT_CONDITION_CODE(instruction)))
  {
    return context->R15 + ARM_INSTRUCTION_SIZE;
  }

  u32int prePost = instruction & 0x01000000;
  u32int upDown = instruction & 0x00800000;
  u32int regOrImm = instruction & 0x00400000; // 0 = reg, 1 = imm
  u32int writeback = instruction & 0x00200000;

  u32int regSrc = (instruction & 0x000F0000) >> 16;
  u32int regDst = (instruction & 0x0000F000) >> 12;
  u32int regDst2 = regDst + 1;

#ifdef DATA_MOVE_TRACE
  printf("LDRD instruction: %#.8x @ PC = %#.8x" EOL, instruction, context->R15);
#endif

  if ((regDst % 2) == 1)
  {
    DIE_NOW(0, "LDRD undefined case: regDst must be even number!");
  }

  u32int offsetAddress = 0;
  u32int baseAddress = loadGuestGPR(regSrc, context);

  u32int wback = (prePost == 0) || (writeback != 0);

  // P = 0 and W == 1 then STR as if user mode
  if ((prePost == 0) && (writeback != 0))
  {
    DIE_NOW(context, "LDRD unpredictable case (P=0 AND W=1)!");
  }

  if (wback && ((regDst == 15) || (regSrc == regDst) || (regSrc == regDst2)))
  {
    DIE_NOW(context, "LDRD unpredictable register selection!");
  }
  if (regDst2 == 15)
  {
    DIE_NOW(context, "LDRD: unpredictable case, regDst2 = PC!");
  }

  if (regOrImm != 0)
  {
    // immediate case
    u32int imm4h = (instruction & 0x00000f00) >> 4;
    u32int imm4l = (instruction & 0x0000000f);
    u32int imm32 = imm4h | imm4l;

    // offsetAddress = if increment then base + imm32 else base - imm32
    if (upDown != 0)
    {
      offsetAddress = baseAddress + imm32;
    }
    else
    {
      offsetAddress = baseAddress - imm32;
    }
#ifdef DATA_MOVE_TRACE
    printf("imm32=%x baseAddress=%#.8x offsetAddres=%#.8x" EOL, imm32, baseAddress, offsetAddress);
#endif
  } // Immediate case ends
  else
  {
    // register case
    u32int regSrc2 = instruction & 0x0000000F;
    u32int offsetRegisterValue = loadGuestGPR(regSrc2, context);
    // regDest2 == PC then UNPREDICTABLE
    if (regSrc2 == 15)
    {
      DIE_NOW(0, "STR reg Rm == PC UNPREDICTABLE case!");
    }

    // if increment then base + offset else base - offset
    if (upDown != 0)
    {
      // increment
      offsetAddress = baseAddress + offsetRegisterValue;
    }
    else
    {
      // decrement
      offsetAddress = baseAddress - offsetRegisterValue;
    }
#ifdef DATA_MOVE_TRACE
    printf("Rm=%x baseAddress=%x offsetRegVal=%x" EOL, regSrc2, baseAddress, offsetRegisterValue);
#endif
  } // Register case ends

  u32int address = 0;
  // if preIndex then use offsetAddress else baseAddress
  if (prePost != 0)
  {
    address = offsetAddress;
  }
  else
  {
    address = baseAddress;
  }
#ifdef DATA_MOVE_TRACE
  printf("LDRD: load address = %x" EOL, address);
#endif

  u32int valueLoaded = context->hardwareLibrary->loadFunction(context->hardwareLibrary, WORD, address);
  u32int valueLoaded2 = context->hardwareLibrary->loadFunction(context->hardwareLibrary, WORD, address + 4);
  // put loaded values to their registers
  storeGuestGPR(regDst, valueLoaded, context);
  storeGuestGPR(regDst2, valueLoaded2, context);

#ifdef DATA_MOVE_TRACE
  printf("LDRD: valueLoaded1 = %x valueLoaded2 = %x" EOL, valueLoaded, valueLoaded2);
#endif

  if (wback)
  {
    // Rn = offsetAddr;
    storeGuestGPR(regSrc, offsetAddress, context);
  }
  return context->R15 + ARM_INSTRUCTION_SIZE;
}

u32int armLdrhtInstruction(GCONTXT *context, u32int instruction)
{
  if (!evaluateConditionCode(context, ARM_EXTRACT_CONDITION_CODE(instruction)))
  {
    return context->R15 + ARM_INSTRUCTION_SIZE;
  }

  DIE_NOW(context, "armLdrhtInstruction not implemented");
}

u32int armLdrexInstruction(GCONTXT *context, u32int instruction)
{
  if (!evaluateConditionCode(context, ARM_EXTRACT_CONDITION_CODE(instruction)))
  {
    return context->R15 + ARM_INSTRUCTION_SIZE;
  }

#ifdef DATA_MOVE_TRACE
  printf("LDREX instruction: %#.8x @ PC = %#.8x" EOL, instruction, context->R15);
#endif

  u32int baseReg = (instruction & 0x000F0000) >> 16;
  u32int regDest = (instruction & 0x0000F000) >> 12;

  if (baseReg == GPR_PC || regDest == GPR_PC)
  {
    DIE_NOW(context, "LDREX unpredictable case (PC used).");
  }

  u32int baseVal = loadGuestGPR(baseReg, context);
  u32int value = context->hardwareLibrary->loadFunction(context->hardwareLibrary, WORD, baseVal);

#ifdef DATA_MOVE_TRACE
  printf("LDREX instruction: baseVal = %#.8x loaded %x, store to %x" EOL, baseVal, value, regDest);
#endif

  storeGuestGPR(regDest, value, context);

  return context->R15 + ARM_INSTRUCTION_SIZE;
}

u32int armLdrexbInstruction(GCONTXT *context, u32int instruction)
{
  if (!evaluateConditionCode(context, ARM_EXTRACT_CONDITION_CODE(instruction)))
  {
    return context->R15 + ARM_INSTRUCTION_SIZE;
  }

#ifdef DATA_MOVE_TRACE
  printf("LDREXB instruction: %08 @ PC = %#.8x" EOL, instruction, context->R15);
#endif

  u32int baseReg = (instruction & 0x000F0000) >> 16;
  u32int regDest = (instruction & 0x0000F000) >> 12;

  if (baseReg == GPR_PC || regDest == GPR_PC)
  {
    DIE_NOW(context, "LDREXB unpredictable case (PC used).");
  }

  u32int baseVal = loadGuestGPR(baseReg, context);
  // byte zero extended to word...
  u32int value = ((u32int) context->hardwareLibrary->loadFunction(context->hardwareLibrary, BYTE, baseVal) & 0xFF);
  storeGuestGPR(regDest, value, context);

  return context->R15 + ARM_INSTRUCTION_SIZE;
}

u32int armLdrexhInstruction(GCONTXT *context, u32int instruction)
{
  if (!evaluateConditionCode(context, ARM_EXTRACT_CONDITION_CODE(instruction)))
  {
    return context->R15 + ARM_INSTRUCTION_SIZE;
  }

#ifdef DATA_MOVE_TRACE
  printf("LDREXH instruction: %#.8x @ PC = %#.8x" EOL, instruction, context->R15);
#endif

  u32int baseReg = (instruction & 0x000F0000) >> 16;
  u32int regDest = (instruction & 0x0000F000) >> 12;

  if (baseReg == GPR_PC || regDest == GPR_PC)
  {
    DIE_NOW(0, "LDREXH unpredictable case (PC used).");
  }
  u32int baseVal = loadGuestGPR(baseReg, context);
  // halfword zero extended to word...
  u32int value = ((u32int) context->hardwareLibrary->loadFunction(context->hardwareLibrary, HALFWORD, baseVal) & 0xFFFF);
  storeGuestGPR(regDest, value, context);

  return context->R15 + ARM_INSTRUCTION_SIZE;
}

u32int armLdrexdInstruction(GCONTXT *context, u32int instruction)
{
  if (!evaluateConditionCode(context, ARM_EXTRACT_CONDITION_CODE(instruction)))
  {
    return context->R15 + ARM_INSTRUCTION_SIZE;
  }

#ifdef DATA_MOVE_TRACE
  printf("LDREXD instruction: %#.8x @ PC = %#.8x" EOL, instr, context->R15);
#endif

  u32int baseReg = (instruction & 0x000F0000) >> 16;
  u32int regDest = (instruction & 0x0000F000) >> 12;

  // must not be PC, destination must be even and not link register
  if ((baseReg == GPR_PC) || ((regDest % 2) != 0) || (regDest == GPR_LR))
  {
    DIE_NOW(0, "LDREXH unpredictable case (invalid registers).");
  }
  u32int baseVal = loadGuestGPR(baseReg, context);

  u32int value = context->hardwareLibrary->loadFunction(context->hardwareLibrary, WORD, baseVal);
  u32int value2 = context->hardwareLibrary->loadFunction(context->hardwareLibrary, WORD, baseVal + 4);
  storeGuestGPR(regDest, value, context);
  storeGuestGPR(regDest + 1, value2, context);

  return context->R15 + ARM_INSTRUCTION_SIZE;
}

u32int armLdmInstruction(GCONTXT *context, u32int instruction)
{
  if (!evaluateConditionCode(context, ARM_EXTRACT_CONDITION_CODE(instruction)))
  {
    return context->R15 + ARM_INSTRUCTION_SIZE;
  }

#ifdef DATA_MOVE_TRACE
  printf("LDM instruction: %#.8x @ PC = %#.8x" EOL, instruction, context->R15);
#endif

  u32int prePost = instruction & 0x01000000;
  u32int upDown = instruction & 0x00800000;
  u32int forceUser = instruction & 0x00400000;
  u32int writeback = instruction & 0x00200000;
  u32int baseReg = (instruction & 0x000F0000) >> 16;
  u32int regList = instruction & 0x0000FFFF;
  u32int baseAddress = loadGuestGPR(baseReg, context);

  if ((baseReg == GPR_PC) || !regList)
  {
    DIE_NOW(0, "LDM UNPREDICTABLE: base=PC or no registers in list");
  }

  u32int savedCPSR = 0;
  bool cpySpsr = FALSE;
  if (forceUser != 0)
  {
    // ok, is this exception return, or LDM user mode?
    if ((instruction & 0x00008000) != 0)
    {
      // force user bit set and PC in list: exception return
      cpySpsr = TRUE;
    }
    else
    {
      // force user bit set and no PC in list: LDM user mode registers
      savedCPSR = context->CPSR;
      context->CPSR = (context->CPSR & ~0x1f) | PSR_USR_MODE;
    }
  }

  u32int address = 0;
  if ((upDown == 0) && (prePost != 0)) // LDM decrement before
  {
    // address = baseAddress - 4*(number of registers to load);
    address = baseAddress - 4 * countBitsSet(regList);
  }
  else if ((upDown == 0) && (prePost == 0)) // LDM decrement after
  {
    // address = baseAddress - 4*(number of registers to load) + 4;
    address = baseAddress - 4 * countBitsSet(regList) + 4;
  }
  else if ((upDown != 0) && (prePost != 0)) // LDM increment before
  {
    // address = baseAddress + 4 - will be incremented as we go
    address = baseAddress + 4;
  }
  else if ((upDown != 0) && (prePost == 0)) // LDM increment after
  {
    // address = baseAddress - will be incremented as we go
    address = baseAddress;
  }

  bool isPCinRegList = FALSE;
  int i;
  for (i = 0; i < 16; i++)
  {
    // if current register set
    if (((regList >> i) & 0x1) == 0x1)
    {
      if (i == 15)
      {
        isPCinRegList = TRUE;
      }
      // R[i] = *(address);
      u32int valueLoaded = context->hardwareLibrary->loadFunction(context->hardwareLibrary, WORD, address);
      storeGuestGPR(i, valueLoaded, context);
#ifdef DATA_MOVE_TRACE
      printf("R[%x] = *(%#.8x) = %#.8x" EOL, i, address, valueLoaded);
#endif
      address = address + 4;
    }
  } // for ends

  // if writeback then baseReg = baseReg +/- 4 * number of registers to load;
  if (writeback != 0)
  {
    if (upDown == 0)
    {
      // decrement
      baseAddress = baseAddress - 4 * countBitsSet(regList);
    }
    else
    {
      // increment
      baseAddress = baseAddress + 4 * countBitsSet(regList);
    }
    storeGuestGPR(baseReg, baseAddress, context);
  }

  if (forceUser != 0)
  {
    // do we need to copy spsr? or return from userland?
    if (cpySpsr)
    {
      // ok, exception return option: restore SPSR to CPSR
      // SPSR! which?... depends what mode we are in...
      u32int modeSpsr = 0;
      switch (context->CPSR & PSR_MODE)
      {
        case PSR_FIQ_MODE:
          modeSpsr = context->SPSR_FIQ;
          break;
        case PSR_IRQ_MODE:
          modeSpsr = context->SPSR_IRQ;
          break;
        case PSR_SVC_MODE:
          modeSpsr = context->SPSR_SVC;
          break;
        case PSR_ABT_MODE:
          modeSpsr = context->SPSR_ABT;
          break;
        case PSR_UND_MODE:
          modeSpsr = context->SPSR_UND;
          break;
        default:
          DIE_NOW(0, "LDM: exception return form sys/usr mode!");
      }
      if ((modeSpsr & PSR_MODE) == PSR_USR_MODE)
      {
        DIE_NOW(context, "LDM: exception return to user mode!");
      }
      context->CPSR = modeSpsr;
    }
    else
    {
      context->CPSR = savedCPSR;
    }
  }

  if (isPCinRegList)
  {
    return context->R15;
  }
  else
  {
    return context->R15 + ARM_INSTRUCTION_SIZE;
  }
}
