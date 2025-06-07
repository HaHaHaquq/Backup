#include "flash_fun.h"
#include "platform_api.h"
#include <stdint.h>

uint32_t flash_read(volatile uint32_t *addr_ptr, uint32_t *buf, uint32_t len) {
    if (*addr_ptr % 4 != 0) {
        return 1; 
    }
    volatile uint32_t *flash_ptr = (volatile uint32_t*)(*addr_ptr);
    uint32_t words_to_read = len / 4;

    for (uint32_t i = 0; i < words_to_read; i++) {
        buf[i] = flash_ptr[i];
    }
    *addr_ptr += len;
    return 0;
}
