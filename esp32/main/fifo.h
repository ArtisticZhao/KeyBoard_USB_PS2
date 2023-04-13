#ifndef __FIFO_H__
#define __FIFO_H__
#include <stdint.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

#include "ps2.h"

// 按键消息结构体
typedef struct keyEvent {
    uint8_t ps2_keycode;
    uint8_t ps2_keytype;
    uint8_t status;   // 1-press 0-release
} keyEvent;

#define FIFO_SIZE (16*5)
#define FIFO_ITEM_SIZE sizeof(keyEvent)

//extern QueueHandle_t key_event_fifo;
void put_key(const Ps2Key* key, uint8_t status);
void get_key(const Ps2Key* key, uint8_t status);

#endif
