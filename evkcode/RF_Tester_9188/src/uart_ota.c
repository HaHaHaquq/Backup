#include "uart_ota.h"
#include "ingsoc.h"
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "platform_api.h"
#include "port_gen_os_driver.h"
#include "errno.h"
#include "flash_fun.h"
#include "evk_config.h"
#include "uart_golden_DUT.h"
#include "profile.h"
#include "crc_16.h"
#include "btstack_util.h"


#define  LOAD_BAUD_RATE     (1000000u)

#ifndef SHAKE_BAUD_RATE
#define  SHAKE_BAUD_RATE    (115200u)
#endif

#define STR_CMD_BURN_STATE        "#$state"
#define STR_CMD_SET_BAUD          "#$sbaud"
#define STR_CMD_CHIP_LOCK         "#$lockk"
#define STR_CMD_CHIP_UNLOCK       "#$unlck"

#define STR_CMD_CHIP_ERASE        "#$chera"
#define STR_CMD_SECTOR_ERASE      "#$stera"



#define STR_CMD_BURN_FLASHST      "#$fshst"
#define STR_CMD_PROM_FLASH        "#$u2fsh"
#define STR_CMD_PROM_EFUES        "#$u2efs"
#define STR_CMD_READ_EFUES        "#$efs2u"


#define STR_CMD_BURN_START        "#$start"
#define STR_CMD_READ_FLASH        "#$readd"
#define STR_CMD_SET_JUMP_ADDR     "#$setja"
#define STR_CMD_START_JUMP        "#$jumpp"

volatile uint32_t self_addr = EVK_BIN_ADDR;

typedef enum
{
    EVENT_ID_UART_CMD_INPUT  = 0x0001,
    EVENT_ID_UART_BURN_START,
    EVENT_ID_UART_BURN_ULK,
    EVENT_ID_UART_BURN_LOCK,    
    EVENT_ID_UART_ACK,
    EVENT_ID_UART_NACK,
    EVENT_ID_UART_UNKNOW_CMD,
    EVENT_ID_UART_IDLE

}BUNER_STATUS;
#define GEN_OS          ((const gen_os_driver_t *)platform_get_gen_os_driver())

static gen_handle_t     cmd_event = NULL;
static evk_config_t *evk_ota = NULL;
static BUNER_STATUS  burner_steps = EVENT_ID_UART_IDLE;
uint32_t *Burn_Buf = NULL;


typedef struct
{
    uint8_t busy;
    uint16_t size;
    char buf[256];
} str_buf_t;

str_buf_t input = {0};

typedef void (*f_cmd_handler)(const char *param);

typedef struct
{
    const char *cmd;
    f_cmd_handler handler;
} cmd_t;


downdloader_cfg_t downdloader = {
	.family = INGCHIPS_FAMILY_916,
	.baud = LOAD_BAUD_RATE,
	.timerout = 1000,
	.verify = 0,
	.protect_enable = 0,
	.un_lock = 1,
	.sn_burn_enable = 1,
	.sn_code_len = 6,
	.sn_code_addr = 0x2074000,
	.sn_start = 1234,
	.sn_step = 1,
	.mate.sector_size = 4096,
	.mate.page_size = 256,
	.mate.block_num = 1,
	.mate.blocks = {[0] = {.size = 181552,
                            .check = 1,
                            .filename = "test.bin",
                            .loadaddr = 0x02002000
                        }
                    },
};



static void uart_event_set(uint16_t event_id);
static void uart_buner_tx_data(const char *cmd, const uint16_t cmd_len,uint8_t *param,uint16_t param_len);

downdloader_cfg_t *get_downloader_cfg(void)
{
    return &downdloader;
}

void cmd_uart_burn_sart(const char *param)
{
	DBG_PRINTF("event Burn start\r\n");
    ((evk_ota = get_evk_config())->work) = 1; 
    uart_event_set(EVENT_ID_UART_BURN_START);
}

void cmd_uart_ack(const char *param)
{
    uart_event_set(EVENT_ID_UART_ACK);
}

void cmd_uart_unknow_cmd(const char *param)
{
    uart_event_set(EVENT_ID_UART_UNKNOW_CMD);
}

void cmd_uart_nack(const char *param)
{
    uart_event_set(EVENT_ID_UART_NACK);
}

void cmd_uart_burn_ulk(const char *param)
{
    uart_event_set(EVENT_ID_UART_BURN_ULK);
}

