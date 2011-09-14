#ifdef CONFIG_CLI
#include "cli/cli.h"
#endif

#include "common/debug.h"
#include "common/memFunctions.h"
#include "common/stdarg.h"
#include "common/string.h"

#include "cpuArch/cpu.h"

#include "drivers/beagle/beIntc.h"
#include "drivers/beagle/beGPTimer.h"
#include "drivers/beagle/beClockMan.h"
#include "drivers/beagle/beUart.h"

#ifdef CONFIG_GUEST_FREERTOS
#include "drivers/beagle/beGPIO.h"
#endif

#ifdef CONFIG_MMC
#include "drivers/beagle/beMMC.h"
#endif

#include "guestManager/guestContext.h"
#include "guestManager/blockCache.h"

#ifdef CONFIG_MMC
#include "io/mmc.h"
#include "io/partitions.h"
#include "io/fs/fat.h"
#endif

#include "linuxBoot/bootLinux.h"
#include "linuxBoot/image.h"

#include "memoryManager/addressing.h" /* For virtual addressing initialisation */
#include "memoryManager/cp15coproc.h"
#include "memoryManager/frameAllocator.h"

#ifdef CONFIG_GUEST_FREERTOS
#include "rtosBoot/bootRtos.h"
#endif

#include "vm/omap35xx/hardwareLibrary.h"
#include "vm/omap35xx/LED.h"


#define HIDDEN_RAM_START   0x8f000000
#define HIDDEN_RAM_SIZE    0x01000000 // 16 MB

extern void setGuestContext(GCONTXT *gContext);

static void printUsage(void);
static int parseCommandline(int argc, char *argv[], u32int *kernAddr, u32int *initrdAddr);

#ifdef CONFIG_MMC
fatfs mainFilesystem;
partitionTable primaryPartitionTable;
struct mmc *mmcDevice;
file * debugStream;
#endif

#ifdef CONFIG_GUEST_FREERTOS
bool rtos;
#endif


void main(s32int argc, char *argv[])
{
  u32int kernAddr = 0;
  u32int initrdAddr = 0;
  int ret = 0;

#ifdef CONFIG_GUEST_FREERTOS
  rtos = FALSE;
#endif

  /* save power: cut the clocks to the display subsystem */
  cmDisableDssClocks();

  mallocInit(HIDDEN_RAM_START, HIDDEN_RAM_SIZE);

  /* initialise uart backend, important to be before any debug output. */
  /* init function initialises UARTs in disabled mode. */
  /* startup fuction starts operation and enables RX IRQ generation */
  beUartInit(1);
  beUartInit(2);
  beUartInit(3);
#ifdef CONFIG_UART_FAST
  beUartStartup(3, 500000);
#else
  beUartStartup(3, 115200);
#endif

#ifndef CONFIG_CLI
  if ((ret = parseCommandline(argc, argv, &kernAddr, &initrdAddr)) < 0)
  {
    printUsage();
    DIE_NOW(0, "Hypervisor startup aborted.");
  }
#endif /* CONFIG_CLI */

  /* create the frametable from which we can alloc memory */
  initialiseFrameTable();

  /* initialize guest context */
  setGuestContext(allocateGuest());

  /* Setup MMU for Hypervisor */
  initialiseVirtualAddressing();

#ifdef CONFIG_CLI
  /*
   * FIXME: deal with guest context and VIRTUAL ADDRESSING!!!
   */
  enterCliLoop();
#else

  /* initialise physical interrupt controller */
  intcBEInit();

  /* now we can unmask first interrupt - UART */
  unmaskInterruptBE(UART3_IRQ);

  /* initialise physical clock manager */
  clkManBEInit();

  /* initialise phyiscal GPT2, dedicated to guest1 */
  gptBEInit(2);

#ifdef CONFIG_MMC
  u32int err = 0;
  if ((err = mmcMainInit()) != 0)
  {
    DIE_NOW(0, "Failed to initialize mmc code.\n");
  }

  if ((err = partTableRead(&mmcDevice->blockDev, &primaryPartitionTable)) != 0)
  {
    DIE_NOW(0, "Failed to read partition table.\n");
  }

  if ((err = fatMount(&mainFilesystem, &mmcDevice->blockDev, 1)) != 0)
  {
    DIE_NOW(0, "Failed to mount FAT partition.\n");
  }

  debugStream = fopen(&mainFilesystem, "debug");
  if (debugStream == 0)
  {
    DIE_NOW(0, "Failed to open (create) debug stream file.\n");
  }
#endif /* CONFIG_MMC */

  // does not return
#ifdef CONFIG_GUEST_FREERTOS
  if (ret == 2)
  {
    rtos = TRUE;
#ifdef CONFIG_DEBUG_STARTUP
    printf("RTOS address: %x\n", kernAddr);
#endif
    doRtosBoot(kernAddr);
  }
  else
#endif
  {
#ifdef CONFIG_DEBUG_STARTUP
    printf("Kernel address: %x, Initrd address: %x\n", kernAddr, initrdAddr);
#endif
    image_header_t imageHeader = getImageHeader(kernAddr);
#ifdef CONFIG_DEBUG_STARTUP
    dumpHdrInfo(&imageHeader);
#endif
    doLinuxBoot(&imageHeader, kernAddr, initrdAddr);
  }
#endif /* CONFIG_CLI */
}

