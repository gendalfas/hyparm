#ifndef __COMMON__LINKER_H__
#define __COMMON__LINKER_H__

#include "common/types.h"


#define HYPERVISOR_BEGIN_ADDRESS        ((u32int)&(__HYPERVISOR_BEGIN__))
#define HYPERVISOR_END_ADDRESS          ((u32int)&(__HYPERVISOR_END__))

#define HYPERVISOR_TEXT_BEGIN_ADDRESS   ((u32int)&(__TEXT_BEGIN__))
#define HYPERVISOR_TEXT_END_ADDRESS     ((u32int)&(__TEXT_END__))
#define HYPERVISOR_RO_XN_BEGIN_ADDRESS  ((u32int)&(__RODATA_BEGIN__))
#define HYPERVISOR_RO_XN_END_ADDRESS    ((u32int)&(__RODATA_END__))
#define HYPERVISOR_RW_XN_BEGIN_ADDRESS  ((u32int)&(__DATA_BEGIN__))
#define HYPERVISOR_RW_XN_END_ADDRESS    ((u32int)&(__BSS_END__))

#define RAM_XN_POOL_BEGIN               ((u32int)&(__RAM_XN_POOL_BEGIN__))
#define RAM_XN_POOL_END                 ((u32int)&(__RAM_XN_POOL_END__))


/*
 * Do NOT use the following symbols directly; use the macros above!
 */
extern void *__HYPERVISOR_BEGIN__;
extern void *__TEXT_BEGIN__;
extern void *__TEXT_END__;
extern void *__RODATA_BEGIN__;
extern void *__RODATA_END__;
extern void *__DATA_BEGIN__;
extern void *__DATA_END__;
extern void *__BSS_BEGIN__;
extern void *__BSS_END__;
extern void *__HYPERVISOR_END__;

extern void *__RAM_XN_POOL_BEGIN__;
extern void *__RAM_XN_POOL_END__;

#endif /* __COMMON_LINKER_H__ */