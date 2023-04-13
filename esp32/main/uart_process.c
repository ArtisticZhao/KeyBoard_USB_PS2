#include "uart_process.h"
#include <stdint.h>
#include <stdio.h>
#include "esp_log.h"

#include "ps2.h"

static char rxdata[PRINT_BUFSIZE];

char* print_bytes(const char* in, size_t size) {
    for (size_t i = 0; i < size; ++i) {
        sprintf((char*)(rxdata+i*3), "%02X ", in[i]);
    }
    rxdata[size*3] = '\0';
    return rxdata;
}

#define IDLE    0   // 0x57
#define HEADER1 1   // 0xab
#define HEADER2 2   // 0x88
#define LEN     3   //
#define DATA    4
void ch9350_parser(const char* in, size_t size) {
    // 解析采用状态机来实现
    static uint8_t ser_fsm_state = 0;
    static uint8_t recv_tmp = 0;
    static uint8_t recv_cnt = 0;
    static uint8_t current_len = 0;
    static uint8_t cmd[CMD_BUFSIZE];
    for (size_t i = 0; i < size; ++i) {
        recv_tmp = in[i];
        switch(ser_fsm_state){
            case IDLE:
                if (recv_tmp == 0x57) {
                    ser_fsm_state = HEADER1;
                    recv_cnt = 0;
                }
                else{
                    ser_fsm_state = IDLE;
                }
                break;
            case HEADER1:
                if (recv_tmp == 0xab) {
                    ser_fsm_state = HEADER2;
                }
                else{
                    ser_fsm_state = IDLE;
                }
                break;
            case HEADER2:
                if (recv_tmp == 0x88) {
                    ser_fsm_state = LEN;
                }
                else{
                    ser_fsm_state = IDLE;
                }
                break;
            case LEN:
                current_len = recv_tmp;
                ser_fsm_state = DATA;
                break;
            case DATA:
                cmd[recv_cnt] = recv_tmp;
                recv_cnt++;
                if (recv_cnt == current_len){
                    ser_fsm_state = IDLE;
                    // recv whole keyboard cmd;
                    // call parser_hid
                    parser_hid((const uint8_t*)cmd);
                }
                break;
            default:
                ser_fsm_state = IDLE;
                break;
        }
    }
}