static void printUsage(void)
{
  printf(
      "Loader usage:" EOL
      "go <loaderAddr> -kernel <kernAddress> -initrd <initrdAddr>" EOL
      "kernel: address of kernel in hexadecimal representation" EOL
      "initrd: address of external initrd in hexadecimal representation" EOL
#ifdef CONFIG_GUEST_FREERTOS
      "rtos: address of rtos in hex format (0xXXXXXXXX)" EOL
#endif
    );
  return;
}

static int parseCommandline(s32int argc, char *argv[], u32int *kernAddr, u32int *initrdAddr)
{
#ifdef CONFIG_DEBUG_STARTUP
  s32int i;
#endif
#ifdef CONFIG_GUEST_FREERTOS
  bool rtos;
#endif

#ifdef CONFIG_DEBUG_STARTUP
  printf("parseCommandline: argc = %d" EOL, argc);
  for (i = 0; i < argc; ++i)
  {
    printf("parseCommandline: argv[%d] = %p" EOL, i, argv[i]);
  }
#endif

  if (argc < 3)
  {
    return -1;
  }

  if (strcmp("-kernel", argv[1]) == 0)
  {
#ifdef CONFIG_GUEST_FREERTOS
    rtos = FALSE;
  }
  else if (strcmp("-rtos", argv[1]) == 0)
  {
    rtos = TRUE;
#endif
  }
  else
  {
#ifdef CONFIG_DEBUG_STARTUP
    printf("parseCommandline: parameter -kernel or -rtos not found" EOL);
#endif
    return -1;
  }

  if (sscanf(argv[2], "%x", kernAddr) != 1)
  {
#ifdef CONFIG_DEBUG_STARTUP
    printf("parseCommandline: parameter value for %s is not a valid address: '%s'" EOL, argv[1], argv[2]);
#endif
    return -1;
  }

#ifdef CONFIG_GUEST_FREERTOS
  if (rtos)
  {
    return 2;
  }
#endif

  if (argc != 5)
  {
    return -1;
  }

  if (strcmp("-initrd", argv[3]) != 0)
  {
#ifdef CONFIG_DEBUG_STARTUP
    printf("parseCommandline: parameter -initrd not found" EOL);
#endif
    return -1;
  }

  if (sscanf(argv[4], "%x", initrdAddr) != 1)
  {
#ifdef CONFIG_DEBUG_STARTUP
    printf("parseCommandline: parameter value for -initrd is not a valid address: '%s'" EOL, argv[4]);
#endif
    return -1;
  }

  return 1;
}

