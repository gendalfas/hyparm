#include <cassert>
#include <cstdint>
#include <iostream>
#include <string>
#include <sstream>

// cat tables.inc.c  | grep 'ENTRY(' | cut -d',' -f2 | sed 's/^\s*//' | sed 's/\s*$//' | sort | uniq | sed 's/^\(.\+\)$/const char *const \1 = "\1";/' 
//
const char *const armAdcInstruction = "armAdcInstruction";
const char *const armAddInstruction = "armAddInstruction";
const char *const armAndInstruction = "armAndInstruction";
const char *const armAsrInstruction = "armAsrInstruction";
const char *const armBicInstruction = "armBicInstruction";
const char *const armBInstruction = "armBInstruction";
const char *const armBkptInstruction = "armBkptInstruction";
const char *const armBlxImmediateInstruction = "armBlxImmediateInstruction";
const char *const armBlxRegisterInstruction = "armBlxRegisterInstruction";
const char *const armBxInstruction = "armBxInstruction";
const char *const armBxjInstruction = "armBxjInstruction";
const char *const armClrexInstruction = "armClrexInstruction";
const char *const armClzInstruction = "armClzInstruction";
const char *const armCmnInstruction = "armCmnInstruction";
const char *const armCmpInstruction = "armCmpInstruction";
const char *const armCpsInstruction = "armCpsInstruction";
const char *const armDbgInstruction = "armDbgInstruction";
const char *const armDmbInstruction = "armDmbInstruction";
const char *const armDsbInstruction = "armDsbInstruction";
const char *const armEorInstruction = "armEorInstruction";
const char *const armIsbInstruction = "armIsbInstruction";
const char *const armLdmInstruction = "armLdmInstruction";
const char *const armLdrbInstruction = "armLdrbInstruction";
const char *const armLdrbtInstruction = "armLdrbtInstruction";
const char *const armLdrdInstruction = "armLdrdInstruction";
const char *const armLdrexbInstruction = "armLdrexbInstruction";
const char *const armLdrexdInstruction = "armLdrexdInstruction";
const char *const armLdrexhInstruction = "armLdrexhInstruction";
const char *const armLdrexInstruction = "armLdrexInstruction";
const char *const armLdrhInstruction = "armLdrhInstruction";
const char *const armLdrhtInstruction = "armLdrhtInstruction";
const char *const armLdrInstruction = "armLdrInstruction";
const char *const armLdrtInstruction = "armLdrtInstruction";
const char *const armLslInstruction = "armLslInstruction";
const char *const armLsrInstruction = "armLsrInstruction";
const char *const armMcrInstruction = "armMcrInstruction";
const char *const armMovInstruction = "armMovInstruction";
const char *const armMovtInstruction = "armMovtInstruction";
const char *const armMovwInstruction = "armMovwInstruction";
const char *const armMrcInstruction = "armMrcInstruction";
const char *const armMrsInstruction = "armMrsInstruction";
const char *const armMsrInstruction = "armMsrInstruction";
const char *const armMvnInstruction = "armMvnInstruction";
const char *const armOrrInstruction = "armOrrInstruction";
const char *const armPldInstruction = "armPldInstruction";
const char *const armPliInstruction = "armPliInstruction";
const char *const armRfeInstruction = "armRfeInstruction";
const char *const armRorInstruction = "armRorInstruction";
const char *const armRrxInstruction = "armRrxInstruction";
const char *const armRsbInstruction = "armRsbInstruction";
const char *const armRscInstruction = "armRscInstruction";
const char *const armSbcInstruction = "armSbcInstruction";
const char *const armSetendInstruction = "armSetendInstruction";
const char *const armSevInstruction = "armSevInstruction";
const char *const armSmcInstruction = "armSmcInstruction";
const char *const armSrsInstruction = "armSrsInstruction";
const char *const armStmInstruction = "armStmInstruction";
const char *const armStrbInstruction = "armStrbInstruction";
const char *const armStrbtInstruction = "armStrbtInstruction";
const char *const armStrdInstruction = "armStrdInstruction";
const char *const armStrexbInstruction = "armStrexbInstruction";
const char *const armStrexdInstruction = "armStrexdInstruction";
const char *const armStrexhInstruction = "armStrexhInstruction";
const char *const armStrexInstruction = "armStrexInstruction";
const char *const armStrhInstruction = "armStrhInstruction";
const char *const armStrhtInstruction = "armStrhtInstruction";
const char *const armStrInstruction = "armStrInstruction";
const char *const armStrtInstruction = "armStrtInstruction";
const char *const armSubInstruction = "armSubInstruction";
const char *const armSwpInstruction = "armSwpInstruction";
const char *const armTeqInstruction = "armTeqInstruction";
const char *const armTstInstruction = "armTstInstruction";
const char *const armWfeInstruction = "armWfeInstruction";
const char *const armWfiInstruction = "armWfiInstruction";
const char *const armYieldInstruction = "armYieldInstruction";
const char *const _handler = "_handler";
const char *const nopInstruction = "nopInstruction";
const char *const svcInstruction = "svcInstruction";
const char *const undefinedInstruction = "undefinedInstruction";

