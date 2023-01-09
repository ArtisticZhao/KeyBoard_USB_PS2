#include <ps2dev.h>    //Emulate a PS/2 device
#include "CH9350L.h"

// #define _DEBUG

unsigned long timecount = 0;
PS2dev keyboard(19,18);  //clock, data
Ch9350 keyboard_agent(&Serial2, &keyboard);
void do_process(char* recv, int len);
// ---- 串口相关变量
bool is_header = false;    // 是否收到的第一个帧头
char recv_tmp;
char recv[20];   // 存储指令数组
int  recv_cnt = 0;
int  current_len;
int ser_fsm_state = 0;
#define IDLE 0      // 0x57
#define HEADER1 1   // 0xab
#define HEADER2 2   // 0x88
#define LEN     3   // 
#define DATA    4

void serialEvent2(){
  while(Serial2.available()){
    recv_tmp = Serial2.read();
    switch(ser_fsm_state){
      case IDLE:
        if (recv_tmp == 0x57) {
          ser_fsm_state = HEADER1;
          recv_cnt = 0;
        }else{
          ser_fsm_state = IDLE;
        }
        break;
      case HEADER1:
        if (recv_tmp == 0xab) {
          ser_fsm_state = HEADER2;
        }else{
          ser_fsm_state = IDLE;
        }
        break;
      case HEADER2:
        if (recv_tmp == 0x88) {
          ser_fsm_state = LEN;
        }else{
          ser_fsm_state = IDLE;
        }
        break;
      case LEN:
        current_len = recv_tmp;
        ser_fsm_state = DATA;
        break;
      case DATA:
        recv[recv_cnt] = recv_tmp;
        recv_cnt++;
        if (recv_cnt == current_len){
          ser_fsm_state = IDLE;
          // call the processor
          do_process(recv, current_len);
        }
        break;
    }
  }
}

void do_process(char* recv, int len){
  char test[5];
#ifdef _DEBUG
  for(int i=0; i<len; i++){
    sprintf(test, "%x ", recv[i]);
    Serial.print(test);
  }
  Serial.println("");
#endif
  keyboard_agent.do_process(recv, len);
}

void setup()
{
  pinMode(LED_BUILTIN, OUTPUT);
  keyboard.keyboard_init();
  Serial.begin(115200);
  Serial2.begin(115200);
  Serial.println("Build time: 22.12.17 23:49");
  keyboard_agent.init_ch9350();
}

void loop()
{
  //Handle PS2 communication and react to keyboard led change
  //This should be done at least once each 10ms
  // unsigned char leds;
  // if(keyboard.keyboard_handle(&leds)) {
  //   Serial.printf("LED: %x\n", leds);
  //   digitalWrite(LED_BUILTIN, leds);
  // }
  // delay(2);

  // //Print a number every second
  // if((millis() - timecount) > 1000) {
  //   keyboard.keyboard_mkbrk(PS2dev::ONE);
  //   Serial.print('.');
  //   timecount = millis();
  // }

  
}