void cmd_uart_burn_lock(const char *param)
{
    uart_event_set(EVENT_ID_UART_BURN_LOCK);
}

const static cmd_t cmds[] =
{
    {
        .cmd = "UartBurnStart916",
        .handler = cmd_uart_burn_sart
    },
    {
        .cmd = "#$ack\n",
        .handler = cmd_uart_ack
    },
    {
        .cmd = "#$nak\n",
        .handler = cmd_uart_nack
    },
    {
        .cmd = "#$ulk\n",
        .handler = cmd_uart_burn_ulk
    },
    {
        .cmd = "#$lck\n",
        .handler = cmd_uart_burn_lock
    }    
};

const static cmd_t cmds_918[] =
{
    {
        .cmd = "UartBurnStart\n",
        .handler = cmd_uart_burn_sart
    },
    {
        .cmd = "#$ack\n",
        .handler = cmd_uart_ack
    },
    {
        .cmd = "#$nak\n",
        .handler = cmd_uart_nack
    },
    {
        .cmd = "#$ulk\n",
        .handler = cmd_uart_burn_ulk
    },
    {
        .cmd = "#$lck\n",
        .handler = cmd_uart_burn_lock
    }    
};

void handle_command(char *cmd_line)
{
    char *param = cmd_line;
	const cmd_t *pCmd = cmds;
    int i;
	if(INGCHIPS_FAMILY_918 == downdloader.family) pCmd = cmds_918;
    while (*param)
    {
        if (*param == ' ')
        {
            *param = '\0';
            param++;
            break;
        }
        else
            param++;
    }

    for (i = 0; i < sizeof(cmds) / sizeof(cmds[0]); i++)
    {    
        if (strcasecmp(pCmd[i].cmd, cmd_line) == 0)
            break;
    }
    if (i >= sizeof(cmds) / sizeof(cmds[0]))
        goto show_help;

    cmds[i].handler(param);
    return;

show_help:

    cmd_uart_unknow_cmd((void*) NULL);

}



typedef struct burner_s
{
	buner_state_t buner_work_mode;
	burn_block_t block;
    uint8_t  binIndex;
	uint16_t total_burn_sec_nums;
	uint32_t total_burn_data_size;
	uint8_t uart_burn_step;
	uint16_t block_index;    
	uint16_t sector_index ;
	uint16_t pag_index ;	
	uint8_t param[32];	
} burner_t;

burner_t ing_buner;

static void burn_ctrl_restart(burner_t *pBurner)
{
	memset(pBurner,0,sizeof(burner_t));
}

