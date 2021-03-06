#include "common/debug.h"
#include "common/string.h"

#include "guestBoot/image.h"
#include "guestBoot/loader.h"
#include "guestBoot/linux.h"


// hardcoded initrd len?!
#define BOARD_INITRD_LEN     0x800000


void bootLinux(GCONTXT *context, u32int loadAddress, u32int initrdAddress, const char *arguments)
{
  u32int currentAddress = loadAddress + sizeof(image_header_t);

  DEBUG(STARTUP, "bootLinux: load address = %#.8x, initrd address = %#.8x" EOL, loadAddress,
      initrdAddress);

  image_header_t imageHeader = getImageHeader(loadAddress);
#if (CONFIG_DEBUG_STARTUP)
  dumpHdrInfo(&imageHeader);
#endif
  u32int targetAddress = imageHeader.ih_load;
  u32int entryPoint = imageHeader.ih_ep;
  u32int sizeInBytes = imageHeader.ih_size;

  if (currentAddress != targetAddress)
  {
    DEBUG(STARTUP, "bootLinux: relocating kernel from %#.8x to %#.8x" EOL, currentAddress,
        targetAddress);
    memmove((void *)targetAddress, (const void *)currentAddress, sizeInBytes);
  }

  struct tag *tagList = getTagListBaseAddress();
  setupStartTag(&tagList);
  setupRevisionTag(&tagList);
  setupMemoryTags(&tagList);
  if (initrdAddress != 0)
  {
    DEBUG(STARTUP, "%s: passing ramdisk address to kernel: %p" EOL, __func__, (void *)initrdAddress);
    setupInitrdTag(&tagList, initrdAddress, initrdAddress + BOARD_INITRD_LEN);
  }
  if (arguments != NULL && strlen(arguments) > 0)
  {
    DEBUG(STARTUP, "%s: passing extra command-line arguments to kernel: %s" EOL, __func__, arguments);
    setupCommandLineTag(&tagList, arguments);
  }
  setupEndTag(&tagList);

  bootGuest(context, GUEST_OS_LINUX, entryPoint);
}
