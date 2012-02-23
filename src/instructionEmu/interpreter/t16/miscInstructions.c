#include "instructionEmu/interpreter/internals.h"

#include "instructionEmu/interpreter/t16/miscInstructions.h"


u32int t16ItInstruction(GCONTXT *context, u32int instruction)
{
  DIE_NOW(context, "no support for ITSTATE (CONFIG_THUMB_IGNORE_IT not set)");
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
