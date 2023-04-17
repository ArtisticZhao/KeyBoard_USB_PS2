#ifndef __DRIVER_H__
#define __DRIVER_H__

#include<stdint.h>
#include "fifo.h"


//#define _PS2DBG

#define PS2_CLK 19
#define PS2_DATA 18
#define GPIO_OUTPUT_PIN_SEL  ((1ULL<<PS2_CLK) | (1ULL<<PS2_DATA))
#define _ps2clk PS2_CLK
#define _ps2data PS2_DATA



#define PS2_KEY_TYPE_NONE 0x00
#define PS2_KEY_TYPE_NORMAL 0x01
#define PS2_KEY_TYPE_E0 0x02
#define PS2_KEY_TYPE_E1 0x04

void init_io();
void init_keyboard();

void sim_key(keyEvent* key);

// 定义机打时延 目前一秒五个
#define TypingTimeout  20
void is_idle();   // auto handle return

#endif
