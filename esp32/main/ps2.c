#include "ps2.h"
#include <string.h>
#include "esp_log.h"

#include "fifo.h"

static Ps2Key hid2ps2[0xE8];  // 定义hid编码到ps2编码的查找表


void parser_hid(const uint8_t* in) {
    static uint8_t last_key[6] = { 0 };  // USB 键盘最多6键按下
    static uint8_t mod_last = 0;         // mod key 上一次的值
    // ------ 处理mod key
    uint8_t mod_key = in[1];
    for (size_t i = 0; i < 8 ; i++) {
        uint8_t key = mod_key & (0x01 << i);
        uint8_t last_mod_key = mod_last & (0x01 << i);
        if (key != 0 && last_mod_key == 0) {
            // mod key press
            put_key(&hid2ps2[0xE0+i], KEY_PRESS);
        }
        else if (key == 0 && last_mod_key != 0) {
            // mod key press
            put_key(&hid2ps2[0xE0+i], KEY_RELEASE);
        }
    }
    mod_last = mod_key;
    // ------ 处理正常按键
    const uint8_t* normal_key = in+3;
    for (uint8_t i = 0; i < 6; i++) {
        if (normal_key[i] != 0 && last_key[i] == 0) {
            // key press
            put_key(&hid2ps2[normal_key[i]], KEY_PRESS);
        }
        else if (normal_key[i] == 0 && last_key[i] != 0) {
            // key release
            put_key(&hid2ps2[last_key[i]], KEY_RELEASE);
        }
    }
    // all done update last_key
    for (uint8_t i = 0; i < 6; ++i) {
        last_key[i] = in[i+3];
    }
}




void _debug(Ps2Key* key, uint8_t mode) {
#ifdef __DEBUG
    if (mode) {
        ESP_LOGI("key_parser", "%s: pressed", key->name);
    }else{
        ESP_LOGI("key_parser", "%s: released", key->name);
    }
#endif
}




// 设置一个结构体内容
static void set_ps2key(uint8_t hid, uint8_t ps2key, uint8_t key_type, const char* name) {
    Ps2Key* ptmp = hid2ps2 + hid;
    ptmp->key = ps2key;
    ptmp->type = key_type;
    strcpy(ptmp->name, name);
}