void uart_buner_task_entry(void)
{
        switch(burner_steps)
        {
            case EVENT_ID_UART_CMD_INPUT:
            {
            //    DBG_PRINTF("CMD_INPUT\r\n");
                handle_command(input.buf);
								memset(input.buf,0,input.size);
                input.size = 0;
                input.busy = 0;   
            }break;
            case EVENT_ID_UART_BURN_START:
            {
	
				DBG_PRINTF("Burn start\r\n"); 
				burn_ctrl_restart(&ing_buner);
				for(ing_buner.binIndex = 0;ing_buner.binIndex < BURN_BIN_NUM_MAX;ing_buner.binIndex++)
				{
					if(1 == downdloader.mate.blocks[ing_buner.binIndex].check)
					{
						DBG_PRINTF("binindex is %d\n",ing_buner.binIndex);
						memcpy(&ing_buner.block,&downdloader.mate.blocks[ing_buner.binIndex],sizeof(burn_block_t));
						break;
					}
				
				}
                ing_buner.buner_work_mode = BURN_STATE_START;
		        Burn_Buf = (uint32_t*)malloc(downdloader.mate.page_size);	
				if (NULL == Burn_Buf) {
                    ing_buner.buner_work_mode = BURN_STATE_MALL_FAIL;
					DBG_PRINTF("%s open failed\r\n",ing_buner.block.filename); 
					Burn_Buf = NULL;
					break;
				}
				DBG_PRINTF("Uart burn start #bin%d - %s\r\n",ing_buner.binIndex,ing_buner.block.filename);
				if(ing_buner.binIndex < BURN_BIN_NUM_MAX) {
                    uart_buner_tx_data(STR_CMD_BURN_STATE,strlen(STR_CMD_BURN_STATE),NULL,0);
                }
            }
            break;     
            case EVENT_ID_UART_ACK:
            {
                switch(ing_buner.uart_burn_step)
                {
                    case burn_step1:
                    {
                        DBG_PRINTF("burn step 1 \n");
                        DBG_PRINTF("Set baud :%d\r\n",downdloader.baud);
                        apUART_BaudRateSet(APB_UART1,OSC_CLK_FREQ,downdloader.baud);
                        ing_buner.param[0] = 00;
                        ing_buner.param[1] = 0x20;
                        delay_ms(10);
                        DBG_PRINTF("set flash qspi mode\r\n");
                        uart_buner_tx_data(STR_CMD_BURN_FLASHST,strlen(STR_CMD_BURN_FLASHST),ing_buner.param,2);
                        ing_buner.uart_burn_step = burn_step2;
                                             
                    }break;
                    case burn_step2:
                    {         
                        DBG_PRINTF("burn step 2 \n"); 
						if((INGCHIPS_FAMILY_918 ==  downdloader.family) && (ing_buner.sector_index == 0) && (ing_buner.block_index == 0))
						{
							 DBG_PRINTF("Set baud :%d\r\n",downdloader.baud);
							 apUART_BaudRateSet(APB_UART1,OSC_CLK_FREQ,downdloader.baud);
						}
                        if(ing_buner.sector_index * downdloader.mate.sector_size >= ing_buner.block.size)
                        {
                            ing_buner.block_index += 1;
                            ing_buner.sector_index = 0;   
							ing_buner.binIndex += 1;
							for(;ing_buner.binIndex < BURN_BIN_NUM_MAX; ing_buner.binIndex++)
							{
								if(1 == downdloader.mate.blocks[ing_buner.binIndex].check)
								{
									memcpy(&ing_buner.block,&downdloader.mate.blocks[ing_buner.binIndex],sizeof(burn_block_t));
									break;
								}
							
							}				
							if(ing_buner.binIndex<BURN_BIN_NUM_MAX)
							DBG_PRINTF("Uart burn start #bin%d - %s\r\n",ing_buner.binIndex,ing_buner.block.filename);							
                        }

                        if(ing_buner.block_index >= downdloader.mate.block_num) 
                        {
                            DBG_PRINTF("burn co %d\n",ing_buner.block_index);
							DBG_PRINTF("si %d, pag %d\n",ing_buner.sector_index, ing_buner.pag_index);
                            ing_buner.uart_burn_step = 0;
                            ing_buner.block_index = 0;
                            ing_buner.sector_index = 0;
                            ing_buner.total_burn_sec_nums = 0;
        
							if(1 == downdloader.set_entry && INGCHIPS_FAMILY_918 == downdloader.family)
							{
							    uint32_t *p = (uint32_t *)ing_buner.param;
							    *p  = downdloader.set_entry;
								ing_buner.uart_burn_step = burn_step7;
							    uart_buner_tx_data(STR_CMD_SET_JUMP_ADDR,strlen(STR_CMD_SET_JUMP_ADDR),(uint8_t*)p,4);  
							}
							else if(1 == downdloader.protect_enable)
                            {
                                uart_buner_tx_data(STR_CMD_CHIP_LOCK,strlen(STR_CMD_CHIP_LOCK),NULL,0);  
                                ing_buner.uart_burn_step = burn_step6;                                
                            }
                            DBG_PRINTF("Burn complete\r\n");
                            int erro = apUART_BaudRateSet(APB_UART1,OSC_CLK_FREQ,115200);//This is available, plus the next version
							DBG_PRINTF("error is %d\n", erro);
                            ing_buner.buner_work_mode = BURN_STATE_CMPL;
							free(Burn_Buf);
							Burn_Buf = NULL;
                            break;                        
                        }      
                        ing_buner.buner_work_mode = BURN_STATE_BURNING;
                        ing_buner.pag_index = 0;                        
                        uint32_t *p = (uint32_t *)ing_buner.param;
						*p = ing_buner.block.loadaddr + ing_buner.sector_index*downdloader.mate.sector_size;
						// printf("Bunr:%x\r\n",(uint32_t)p[0]);
						if(INGCHIPS_FAMILY_916 == downdloader.family){
							ing_buner.uart_burn_step = burn_step3;
							uart_buner_tx_data(STR_CMD_SECTOR_ERASE,strlen(STR_CMD_SECTOR_ERASE),(uint8_t*)p,4);  
						}else
						{
                            //Start reading data
                            uint32_t res = flash_read(&self_addr, Burn_Buf, downdloader.mate.page_size);
							if (res) {
							    DBG_PRINTF("Flash read err\r\n");
                                ing_buner.buner_work_mode = BURN_STATE_FLASH_READ_ERR;
								ing_buner.uart_burn_step = 0;
								free(Burn_Buf);
								Burn_Buf = NULL;
								break;
							}else
							{
								*(uint16_t*)(p+1) = downdloader.mate.page_size;
								ing_buner.uart_burn_step = burn_step4;
								uart_buner_tx_data(STR_CMD_BURN_START,strlen(STR_CMD_BURN_START),(uint8_t*)p,6);  
							}
						}
                        ing_buner.sector_index += 1;
                        ing_buner.total_burn_sec_nums += 1;
                    }break;
                    case burn_step3:
                    {    
                        DBG_PRINTF("burn step 3 \n");      
                        uint32_t *p = (uint32_t *)ing_buner.param;
                        *p = ing_buner.block.loadaddr + (ing_buner.sector_index-1)*downdloader.mate.sector_size + ing_buner.pag_index*downdloader.mate.page_size;
                        *(p+1) = downdloader.mate.page_size-1;
                        ing_buner.pag_index += 1;                                              
                        uart_buner_tx_data(STR_CMD_PROM_FLASH,strlen(STR_CMD_PROM_FLASH),(uint8_t*)p,5);    
                        ing_buner.uart_burn_step = burn_step4;
                    }break;      
                    case burn_step4:
                    {   
                        uint16_t  crc = 0;  
                        DBG_PRINTF("burn step 4 \n");
						if(INGCHIPS_FAMILY_916 == downdloader.family )
						{
                            uint32_t res  = flash_read(&self_addr, Burn_Buf, downdloader.mate.page_size);
                            // DBG_PRINTF("addr is %x\n",self_addr);
							if (res) {
                                DBG_PRINTF("Flash read err\r\n");
                                ing_buner.buner_work_mode = BURN_STATE_FLASH_READ_ERR;
                                ing_buner.uart_burn_step = 0;
                                free(Burn_Buf);
                                Burn_Buf = NULL;
                                break;
							}
							else
							{
                                ing_buner.total_burn_data_size += downdloader.mate.page_size;
                                crc = check_crc16((char*)Burn_Buf,downdloader.mate.page_size);

								driver_append_tx_data(Burn_Buf, downdloader.mate.page_size);
								driver_append_tx_data((uint8_t *)&crc, 2);
								// DBG_PRINTF("w:%d\r\n",ing_buner.total_burn_data_size);
								if(downdloader.mate.page_size * (ing_buner.pag_index) >= downdloader.mate.sector_size)
								{
									ing_buner.uart_burn_step = burn_step2;
												   
								}else
								{                        
									ing_buner.uart_burn_step = burn_step3;   
								}
							}
						}else
						{
                            //918A page is 8K
							ing_buner.total_burn_data_size += downdloader.mate.page_size;
							crc = check_crc16_918((uint8_t *)Burn_Buf,downdloader.mate.page_size);
							driver_append_tx_data((uint8_t *)Burn_Buf, downdloader.mate.page_size/2);
//							vTaskDelay(pdMS_TO_TICKS(20));
                            delay_ms(10);
							driver_append_tx_data((uint8_t *)(Burn_Buf+downdloader.mate.page_size/2), downdloader.mate.page_size/2);
							driver_append_tx_data((uint8_t *)&crc, 2);
							ing_buner.uart_burn_step = burn_step2;
							ing_buner.pag_index += 1;  
						}

                        
                    }break;    
                    case burn_step5:
                    {   
                        DBG_PRINTF("burn step 5 \n");
						if(INGCHIPS_FAMILY_916 == downdloader.family)
							ing_buner.uart_burn_step = burn_step1;
						else
							ing_buner.uart_burn_step = burn_step2;
                        uint32_t *p = (uint32_t *)ing_buner.param;
                        *p = downdloader.baud;
						ing_buner.block_index = 0;
                        ing_buner.sector_index = 0;
                        ing_buner.total_burn_sec_nums = 0;
                        ing_buner.pag_index = 0;  

                        uart_buner_tx_data(STR_CMD_SET_BAUD,strlen(STR_CMD_SET_BAUD),(uint8_t*)p,4);     
                                
                    }break;     
                    case burn_step6:
                    {
                        DBG_PRINTF("burn step 6 \n");
                        ing_buner.uart_burn_step = 0;
                        DBG_PRINTF("Lock complete\r\n");
                                
                    }break;  
					case burn_step7:
                    {
                        DBG_PRINTF("burn step 7 \n");
						if(1 == downdloader.protect_enable)
						{
							uart_buner_tx_data(STR_CMD_CHIP_LOCK,strlen(STR_CMD_CHIP_LOCK),NULL,0);  
							ing_buner.uart_burn_step = burn_step6;                                
						}
						else
						{
							ing_buner.uart_burn_step = 0;
						}
					}					
                    default :break;
                
                }            
            }
            break;    
            case EVENT_ID_UART_NACK:
            {      
                DBG_PRINTF("burn step 8 \n");    
                DBG_PRINTF("nack\r\n");
				free(Burn_Buf);
				Burn_Buf = NULL;
            }break;
            case EVENT_ID_UART_BURN_ULK:
            {
                DBG_PRINTF("burn step 9 \n");
                DBG_PRINTF("flash unlock\r\n");
                if(INGCHIPS_FAMILY_916 == downdloader.family)
                    ing_buner.uart_burn_step = burn_step1;
                else
                    ing_buner.uart_burn_step = burn_step2;
                ing_buner.block_index = 0;
                ing_buner.sector_index = 0;
                ing_buner.total_burn_sec_nums = 0;
                ing_buner.pag_index = 0;  
                uint32_t *p = (uint32_t *)ing_buner.param;
                
                *p = downdloader.baud;
                delay_ms(100);
                uart_buner_tx_data(STR_CMD_SET_BAUD,strlen(STR_CMD_SET_BAUD),(uint8_t*)p,4);                    

            }break;   
            case EVENT_ID_UART_BURN_LOCK:
            {
            DBG_PRINTF("burn step 10 \n");
                DBG_PRINTF("Flash was locked\r\n");
                if(downdloader.un_lock)
                {
                ing_buner.uart_burn_step = burn_step5;
                uart_buner_tx_data(STR_CMD_CHIP_UNLOCK,strlen(STR_CMD_CHIP_UNLOCK),NULL,0);   
                }
            else
            {  
                ing_buner.uart_burn_step = 0;
            }
            
            }break;              
            case EVENT_ID_UART_UNKNOW_CMD:
            {
            DBG_PRINTF("burn step 11 \n");
                if(ing_buner.uart_burn_step)
                {
                ing_buner.block_index = 0;
                ing_buner.sector_index = 0;
                ing_buner.total_burn_sec_nums = 0;
                ing_buner.uart_burn_step = 0;
                free(Burn_Buf);
                Burn_Buf = NULL;
                }
            }
            break;                 
            default:
                break;            
    }        
    // }
}

