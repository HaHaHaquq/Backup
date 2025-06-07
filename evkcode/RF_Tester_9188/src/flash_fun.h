#ifndef __FLASH_FUN_H
#define __FLASH_FUN_H

#include <stdint.h>

uint32_t flash_read(volatile uint32_t *addr_ptr, uint32_t *buf, uint32_t len);






#endif