void init_table() {
    // A-Z
    set_ps2key( 4, 0x1C, PS2_KEY_TYPE_NORMAL, "A");
    set_ps2key( 5, 0x32, PS2_KEY_TYPE_NORMAL, "B");
    set_ps2key( 6, 0x21, PS2_KEY_TYPE_NORMAL, "C");
    set_ps2key( 7, 0x23, PS2_KEY_TYPE_NORMAL, "D");
    set_ps2key( 8, 0x24, PS2_KEY_TYPE_NORMAL, "E");
    set_ps2key( 9, 0x2B, PS2_KEY_TYPE_NORMAL, "F");
    set_ps2key(10, 0x34, PS2_KEY_TYPE_NORMAL, "G");
    set_ps2key(11, 0x33, PS2_KEY_TYPE_NORMAL, "H");
    set_ps2key(12, 0x43, PS2_KEY_TYPE_NORMAL, "I");
    set_ps2key(13, 0x3B, PS2_KEY_TYPE_NORMAL, "J");
    set_ps2key(14, 0x42, PS2_KEY_TYPE_NORMAL, "K");
    set_ps2key(15, 0x4B, PS2_KEY_TYPE_NORMAL, "L");
    set_ps2key(16, 0x3A, PS2_KEY_TYPE_NORMAL, "M");
    set_ps2key(17, 0x31, PS2_KEY_TYPE_NORMAL, "N");
    set_ps2key(18, 0x44, PS2_KEY_TYPE_NORMAL, "O");
    set_ps2key(19, 0x4D, PS2_KEY_TYPE_NORMAL, "P");
    set_ps2key(20, 0x15, PS2_KEY_TYPE_NORMAL, "Q");
    set_ps2key(21, 0x2D, PS2_KEY_TYPE_NORMAL, "R");
    set_ps2key(22, 0x1B, PS2_KEY_TYPE_NORMAL, "S");
    set_ps2key(23, 0x2C, PS2_KEY_TYPE_NORMAL, "T");
    set_ps2key(24, 0x3C, PS2_KEY_TYPE_NORMAL, "U");
    set_ps2key(25, 0x2A, PS2_KEY_TYPE_NORMAL, "V");
    set_ps2key(26, 0x1D, PS2_KEY_TYPE_NORMAL, "W");
    set_ps2key(27, 0x22, PS2_KEY_TYPE_NORMAL, "X");
    set_ps2key(28, 0x35, PS2_KEY_TYPE_NORMAL, "Y");
    set_ps2key(29, 0x1A, PS2_KEY_TYPE_NORMAL, "Z");
    // 1-0
    set_ps2key(30, 0x16, PS2_KEY_TYPE_NORMAL, "1");
    set_ps2key(31, 0x1E, PS2_KEY_TYPE_NORMAL, "2");
    set_ps2key(32, 0x26, PS2_KEY_TYPE_NORMAL, "3");
    set_ps2key(33, 0x25, PS2_KEY_TYPE_NORMAL, "4");
    set_ps2key(34, 0x2E, PS2_KEY_TYPE_NORMAL, "5");
    set_ps2key(35, 0x36, PS2_KEY_TYPE_NORMAL, "6");
    set_ps2key(36, 0x3D, PS2_KEY_TYPE_NORMAL, "7");
    set_ps2key(37, 0x3E, PS2_KEY_TYPE_NORMAL, "8");
    set_ps2key(38, 0x46, PS2_KEY_TYPE_NORMAL, "9");
    set_ps2key(39, 0x45, PS2_KEY_TYPE_NORMAL, "0");

    set_ps2key(0x28, 0x5A, PS2_KEY_TYPE_NORMAL, "Enter");
    set_ps2key(0x29, 0x76, PS2_KEY_TYPE_NORMAL, "Esc");
    set_ps2key(0x2A, 0x66, PS2_KEY_TYPE_NORMAL, "Bsp");
    set_ps2key(0x2B, 0x0D, PS2_KEY_TYPE_NORMAL, "Tab");
    set_ps2key(0x2C, 0x29, PS2_KEY_TYPE_NORMAL, "Space");
    set_ps2key(0x2D, 0x4E, PS2_KEY_TYPE_NORMAL, "-_");
    set_ps2key(0x2E, 0x55, PS2_KEY_TYPE_NORMAL, "=+");
    set_ps2key(0x2F, 0x54, PS2_KEY_TYPE_NORMAL, "[{");
    set_ps2key(0x30, 0x5B, PS2_KEY_TYPE_NORMAL, "]}");
    set_ps2key(0x31, 0x5D, PS2_KEY_TYPE_NORMAL, "\\|");

    set_ps2key(0x33, 0x4C, PS2_KEY_TYPE_NORMAL, ";:");
    set_ps2key(0x34, 0x52, PS2_KEY_TYPE_NORMAL, "'\"");
    set_ps2key(0x35, 0x0E, PS2_KEY_TYPE_NORMAL, "`~");
    set_ps2key(0x36, 0x41, PS2_KEY_TYPE_NORMAL, ",<");
    set_ps2key(0x37, 0x49, PS2_KEY_TYPE_NORMAL, ".>");
    set_ps2key(0x38, 0x4A, PS2_KEY_TYPE_NORMAL, "/?");
    set_ps2key(0x39, 0x58, PS2_KEY_TYPE_NORMAL, "CapsLock");

    // F1-F12
    set_ps2key(58, 0x05, PS2_KEY_TYPE_NORMAL, "F1");
    set_ps2key(59, 0x06, PS2_KEY_TYPE_NORMAL, "F2");
    set_ps2key(60, 0x04, PS2_KEY_TYPE_NORMAL, "F3");
    set_ps2key(61, 0x0C, PS2_KEY_TYPE_NORMAL, "F4");
    set_ps2key(62, 0x03, PS2_KEY_TYPE_NORMAL, "F5");
    set_ps2key(63, 0x0B, PS2_KEY_TYPE_NORMAL, "F6");
    set_ps2key(64, 0x83, PS2_KEY_TYPE_NORMAL, "F7");
    set_ps2key(65, 0x0A, PS2_KEY_TYPE_NORMAL, "F8");
    set_ps2key(66, 0x01, PS2_KEY_TYPE_NORMAL, "F9");
    set_ps2key(67, 0x09, PS2_KEY_TYPE_NORMAL, "F10");
    set_ps2key(68, 0x78, PS2_KEY_TYPE_NORMAL, "F11");
    set_ps2key(69, 0x07, PS2_KEY_TYPE_NORMAL, "F12");

    set_ps2key(71, 0x7E, PS2_KEY_TYPE_NORMAL, "ScrLk");

    // num pad
    set_ps2key(0x53, 0x77, PS2_KEY_TYPE_NORMAL, "NumLk");
    set_ps2key(0x54, 0x4A, PS2_KEY_TYPE_E0,     "Num/");
    set_ps2key(0x55, 0x7C, PS2_KEY_TYPE_NORMAL, "Num*");
    set_ps2key(0x56, 0x7B, PS2_KEY_TYPE_NORMAL, "Num-");
    set_ps2key(0x57, 0x79, PS2_KEY_TYPE_NORMAL, "Num+");
    set_ps2key(0x58, 0x5A, PS2_KEY_TYPE_E0,     "NumEnt");
    // num pad 1-0
    set_ps2key(89, 0x69, PS2_KEY_TYPE_NORMAL, "Num1");
    set_ps2key(90, 0x72, PS2_KEY_TYPE_NORMAL, "Num2");
    set_ps2key(91, 0x7a, PS2_KEY_TYPE_NORMAL, "Num3");
    set_ps2key(92, 0x6b, PS2_KEY_TYPE_NORMAL, "Num4");
    set_ps2key(93, 0x73, PS2_KEY_TYPE_NORMAL, "Num5");
    set_ps2key(94, 0x74, PS2_KEY_TYPE_NORMAL, "Num6");
    set_ps2key(95, 0x6c, PS2_KEY_TYPE_NORMAL, "Num7");
    set_ps2key(96, 0x75, PS2_KEY_TYPE_NORMAL, "Num8");
    set_ps2key(97, 0x7d, PS2_KEY_TYPE_NORMAL, "Num9");
    set_ps2key(98, 0x70, PS2_KEY_TYPE_NORMAL, "Num0");
    set_ps2key(99, 0x63, PS2_KEY_TYPE_NORMAL, "Num.");

    set_ps2key(0xE0, 0x14, PS2_KEY_TYPE_NORMAL, "LCtrl");
    set_ps2key(0xE1, 0x12, PS2_KEY_TYPE_NORMAL, "LShift");
    set_ps2key(0xE2, 0x11, PS2_KEY_TYPE_NORMAL, "LAlt");
    set_ps2key(0xE3, 0x1F, PS2_KEY_TYPE_E0, "LGUI");
    set_ps2key(0xE4, 0x14, PS2_KEY_TYPE_E0, "RCtrl");
    set_ps2key(0xE5, 0x59, PS2_KEY_TYPE_E0, "RShift");
    set_ps2key(0xE6, 0x11, PS2_KEY_TYPE_E0, "RAlt");
    set_ps2key(0xE7, 0x27, PS2_KEY_TYPE_E0, "RGUI");

    set_ps2key(0x46, 0x7C, PS2_KEY_TYPE_E0, "PrtSc");

    set_ps2key(0x49, 0x70, PS2_KEY_TYPE_E0, "Ins");
    set_ps2key(0x4A, 0x6C, PS2_KEY_TYPE_E0, "Home");
    set_ps2key(0x4B, 0x7D, PS2_KEY_TYPE_E0, "PgUp");
    set_ps2key(0x4C, 0x71, PS2_KEY_TYPE_E0, "Del");
    set_ps2key(0x4D, 0x69, PS2_KEY_TYPE_E0, "End");
    set_ps2key(0x4E, 0x7A, PS2_KEY_TYPE_E0, "PgDown");
    set_ps2key(0x4F, 0x74, PS2_KEY_TYPE_E0, "Right");
    set_ps2key(0x50, 0x6B, PS2_KEY_TYPE_E0, "Left");
    set_ps2key(0x51, 0x72, PS2_KEY_TYPE_E0, "Donw");
    set_ps2key(0x52, 0x75, PS2_KEY_TYPE_E0, "Up");

    set_ps2key(0x65, 0x2F, PS2_KEY_TYPE_E0, "App");
}