// With -f3, non-NULL
const char *const armALUImmRegRSR = "armALUImmRegRSR";
const char *const armALUImmRegRSRNoDest = "armALUImmRegRSRNoDest";
const char *const armLdrdhPCInstruction = "armLdrdhPCInstruction";
const char *const armLdrPCInstruction = "armLdrPCInstruction";
const char *const armMovPCInstruction = "armMovPCInstruction";
const char *const armShiftPCImm = "armShiftPCImm";
const char *const armStmPC = "armStmPC";
const char *const armStrPCInstruction = "armStrPCInstruction";
const char *const armStrtPCInstruction = "armStrtPCInstruction";
const char *const _pcHandler = "_pcHandler";


enum IRC
{
  IRC_UNDEFINED = -1,
  IRC_SAFE = 0,
  IRC_REPLACE = 1,
  IRC_REMOVE = 2,
  IRC_PATCH_PC = 4
};


// TSD
struct decodingTableEntry { IRC code; const char * handler; const char * pcHandler; uint32_t value; uint32_t mask; const char *instructionString; };
struct decodingTable { uint32_t mask; uint32_t value; struct decodingTableEntry *table; };
#include "tables.inc.c"
static struct decodingTableEntry *decodeTSD(uint32_t instruction)
{
  const struct decodingTable *categories = armCategories;
  while ((instruction & categories->mask) != categories->value) ++categories;
  struct decodingTableEntry *entry = categories->table;
  if (!entry) return nullptr;
  while ((instruction & entry->mask) != entry->value) ++entry;
  return entry->mask == 0 ? nullptr : entry;
}


// Autodecoder
struct Handler { struct { void operator =(void *str) { str_= reinterpret_cast<const char *>(str); } std::string str_; } barePtr; };
int decodeAD(uint32_t instruction, Handler *handler)
{
#define printf(...)
#define DIE_NOW(x,y) return IRC_UNDEFINED
#include "graph.inc.c"
#undef DIE_NOW
#undef printf
}

int main()
{
  std::cout << std::hex << std::showbase;
  for (uint64_t instr = 0; instr <= 0xFFFFFFFFu; ++instr)
  {
    std::ostringstream os;

    // Run TSD
    auto tsd = decodeTSD(instr);
    os << " " << (tsd == nullptr ? "UND" : tsd->instructionString)
       << " TSD=" << (tsd == nullptr ? "undefined" : (tsd->code == IRC_SAFE ? "safe" : (tsd->code == IRC_PATCH_PC ? "patch" : (tsd->code == IRC_REMOVE ? "remove" : (tsd->code == IRC_REPLACE ? "replace" : "?")))))
       << "[i=" << (tsd != nullptr && tsd->handler != nullptr ? tsd->handler : "") << ",p=" << (tsd != nullptr && tsd->pcHandler != nullptr ? tsd->pcHandler : "") << "]";
    
    // Run AD
    Handler h;
    int ad = decodeAD(instr, &h);
    os << " AD=" << (ad == IRC_SAFE ? "safe" : (ad == IRC_PATCH_PC ? "patch" : (ad == IRC_REMOVE ? "remove" : (ad == IRC_REPLACE ? "replace" : "?")))) << "[" << h.barePtr.str_ << "]";

    if (tsd == nullptr)
    {
      if (ad != IRC_UNDEFINED)
      {
	std::cout << "Mismatch at " << instr << os.str() << std::endl;
      }
    }
    else if (tsd->code == IRC_SAFE)
    {
      if (ad != IRC_SAFE)
      {
	std::cout << "Mismatch at " << instr << os.str() << std::endl;
      }
    }
    else if (tsd->code == IRC_PATCH_PC)
    {
      if (ad != IRC_PATCH_PC || h.barePtr.str_ != tsd->pcHandler)
      {
	assert(tsd->pcHandler != nullptr);
	std::cout << "Mismatch at " << instr << os.str() << std::endl;
      }
    }
    else if (tsd->code == IRC_REPLACE)
    {
      if (ad != IRC_REPLACE || h.barePtr.str_ != tsd->handler)
      {
	assert(tsd->pcHandler != nullptr);
	std::cout << "Mismatch at " << instr << os.str() << std::endl;
      }
    }
    else if (tsd->code == IRC_REMOVE)
    {
      if (ad != IRC_REMOVE)
      {
	std::cout << "Mismatch at " << instr << os.str() << std::endl;
      }
    }
    else
    {
      std::cout << "Wtf at " << instr << os.str() << std::endl;
    }
  }
}