static void append_data(str_buf_t *buf, const char *d, const uint16_t len)
{
    if (buf->size + len > sizeof(buf->buf))
        buf->size = 0;

    if (buf->size + len <= sizeof(buf->buf))
    {
        memcpy(buf->buf + buf->size, d, len);
        buf->size += len;
    }

}

void uart_buner_rx_data(const char *d, uint8_t len,uint8_t cmpl)
{
    
    if(input.busy || len == 0) return;
	
    append_data(&input, d, len);
    // DBG_PRINTF("len is %x\n",len);
    if (!(len%16) || cmpl)
    {       
        if(1 >=  input.size)
        {
            input.busy = 0;
            input.size = 0;
            return;
        }
        input.buf[input.size] = '\0';
        input.busy = 1;
        uart_event_set(EVENT_ID_UART_CMD_INPUT);     
    }
}

static void uart_event_set(uint16_t event_id)
{
    burner_steps = event_id;
    uart_buner_task_entry();
}

static void uart_buner_tx_data(const char *cmd, const uint16_t cmd_len,uint8_t *param,uint16_t param_len)
{
    if(cmd)
        driver_append_tx_data(cmd, cmd_len);
    if(param)
        driver_append_tx_data(param, param_len);
    }

buner_state_t get_buner_state(void)
{
	return ing_buner.buner_work_mode;
}
