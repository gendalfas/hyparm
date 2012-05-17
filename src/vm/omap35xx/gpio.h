#ifndef __VM__OMAP_35XX__GPIO_H__
#define __VM__OMAP_35XX__GPIO_H__

#include "common/compiler.h"
#include "common/types.h"

#include "vm/omap35xx/hardwareLibrary.h"


/************************
 * REGISTER DEFINITIONS *
 ************************/
#define GPIO_REVISION            0x000

#define GPIO_SYSCONFIG           0x010
#define GPIO_SYSCONFIG_RESERVED     0xFFFFFFE0
#define GPIO_SYSCONFIG_IDLEMODE     0x00000018
#define GPIO_SYSCONFIG_ENAWAKEUP    0x00000004
#define GPIO_SYSCONFIG_SOFTRESET    0x00000002
#define GPIO_SYSCONFIG_AUTOIDLE     0x00000001

#define GPIO_SYSSTATUS           0x014
#define GPIO_SYSSTATUS_RESERVED     0xFFFFFFFE
#define GPIO_SYSSTATUS_RESETDONE    0x00000001

#define GPIO_IRQSTATUS1          0x018
#define GPIO_IRQENABLE1          0x01C
#define GPIO_WAKEUPENABLE        0x020
#define GPIO_IRQSTATUS2          0x028
#define GPIO_IRQENABLE2          0x02C

#define GPIO_CTRL                0x030
#define GPIO_CTRL_RESERVED          0xFFFFFFF8
#define GPIO_CTRL_GATERATIO         0x00000006
#define GPIO_CTRL_DISABLEMOD        0x00000001

#define GPIO_OE                  0x034
#define GPIO_DATAIN              0x038
#define GPIO_DATAOUT             0x03C
#define GPIO_LEVELDETECT0        0x040
#define GPIO_LEVELDETECT1        0x044
#define GPIO_RISINGDETECT        0x048
#define GPIO_FALLINGDETECT       0x04C
#define GPIO_DEBOUNCENABLE       0x050
#define GPIO_DEBOUNCINGTIME      0x054
#define GPIO_CLEARIRQENABLE1     0x060
#define GPIO_SETIRQENABLE1       0x064
#define GPIO_CLEARIRQENABLE2     0x070
#define GPIO_SETIRQENABLE2       0x074
#define GPIO_CLEARWKUENA         0x080
#define GPIO_SETWKUENA           0x084
#define GPIO_CLEARDATAOUT        0x090
#define GPIO_SETDATAOUT          0x094


void connectGpio(u32int gpioNumber, u32int physicalGpioNumber) __cold__;

void initGpio(u32int gpioNumber) __cold__;

u32int loadGpio(device *dev, ACCESS_SIZE size, u32int virtualAddress, u32int physicalAddress);

void storeGpio(device *dev, ACCESS_SIZE size, u32int virtualAddress, u32int physicalAddress, u32int value);


struct Gpio
{
  u32int gpioRevision;
  u32int gpioSysConfig;
  u32int gpioSysStatus;
  u32int gpioIrqStatus1;
  u32int gpioIrqEnable1;
  u32int gpioWakeupEnable;
  u32int gpioIrqStatus2;
  u32int gpioIrqEnable2;
  u32int gpioCtrl;
  u32int gpioOE;
  u32int gpioDataIn;
  u32int gpioDataOut;
  u32int gpioLvlDetect0;
  u32int gpioLvlDetect1;
  u32int gpioRisingDetect;
  u32int gpioFallingDetect;
  u32int gpioDebounceEnable;
  u32int gpioDebouncingTime;
  s32int physicalId;
};

#endif
