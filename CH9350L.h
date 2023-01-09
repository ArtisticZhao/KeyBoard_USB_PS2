#ifndef CH9350L_H
#define CH9350L_H

#include "HardwareSerial.h"
#include "ps2dev.h"

struct Ps2Key {
  uint8_t key;
  uint8_t type;
  char    name[5];    // 字符串最多写四个字符
};



class Ch9350 {
  public:
    Ch9350(HardwareSerial* ser, PS2dev* ps2);
    void init_ch9350();
    void do_process(char* recv, int len);
  private:
    HardwareSerial* _ser;
    PS2dev* _ps2;
    // HID Usage ID Max = E7 (Usage Page 07), E8-FFFF Reserved
    Ps2Key hid2ps2[0xE8];
    uint8_t last_mod;
    uint8_t last_key[6];

    void _init_hid2ps2();
    void _ack_slave();
    void _debug(bool press, uint8_t code, char* key);

};

#endif