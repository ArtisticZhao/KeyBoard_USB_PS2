#include "CH9350L.h"
#include <string.h>

#define PS2_KEY_TYPE_NONE 0x00
#define PS2_KEY_TYPE_NORMAL 0x01
#define PS2_KEY_TYPE_E0 0x02
#define PS2_KEY_TYPE_E1 0x04

struct MODIFIERKEYS {
  uint8_t LeftCtrl: 1;
  uint8_t LeftShift: 1;
  uint8_t LeftAlt: 1;
  uint8_t LeftSuper: 1;
  uint8_t RightCtrl: 1;
  uint8_t RightShift: 1;
  uint8_t RightAlt: 1;
  uint8_t RightSuper: 1;
};

char prt_str[50];

Ch9350::Ch9350(HardwareSerial* ser, PS2dev* ps2){
  this->_ser = ser;
  this->_ps2 = ps2;
}
void Ch9350::init_ch9350(){
  _ack_slave();
  _init_hid2ps2();
}


void Ch9350::do_process(char* recv, int len){
  // 判断mod keys
  MODIFIERKEYS mod;
  MODIFIERKEYS mod_last;
  // ---- mod key process
  *((uint8_t*)&mod) = recv[1];
  *((uint8_t*)&mod_last) = last_mod;
  // -- LCtrl
  if (mod_last.LeftCtrl){
    if (!mod.LeftCtrl){
      // release
      _ps2->keyboard_release(PS2dev::LEFT_CONTROL);
      _debug(false, (uint8_t)PS2dev::LEFT_CONTROL, "LCtrl");
    }
  }else{
    if (mod.LeftCtrl){
      // press
      _ps2->keyboard_press(PS2dev::LEFT_CONTROL);
      _debug(true, (uint8_t)PS2dev::LEFT_CONTROL, "LCtrl");
    }
  }
  // -- LShift
  if (mod_last.LeftShift){
    if (!mod.LeftShift){
      // release
      _ps2->keyboard_release(PS2dev::LEFT_SHIFT);
      _debug(false, (uint8_t)PS2dev::LEFT_SHIFT, "LShift");
    }
  }else{
    if (mod.LeftShift){
      // press
      _ps2->keyboard_press(PS2dev::LEFT_SHIFT);
      _debug(true, (uint8_t)PS2dev::LEFT_SHIFT, "LShift");
    }
  }
  // -- LAlt
  if (mod_last.LeftAlt){
    if (!mod.LeftAlt){
      // release
      _ps2->keyboard_release(PS2dev::LEFT_ALT);
      _debug(false, (uint8_t)PS2dev::LEFT_ALT, "LAlt");
    }
  }else{
    if (mod.LeftAlt){
      // press
      _ps2->keyboard_press(PS2dev::LEFT_ALT);
      _debug(true, (uint8_t)PS2dev::LEFT_ALT, "LAlt");
    }
  }
  // -- LSuper
  if (mod_last.LeftSuper){
    if (!mod.LeftSuper){
      // release
      _ps2->keyboard_release_special(PS2dev::LEFT_WIN);
      _debug(false, (uint8_t)PS2dev::LEFT_WIN, "LSuper");
    }
  }else{
    if (mod.LeftSuper){
      // press
      _ps2->keyboard_press_special(PS2dev::LEFT_WIN);
      _debug(true, (uint8_t)PS2dev::LEFT_WIN, "LSuper");
    }
  }
  // -- RCtrl
  if (mod_last.RightCtrl){
    if (!mod.RightCtrl){
      // release
      _ps2->keyboard_release_special(PS2dev::RIGHT_CONTROL);
      _debug(false, (uint8_t)PS2dev::RIGHT_CONTROL, "RCtrl");
    }
  }else{
    if (mod.RightCtrl){
      // press
      _ps2->keyboard_press_special(PS2dev::RIGHT_CONTROL);
      _debug(true, (uint8_t)PS2dev::RIGHT_CONTROL, "RCtrl");
    }
  }
  // -- RShift
  if (mod_last.RightShift){
    if (!mod.RightShift){
      // release
      _ps2->keyboard_release(PS2dev::RIGHT_SHIFT);
      _debug(false, (uint8_t)PS2dev::RIGHT_SHIFT, "RShift");
    }
  }else{
    if (mod.RightShift){
      // press
      _ps2->keyboard_press(PS2dev::RIGHT_SHIFT);
      _debug(true, (uint8_t)PS2dev::RIGHT_SHIFT, "RShift");
    }
  }
  // -- LRAlt
  if (mod_last.RightAlt){
    if (!mod.RightAlt){
      // release
      _ps2->keyboard_release_special(PS2dev::RIGHT_ALT);
      _debug(false, (uint8_t)PS2dev::RIGHT_ALT, "RAlt");
    }
  }else{
    if (mod.RightAlt){
      // press
      _ps2->keyboard_press_special(PS2dev::RIGHT_ALT);
      _debug(true, (uint8_t)PS2dev::RIGHT_ALT, "RAlt");
    }
  }
  // -- RSuper
  if (mod_last.RightSuper){
    if (!mod.RightSuper){
      // release
      _ps2->keyboard_release_special(PS2dev::RIGHT_WIN);
      _debug(false, (uint8_t)PS2dev::RIGHT_WIN, "RSuper");
    }
  }else{
    if (mod.RightSuper){
      // press
      _ps2->keyboard_press_special(PS2dev::RIGHT_WIN);
      _debug(true, (uint8_t)PS2dev::RIGHT_WIN, "RSuper");
    }
  }
  last_mod = recv[1];
  // ---- mod key process done
  // ---- key process
  // press
  for (int i=3; i<9; i++){
    if (recv[i] == 0) break;
    bool recv_in_old = false;
    for (int j=0; j<6; j++){
      if (recv[i] == last_key[j]){
        recv_in_old = true;
        break;
      }
      if (last_key[j]==0) break;
    }
    if (!recv_in_old){
      if (hid2ps2[recv[i]].type == PS2_KEY_TYPE_NORMAL){
        _ps2->keyboard_press(hid2ps2[recv[i]].key);
      }else{
        _ps2->keyboard_press_special(hid2ps2[recv[i]].key);
      }
      
      _debug(true, hid2ps2[recv[i]].key, hid2ps2[recv[i]].name);
    }
      
  }
  // release
  for (int i=0; i<6; i++){
    if(last_key[i] == 0) break;
    bool last_in_new = false;
    for (int j=3; j<9; j++){
      if (last_key[i] == recv[j]){
        last_in_new = true;
        break;
      }
      if (!last_in_new){
        if (hid2ps2[last_key[i]].type == PS2_KEY_TYPE_NORMAL){
          _ps2->keyboard_release(hid2ps2[last_key[i]].key);
        }else{
          _ps2->keyboard_release_special(hid2ps2[last_key[i]].key);
        }
        _debug(false, hid2ps2[last_key[i]].key, hid2ps2[last_key[i]].name);
        break;
      }
    }
  }
  // update last
  for (int i=3; i<9; i++){
    last_key[i-3] = recv[i];
  }
  // ---- mod key process done
}

