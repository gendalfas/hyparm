#include "common/bit.h"

#include "instructionEmu/interpreter/internals.h"

#include "instructionEmu/interpreter/t16/miscInstructions.h"


u32int t16ItInstruction(GCONTXT *context, u32int instruction)
{
  // Get ITSTATE from instruction
  u32int itState = instruction & 0xFF;
  u32int offset = 0;
  u16int* end = (u16int *) context->R15;
  end++;

  /*
   * ITAdvance()
   * Instructions that will not be executed are immediately skipped.
   */
  while (!evaluateConditionCode(context, itState >> 4))
  {
    if ((itState & 0x7) == 0)
    {
      itState = 0;
    }
    else
    {
      itState = (itState & 0xE0) | ((itState << 1) & 0x1F);
    }

    switch (*end & THUMB32)
    {
      case THUMB32_1:
      case THUMB32_2:
      case THUMB32_3:
        end++;
        break;
    }
    end++;
  }

  // Update ITSTATE in CPSR
  context->CPSR = (context->CPSR & ~(PSR_ITSTATE_7_2 | PSR_ITSTATE_1_0));
  context->CPSR = context->CPSR | maskedBitShift(itState, PSR_ITSTATE_7_2) | maskedBitShift(itState, PSR_ITSTATE_1_0);

  return (u32int) end;
}

u32int t16UxtbInstruction(GCONTXT *context, u32int instruction)
{
  DIE_NOW(context, "not implemented");
}

u32int t16UxthInstruction(GCONTXT *context, u32int instruction)
{
  DIE_NOW(context, "not implemented");
}

u32int t16BkptInstruction(GCONTXT *context, u32int instruction)
{
#ifdef CONFIG_GUEST_TEST
  u32int val = instruction & 0x00FF;

  evalBkptVal(context, val);
#else
  DIE_NOW(context, "not implemented");
#endif
}
