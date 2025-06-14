#ifndef _UART_OTA_H_
#define _UART_OTA_H_

#include <stdint.h>

//typedef  struct {
//    uint8_t start_byte; // Start byte
//    uint8_t length;     // Length of the data
//    uint8_t command;    // Command byte
//    uint8_t data[255];  // Data bytes
//    uint8_t checksum;    // Checksum byte
//} UART_Packet;

#define BURN_BIN_NUM_MAX  6
#pragma pack (push, 1)
typedef struct burn_block
{
	int check;
	char filename[64];
    uint32_t loadaddr;
    uint32_t size;
} burn_block_t;

typedef struct burn_meta
{
    uint16_t sector_size;
    uint16_t page_size;
	uint32_t total_size;
    uint8_t  block_num;    
    burn_block_t blocks[BURN_BIN_NUM_MAX];
} burn_meta_t;

typedef struct downdloader_cfg
{
	uint8_t family;
    uint32_t baud;
    uint32_t timerout;
	int  verify;
    int  protect_enable;    
    int  un_lock;
    int  sn_burn_enable;
	int  set_entry;
	uint32_t entry_addr;
    uint8_t  sn_code_len;
    uint32_t sn_code_addr;
    uint32_t sn_start;
    uint16_t sn_step;
	burn_meta_t mate;
} downdloader_cfg_t;

#pragma pack (pop)

typedef enum : uint8_t{
	BURN_STATE_OK = 0,
    BURN_STATE_START,
    BURN_STATE_BURNING,
	BURN_STATE_FLASH_LOCK,
	BURN_STATE_TIMEROUT,
    BURN_STATE_MALL_FAIL,
	BURN_STATE_MEM_OVER,
    BURN_STATE_FILE_ERR,
    BURN_STATE_FLASH_READ_ERR,
	BURN_STATE_NAK,
	BURN_STATE_UNKNOW_CMD,
	BURN_STATE_VOID,
    BURN_STATE_CMPL
}buner_state_t;

extern  void uart_buner_rx_data(const char *d, uint8_t len,uint8_t cmpl);
extern  void uart_buner_start(void);
extern  downdloader_cfg_t *get_downloader_cfg(void);
buner_state_t get_buner_state(void);
#endif