void Ch9350::_ack_slave(){
  char cmd_ack_salve[] = {0x57, 0xab, 0x12, 0x00, 0x00, 0x00, 0x00, 0xff, 0x80, 0x00, 0x20};
  for (int i=0; i<11; i++)
    _ser->write(cmd_ack_salve[i]);
}
void Ch9350::_debug(bool press, uint8_t code, char* key){
  if (press){
    sprintf(prt_str, "press: %s, %x", key, code);
    Serial.println(prt_str);
  }else{
    sprintf(prt_str, "release: %s, %x", key, code);
    Serial.println(prt_str);
  }
}
void Ch9350::_init_hid2ps2() {
  last_mod = 0;
  for (int i=0;i<6;i++){
    last_key[i] = 0;
  }
  //

  // PS/2 normal scan code
  // | HID | PS/2 Make | PS/2 Break |
  // | --- | --------- | ---------- |
  // | 04  | 1C        | F0 1C      |

  // A-Z
  hid2ps2[0x04] = {0x1C, PS2_KEY_TYPE_NORMAL};
    strcpy(hid2ps2[0x04].name, "A");
  hid2ps2[0x05] = {0x32, PS2_KEY_TYPE_NORMAL};
    strcpy(hid2ps2[0x05].name, "B");
  hid2ps2[0x06] = {0x21, PS2_KEY_TYPE_NORMAL};
    strcpy(hid2ps2[0x06].name, "C");
  hid2ps2[0x07] = {0x23, PS2_KEY_TYPE_NORMAL};
    strcpy(hid2ps2[0x06].name, "D");
  hid2ps2[0x08] = {0x24, PS2_KEY_TYPE_NORMAL};
    strcpy(hid2ps2[0x06].name, "E");
  hid2ps2[0x09] = {0x2B, PS2_KEY_TYPE_NORMAL};
    strcpy(hid2ps2[0x06].name, "F");
  hid2ps2[0x0A] = {0x34, PS2_KEY_TYPE_NORMAL};
    strcpy(hid2ps2[0x06].name, "G");
  hid2ps2[0x0B] = {0x33, PS2_KEY_TYPE_NORMAL};
    strcpy(hid2ps2[0x06].name, "H");
  hid2ps2[0x0C] = {0x43, PS2_KEY_TYPE_NORMAL};
    strcpy(hid2ps2[0x06].name, "I");
  hid2ps2[0x0D] = {0x3B, PS2_KEY_TYPE_NORMAL};
    strcpy(hid2ps2[0x06].name, "J");
  hid2ps2[0x0E] = {0x42, PS2_KEY_TYPE_NORMAL};
    strcpy(hid2ps2[0x06].name, "K");
  hid2ps2[0x0F] = {0x4B, PS2_KEY_TYPE_NORMAL};
    strcpy(hid2ps2[0x06].name, "L");
  hid2ps2[0x10] = {0x3A, PS2_KEY_TYPE_NORMAL};
    strcpy(hid2ps2[0x06].name, "M");
  hid2ps2[0x11] = {0x31, PS2_KEY_TYPE_NORMAL};
    strcpy(hid2ps2[0x06].name, "N");
  hid2ps2[0x12] = {0x44, PS2_KEY_TYPE_NORMAL};
    strcpy(hid2ps2[0x06].name, "O");
  hid2ps2[0x13] = {0x4D, PS2_KEY_TYPE_NORMAL};
    strcpy(hid2ps2[0x06].name, "P");
  hid2ps2[0x14] = {0x15, PS2_KEY_TYPE_NORMAL};
    strcpy(hid2ps2[0x06].name, "Q");
  hid2ps2[0x15] = {0x2D, PS2_KEY_TYPE_NORMAL};
    strcpy(hid2ps2[0x06].name, "R");
  hid2ps2[0x16] = {0x1B, PS2_KEY_TYPE_NORMAL};
    strcpy(hid2ps2[0x06].name, "S");
  hid2ps2[0x17] = {0x2C, PS2_KEY_TYPE_NORMAL};
    strcpy(hid2ps2[0x06].name, "T");
  hid2ps2[0x18] = {0x3C, PS2_KEY_TYPE_NORMAL};
    strcpy(hid2ps2[0x06].name, "U");
  hid2ps2[0x19] = {0x2A, PS2_KEY_TYPE_NORMAL};
    strcpy(hid2ps2[0x06].name, "V");
  hid2ps2[0x1A] = {0x1D, PS2_KEY_TYPE_NORMAL};
    strcpy(hid2ps2[0x06].name, "W");
  hid2ps2[0x1B] = {0x22, PS2_KEY_TYPE_NORMAL};
    strcpy(hid2ps2[0x06].name, "X");
  hid2ps2[0x1C] = {0x35, PS2_KEY_TYPE_NORMAL};
    strcpy(hid2ps2[0x06].name, "Y");
  hid2ps2[0x1D] = {0x1A, PS2_KEY_TYPE_NORMAL};
    strcpy(hid2ps2[0x06].name, "Z");

  // 1-0
  hid2ps2[0x1E] = {0x16, PS2_KEY_TYPE_NORMAL};
    strcpy(hid2ps2[0x1E].name, "1");
  hid2ps2[0x1F] = {0x1E, PS2_KEY_TYPE_NORMAL};
    strcpy(hid2ps2[0x1F].name, "2");
  hid2ps2[0x20] = {0x26, PS2_KEY_TYPE_NORMAL};
    strcpy(hid2ps2[0x20].name, "3");
  hid2ps2[0x21] = {0x25, PS2_KEY_TYPE_NORMAL};
    strcpy(hid2ps2[0x21].name, "4");
  hid2ps2[0x22] = {0x2E, PS2_KEY_TYPE_NORMAL};
    strcpy(hid2ps2[0x22].name, "5");
  hid2ps2[0x23] = {0x36, PS2_KEY_TYPE_NORMAL};
    strcpy(hid2ps2[0x23].name, "6");
  hid2ps2[0x24] = {0x3D, PS2_KEY_TYPE_NORMAL};
    strcpy(hid2ps2[0x24].name, "7");
  hid2ps2[0x25] = {0x3E, PS2_KEY_TYPE_NORMAL};
    strcpy(hid2ps2[0x25].name, "8");
  hid2ps2[0x26] = {0x46, PS2_KEY_TYPE_NORMAL};
    strcpy(hid2ps2[0x26].name, "9");
  hid2ps2[0x27] = {0x45, PS2_KEY_TYPE_NORMAL};
    strcpy(hid2ps2[0x27].name, "0");

  hid2ps2[0x28] = {0x5A, PS2_KEY_TYPE_NORMAL};  // Return
    strcpy(hid2ps2[0x28].name, "ent");
  hid2ps2[0x29] = {0x76, PS2_KEY_TYPE_NORMAL};  // Escape
    strcpy(hid2ps2[0x29].name, "esc");
  hid2ps2[0x2A] = {0x66, PS2_KEY_TYPE_NORMAL};  // Backspace
    strcpy(hid2ps2[0x2A].name, "bsp");
  hid2ps2[0x2B] = {0x0D, PS2_KEY_TYPE_NORMAL};  // Tab
    strcpy(hid2ps2[0x2B].name, "tab");
  hid2ps2[0x2C] = {0x29, PS2_KEY_TYPE_NORMAL};  // Space
    strcpy(hid2ps2[0x2C].name, "sp");
  hid2ps2[0x2D] = {0x4E, PS2_KEY_TYPE_NORMAL};  // -_
    strcpy(hid2ps2[0x2D].name, "-_");
  hid2ps2[0x2E] = {0x55, PS2_KEY_TYPE_NORMAL};  // =+
    strcpy(hid2ps2[0x2E].name, "=+");
  hid2ps2[0x2F] = {0x54, PS2_KEY_TYPE_NORMAL};  // [{
    strcpy(hid2ps2[0x2F].name, "[{");
  hid2ps2[0x30] = {0x5B, PS2_KEY_TYPE_NORMAL};  // ]}
    strcpy(hid2ps2[0x30].name, "]}");
  hid2ps2[0x31] = {0x5D, PS2_KEY_TYPE_NORMAL};  // \|
    strcpy(hid2ps2[0x31].name, "|");

  hid2ps2[0x33] = {0x4C, PS2_KEY_TYPE_NORMAL};  // ;:
    strcpy(hid2ps2[0x33].name, ";");
  hid2ps2[0x34] = {0x52, PS2_KEY_TYPE_NORMAL};  // '"
    strcpy(hid2ps2[0x34].name, "'");
  hid2ps2[0x35] = {0x0E, PS2_KEY_TYPE_NORMAL};  // `~
    strcpy(hid2ps2[0x35].name, "~");
  hid2ps2[0x36] = {0x41, PS2_KEY_TYPE_NORMAL};  // ,<
    strcpy(hid2ps2[0x36].name, ",");
  hid2ps2[0x37] = {0x49, PS2_KEY_TYPE_NORMAL};  // ,>
    strcpy(hid2ps2[0x37].name, ".");
  hid2ps2[0x38] = {0x4A, PS2_KEY_TYPE_NORMAL};  // /?
    strcpy(hid2ps2[0x38].name, "?");
  hid2ps2[0x39] = {0x58, PS2_KEY_TYPE_NORMAL};  // Caps Lock
    strcpy(hid2ps2[0x39].name, "caps");

  // F1-F12
  hid2ps2[0x3A] = {0x05, PS2_KEY_TYPE_NORMAL};
    strcpy(hid2ps2[0x3A].name, "F1");
  hid2ps2[0x3B] = {0x06, PS2_KEY_TYPE_NORMAL};
    strcpy(hid2ps2[0x3B].name, "F2");
  hid2ps2[0x3C] = {0x04, PS2_KEY_TYPE_NORMAL};
    strcpy(hid2ps2[0x3C].name, "F3");
  hid2ps2[0x3D] = {0x0C, PS2_KEY_TYPE_NORMAL};
    strcpy(hid2ps2[0x3D].name, "F4");
  hid2ps2[0x3E] = {0x03, PS2_KEY_TYPE_NORMAL};
    strcpy(hid2ps2[0x3E].name, "F5");
  hid2ps2[0x3F] = {0x0B, PS2_KEY_TYPE_NORMAL};
    strcpy(hid2ps2[0x3F].name, "F6");
  hid2ps2[0x40] = {0x83, PS2_KEY_TYPE_NORMAL};
    strcpy(hid2ps2[0x40].name, "F7");
  hid2ps2[0x41] = {0x0A, PS2_KEY_TYPE_NORMAL};
    strcpy(hid2ps2[0x41].name, "F8");
  hid2ps2[0x42] = {0x01, PS2_KEY_TYPE_NORMAL};
    strcpy(hid2ps2[0x42].name, "F9");
  hid2ps2[0x43] = {0x09, PS2_KEY_TYPE_NORMAL};
    strcpy(hid2ps2[0x43].name, "F10");
  hid2ps2[0x44] = {0x78, PS2_KEY_TYPE_NORMAL};
    strcpy(hid2ps2[0x44].name, "F11");
  hid2ps2[0x45] = {0x07, PS2_KEY_TYPE_NORMAL};
    strcpy(hid2ps2[0x45].name, "F12");

  hid2ps2[0x47] = {0x7E, PS2_KEY_TYPE_NORMAL};
  strcpy(hid2ps2[0x47].name, "scro");

  hid2ps2[0x53] = {0x77, PS2_KEY_TYPE_NORMAL};
  strcpy(hid2ps2[0x53].name, "nmlk");

  hid2ps2[0x55] = {0x7C, PS2_KEY_TYPE_NORMAL};
  strcpy(hid2ps2[0x55].name, "nm*");
  hid2ps2[0x56] = {0x7B, PS2_KEY_TYPE_NORMAL};
  strcpy(hid2ps2[0x56].name, "nm-");
  hid2ps2[0x57] = {0x79, PS2_KEY_TYPE_NORMAL};
  strcpy(hid2ps2[0x57].name, "nm+");

  // 数字键盘1-9, 0
  hid2ps2[0x59] = {0x69, PS2_KEY_TYPE_NORMAL};
  strcpy(hid2ps2[0x59].name, "nm1");
  hid2ps2[0x5a] = {0x72, PS2_KEY_TYPE_NORMAL};
  strcpy(hid2ps2[0x5a].name, "nm2");
  hid2ps2[0x5b] = {0x7a, PS2_KEY_TYPE_NORMAL};
  strcpy(hid2ps2[0x5b].name, "nm3");
  hid2ps2[0x5c] = {0x6b, PS2_KEY_TYPE_NORMAL};
  strcpy(hid2ps2[0x5c].name, "nm4");
  hid2ps2[0x5d] = {0x73, PS2_KEY_TYPE_NORMAL};
  strcpy(hid2ps2[0x5d].name, "nm5");
  hid2ps2[0x5e] = {0x74, PS2_KEY_TYPE_NORMAL};
  strcpy(hid2ps2[0x5e].name, "nm6");
  hid2ps2[0x5f] = {0x6c, PS2_KEY_TYPE_NORMAL};
  strcpy(hid2ps2[0x5f].name, "nm7");
  hid2ps2[0x60] = {0x75, PS2_KEY_TYPE_NORMAL};
  strcpy(hid2ps2[0x60].name, "nm8");
  hid2ps2[0x61] = {0x7d, PS2_KEY_TYPE_NORMAL};
  strcpy(hid2ps2[0x61].name, "nm9");
  hid2ps2[0x62] = {0x70, PS2_KEY_TYPE_NORMAL};
  strcpy(hid2ps2[0x62].name, "nm0");

  hid2ps2[0x63] = {0x71, PS2_KEY_TYPE_NORMAL};
  strcpy(hid2ps2[0x63].name, "nm.");
  hid2ps2[0x67] = {0x0F, PS2_KEY_TYPE_NORMAL};
  strcpy(hid2ps2[0x67].name, "nm=");

  hid2ps2[0xE0] = {0x14, PS2_KEY_TYPE_NORMAL};
  strcpy(hid2ps2[0xE0].name, "LCtr");
  hid2ps2[0xE1] = {0x12, PS2_KEY_TYPE_NORMAL};
  strcpy(hid2ps2[0xE1].name, "Lshf");
  hid2ps2[0xE2] = {0x11, PS2_KEY_TYPE_NORMAL};
  strcpy(hid2ps2[0xE2].name, "LAlt");

  hid2ps2[0xE5] = {0x59, PS2_KEY_TYPE_NORMAL};
  strcpy(hid2ps2[0xE5].name, "Rshf");

  // PS/2 scan code prefix with E0
  // | HID | PS/2 Make | PS/2 Break |
  // | --- | --------- | ---------- |
  // | 46  | E0 7C     | E0 F0 7C   |
  hid2ps2[0x46] = {0x7C, PS2_KEY_TYPE_E0};
  strcpy(hid2ps2[0x46].name, "PrSc");

  hid2ps2[0x49] = {0x70, PS2_KEY_TYPE_E0};
  strcpy(hid2ps2[0x49].name, "ins");
  hid2ps2[0x4A] = {0x6C, PS2_KEY_TYPE_E0};
  strcpy(hid2ps2[0x4A].name, "home");
  hid2ps2[0x4B] = {0x7D, PS2_KEY_TYPE_E0};
  strcpy(hid2ps2[0x4B].name, "pgup");
  hid2ps2[0x4C] = {0x71, PS2_KEY_TYPE_E0};
  strcpy(hid2ps2[0x4C].name, "del");
  hid2ps2[0x4D] = {0x69, PS2_KEY_TYPE_E0};
  strcpy(hid2ps2[0x4D].name, "end");
  hid2ps2[0x4E] = {0x7A, PS2_KEY_TYPE_E0};
  strcpy(hid2ps2[0x4E].name, "pgdn");
  hid2ps2[0x4F] = {0x74, PS2_KEY_TYPE_E0};
  strcpy(hid2ps2[0x4F].name, "rigt");
  hid2ps2[0x50] = {0x6B, PS2_KEY_TYPE_E0};
  strcpy(hid2ps2[0x50].name, "left");
  hid2ps2[0x51] = {0x72, PS2_KEY_TYPE_E0};
  strcpy(hid2ps2[0x51].name, "down");
  hid2ps2[0x52] = {0x75, PS2_KEY_TYPE_E0};
  strcpy(hid2ps2[0x52].name, "up");

  hid2ps2[0x54] = {0x4A, PS2_KEY_TYPE_E0};
  strcpy(hid2ps2[0x54].name, "nm/");

  hid2ps2[0x58] = {0x5A, PS2_KEY_TYPE_E0};
  strcpy(hid2ps2[0x58].name, "nmEt");

  hid2ps2[0x65] = {0x2F, PS2_KEY_TYPE_E0};  // App

  hid2ps2[0xE3] = {0x1F, PS2_KEY_TYPE_E0};  // Left GUI
  hid2ps2[0xE4] = {0x14, PS2_KEY_TYPE_E0};  // Right Control

  hid2ps2[0xE6] = {0x11, PS2_KEY_TYPE_E0};  // Right Alt
  hid2ps2[0xE7] = {0x27, PS2_KEY_TYPE_E0};  // Right GUI

  // PS/2 scan code prefix with E1
  // | HID | PS/2 Make               | PS/2 Break |
  // | --- | ----------------------- | ---------- |
  // | 48  | E1 14 77 E1 F0 14 F0 77 | None       |
  hid2ps2[0x48] = {0x0, PS2_KEY_TYPE_E1};  // Pause/Break
}
