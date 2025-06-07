#ifndef __AMO_CTRL_H
#define __AMO_CTRL_H

#include <stdint.h>
#include "platform_api.h"

typedef enum{
    AMO_READY,
    AMO_BURNING,
    AMO_FAILED,
    AMO_SUCCESS
}amo_burn_status;


#endif