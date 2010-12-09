#include "guestExceptions.h"
#include "guestContext.h"
#include "intc.h"
#include "cpu.h"
#include "debug.h"

extern GCONTXT * getGuestContext(void);


void tickEvent(u32int irqNumber)
{
  GCONTXT * context = getGuestContext();

  if (irqNumber == GPT2_IRQ)
  {
    // adjust guest interrupt controller state
    setInterrupt(GPT1_IRQ);
    // are we forwarding the interrupt event?
    if ((context->CPSR & CPSR_IRQ_DIS) == 0)
    {
      // guest has enabled interrupts globally.
      // set guest irq pending flag!
      context->guestIrqPending = TRUE;
    }
  }
  else
  {
    DIE_NOW(0, "GuestInterrupts: tickEvent from unknown source timer.");
  }
}

void deliverInterrupt(void)
{
  GCONTXT * context = getGuestContext();
  // 1. reset irq pending flag.
  context->guestIrqPending = FALSE;
  // 2. copy guest CPSR into SPSR_IRQ
  context->SPSR_IRQ = context->CPSR;
  // 3. put guest CPSR in IRQ mode
  context->CPSR = (context->CPSR & ~CPSR_MODE) | CPSR_MODE_IRQ;
  // 4. disable further guest interrupts
  context->CPSR |= CPSR_IRQ_DIS;
  // 5. set LR to PC+4
  context->R14_IRQ = context->R15 + 4;
  // 6. set PC to guest irq handler address
  if (context->virtAddrEnabled)
  {
    if (context->guestHighVectorSet)
    {
      context->R15 = 0xffff0018;
    }
    else
    {
      context->R15 = 0x00000018;
    }
  }
  else
  {
    DIE_NOW(0, "deliverInterrupt: IRQ to be delivered with guest vmem off.");
  }
  // now prepared the global guest environment. scanner will scan correct code.
}

void deliverAbort()
{
  GCONTXT * context = getGuestContext();
  // 1. reset abt pending flag
  context->guestAbtPending = FALSE;
  // 2. copy CPSR into SPSR_ABT
  context->SPSR_ABT = context->CPSR;
  // 3. put guest CPSR in ABT mode
  context->CPSR = (context->CPSR & ~CPSR_MODE) | CPSR_MODE_ABT;
  // 4. set LR to PC+8
  context->R14_ABT = context->R15 + 8;
  // 5. set PC to guest irq handler address
  if (context->virtAddrEnabled)
  {
    if (context->guestHighVectorSet)
    {
      context->R15 = 0xffff0010;
    }
    else
    {
      context->R15 = 0x00000010;
    }
  }
  else
  {
    DIE_NOW(0, "deliverInterrupt: IRQ to be delivered with guest vmem off.");
  }
  // now prepared the global guest environment. scanner will scan correct code.
}
