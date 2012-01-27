#ifndef __INSTRUCTION_EMU__DATA_MOVE_INSTR__
#define __INSTRUCTION_EMU__DATA_MOVE_INSTR__

#include "common/types.h"

#include "memoryManager/globalMemoryMapper.h"

#include "guestManager/guestContext.h"


// uncomment me for LOAD/STORE instruction trace: #define DATA_MOVE_TRACE
void invalidDataMoveTrap(const char * msg, GCONTXT * gc);

u32int strInstruction(GCONTXT * context);

u32int strbInstruction(GCONTXT * context);

u32int strhInstruction(GCONTXT * context);

u32int stmInstruction(GCONTXT * context);

u32int strdInstruction(GCONTXT * context);

u32int strexInstruction(GCONTXT * context);

u32int strexbInstruction(GCONTXT * context);

u32int strexdInstruction(GCONTXT * context);

u32int strexhInstruction(GCONTXT * context);

u32int strhtInstruction(GCONTXT * context);

u32int ldrhInstruction(GCONTXT * context);

u32int ldrInstruction(GCONTXT * context);

u32int ldrdInstruction(GCONTXT * context);

u32int ldrbInstruction(GCONTXT * context);

u32int popLdrInstruction(GCONTXT * context);

u32int popLdmInstruction(GCONTXT * context);

u32int ldmInstruction(GCONTXT * context);

u32int ldrexInstruction(GCONTXT * context);

u32int ldrexbInstruction(GCONTXT * context);

u32int ldrexdInstruction(GCONTXT * context);

u32int ldrexhInstruction(GCONTXT * context);

u32int ldrhtInstruction(GCONTXT * context);

#ifdef CONFIG_BLOCK_COPY
u32int* strPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress);
u32int* strbPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress);
u32int* strhPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress);
u32int* stmPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress);
u32int* strdPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress);
u32int* strexPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress);
u32int* strexbPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress);
u32int* strexdPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress);
u32int* strexhPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress);
u32int* strhtPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress);
u32int* ldrhPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress);
u32int* ldrPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress);
u32int* ldrdPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress);
u32int* ldrbPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress);
u32int* popLdrPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress);
u32int* popLdmPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress);
u32int* ldmPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress);
u32int* ldrexPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress);
u32int* ldrexbPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress);
u32int* ldrexdPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress);
u32int* ldrexhPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress);
u32int* ldrhtPCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr, u32int * blockCopyCacheStartAddress);
#endif

#endif
