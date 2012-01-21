#include "common/debug.h"

#include "guestManager/guestContext.h"

#include "instructionEmu/dataMoveInstr.h"

#include "memoryManager/globalMemoryMapper.h"


extern GCONTXT * getGuestContext(void); //from main.c


/* generic load store instruction emulation  *
 * called when we permission fault on memory *
 * access to a protected area - must emulate */
void emulateLoadStoreGeneric(GCONTXT * context, u32int address)
{
  // get the store instruction
  u32int instr = *((u32int*)context->R15);
  // save the end of block instruction, as we faulted /not/ on EOB
  u32int eobInstrBackup = context->endOfBlockInstr;
#ifdef CONFIG_BLOCK_COPY
  // save the PCOfLastInstruction. The emulationfunctions make use of this value to calculate the next PC so R15 should be stored here temporary
  // but after the abort the PCOfLastInstruction should be back a valid value since the next emulation function will make use of this!
  u32int PCOfLastInstructionBackup = context->PCOfLastInstruction;

  //emulate methods will take PCOfLastInstruction from context, put it there
  context->PCOfLastInstruction = context->R15;
#endif
  // emulate methods will take instr from context, put it there
  context->endOfBlockInstr = instr;
  if ( ((instr & STR_IMM_MASK) == STR_IMM_MASKED) ||
       ((instr & STR_REG_MASK) == STR_REG_MASKED) )
  {
    // storing to a protected area.. adjust block cache if needed
    validateCachePreChange(context->blockCache, address);
    // STR Rd, [Rn, Rm/#imm12]
    strInstruction(context);
  }
  else if ( ((instr & STRB_IMM_MASK) == STRB_IMM_MASKED) ||
            ((instr & STRB_REG_MASK) == STRB_REG_MASKED) )
  {
    // storing to a protected area.. adjust block cache if needed
    validateCachePreChange(context->blockCache, address);
    // STRB Rd, [Rn, Rm/#imm12]
    strbInstruction(context);
  }
  else if ( ((instr & STRH_IMM_MASK) == STRH_IMM_MASKED) ||
            ((instr & STRH_REG_MASK) == STRH_REG_MASKED) )
  {
    // storing to a protected area.. adjust block cache if needed
    validateCachePreChange(context->blockCache, address);
    // STRH Rd, [Rn, Rm/#imm12]
    strhInstruction(context);
  }
  else if ( ((instr & STRD_IMM_MASK) == STRD_IMM_MASKED) ||
            ((instr & STRD_REG_MASK) == STRD_REG_MASKED) )
  {
    // storing to a protected area.. adjust block cache if needed
    validateCachePreChange(context->blockCache, address);
    // STRD Rd, [Rn, Rm/#imm12]
    strdInstruction(context);
  }
  else if ((instr & STM_MASK) == STM_MASKED)
  {
    // more tricky with cache validation! since we do this in the stmInstruction
    // per address (word in memory) depending on the length of {reg list}
    // STM Rn, {reg list}
    stmInstruction(context);
  }
  else if ((instr & LDR_MASK) == LDR_MASKED)
  {
    // loads don't change memory. no need to validate cache
    // LDR Rd, Rn/#imm12
    ldrInstruction(context);
  }
  else if ((instr & LDRB_MASK) == LDRB_MASKED)
  {
    // LDRB Rd, [Rn, Rm/#imm12]
    ldrbInstruction(context);
  }
  else if ( ((instr & LDRH_IMM_MASK) == LDRH_IMM_MASKED) ||
            ((instr & LDRH_REG_MASK) == LDRH_REG_MASKED) )
  {
    // LDRH Rd, [Rn, Rm/#imm12]
    ldrhInstruction(context);
  }
  else if ( ((instr & LDRD_IMM_MASK) == LDRD_IMM_MASKED) ||
            ((instr & LDRD_REG_MASK) == LDRD_REG_MASKED) )
  {
    // LDRD Rd, [Rn, Rm/#imm12]
    ldrdInstruction(context);
  }
  else if ((instr & LDM_MASK) == LDM_MASKED)
  {
    // LDM, Rn, {reg list}
    ldmInstruction(context);
  }
  else if ((instr & LDREX_MASK) == LDREX_MASKED)
  {
    // LDREX Rd, [Rn]
    ldrexInstruction(context);
  }
  else
  {
    DEBUG_STRING("LoadStore @ ");
    DEBUG_INT(context->R15);
    DEBUG_STRING(" instruction ");
    DEBUG_INT(instr);
    DEBUG_NEWLINE(); 
    DIE_NOW(context, "Load/Store generic unimplemented\n");
  } 
  // restore end of block instruction & PCOFLastInstruction
  context->endOfBlockInstr = eobInstrBackup;
#ifdef CONFIG_BLOCK_COPY
  context->PCOfLastInstruction = PCOfLastInstructionBackup;
#endif
}
