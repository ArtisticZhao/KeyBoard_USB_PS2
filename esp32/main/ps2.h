#ifndef __PS_2_H_
#define __PS_2_H_
#include <stdint.h>

#define KEY_PRESS 1
#define KEY_RELEASE 0

#define PS2_KEY_TYPE_NONE 0x00
#define PS2_KEY_TYPE_NORMAL 0x01
#define PS2_KEY_TYPE_E0 0x02
#define PS2_KEY_TYPE_E1 0x04

typedef struct Ps2Key {
    uint8_t key;
    uint8_t type;
    char    name[10];
}Ps2Key;


// 初始化查找表
void init_table();

// 解析按键值
void parser_hid(const uint8_t* in);

// 打印按键值和状态
//#define __DEBUG
void _debug(Ps2Key* key, uint8_t mode);
#endif
