#ifndef __INSTRUCTION_EMU__TRANSLATOR_H__
#define __INSTRUCTION_EMU__TRANSLATOR_H__

#include "common/types.h"

#include "guestManager/basicBlockStore.h"
#include "guestManager/guestContext.h"

void translate(GCONTXT* context, BasicBlock* block, DecodedInstruction* decoding, u32int instruction);

void putBranch(u32int branchLocation, u32int branchTarget, u32int condition);

bool isBranch(u32int instruction);
bool isServiceCall(u32int instruction);
bool isUndefinedCall(u32int instruction);

u32int findBlockIndexNumber(GCONTXT *context, u32int hostPC);

u32int hostpcToGuestpc(GCONTXT* context);

#endif