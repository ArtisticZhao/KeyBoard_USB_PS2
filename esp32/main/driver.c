#include <unistd.h>
#include "driver.h"
#include "driver/gpio.h"
#include "esp_log.h"

//since for the device side we are going to be in charge of the clock,
//the two defines below are how long each _phase_ of the clock cycle is
#define M_CLKFULL 40UL
// we make changes in the middle of a phase, this how long from the
// start of phase to the when we drive the data line
#define M_CLKHALF 20UL

// Delay between bytes
// I've found i need at least 400us to get this working at all,
// but even more is needed for reliability, so i've put 1000us
#define M_BYTEWAIT 1000UL

// Timeout if computer not sending for 30ms
#define M_TIMEOUT 30UL

#define CLKFULL 40
#define CLKHALF 20
#define BYTEWAIT 1000
#define TIMEOUT 30

// const TickType_t CLKFULL = pdMS_TO_TICKS( M_CLKFULL );
// const TickType_t CLKHALF = pdMS_TO_TICKS( M_CLKHALF );
// const TickType_t BYTEWAIT = pdMS_TO_TICKS( M_BYTEWAIT );
// const TickType_t TIMEOUT = pdMS_TO_TICKS( M_TIMEOUT );

/*#define gohi(pin) gpio_set_level(pin, 1)*/
/*#define golo(pin) gpio_set_level(pin, 0)*/
#define delayMicroseconds(time) usleep(time)
#define digitalRead(pin) gpio_get_level(pin)
#define LOW 0
#define HIGH 1

static void gohi(uint32_t pin) {
    gpio_set_direction(pin, GPIO_MODE_INPUT);
    gpio_pullup_en(pin);
}

static void golo(uint32_t pin) {
    gpio_set_direction(pin, GPIO_MODE_OUTPUT);
    gpio_set_level(pin, 0);

}

int8_t ps2_write(uint8_t data) {
    delayMicroseconds(BYTEWAIT);

    unsigned char i;
    unsigned char parity = 1;

    if (digitalRead(_ps2clk) == LOW) {
        return -1;
    }

    if (digitalRead(_ps2data) == LOW) {
        return -2;
    }

    golo(_ps2data);
    delayMicroseconds(CLKHALF);
    // device sends on falling clock
    golo(_ps2clk);	// start bit
    delayMicroseconds(CLKFULL);
    gohi(_ps2clk);
    delayMicroseconds(CLKHALF);

    for (i=0; i < 8; i++)
    {
        if (data & 0x01)
        {
            gohi(_ps2data);
        } else {
            golo(_ps2data);
        }
        delayMicroseconds(CLKHALF);
        golo(_ps2clk);
        delayMicroseconds(CLKFULL);
        gohi(_ps2clk);
        delayMicroseconds(CLKHALF);

        parity = parity ^ (data & 0x01);
        data = data >> 1;
    }
    // parity bit
    if (parity)
    {
        gohi(_ps2data);
    } else {
        golo(_ps2data);
    }
    delayMicroseconds(CLKHALF);
    golo(_ps2clk);
    delayMicroseconds(CLKFULL);
    gohi(_ps2clk);
    delayMicroseconds(CLKHALF);

    // stop bit
    gohi(_ps2data);
    delayMicroseconds(CLKHALF);
    golo(_ps2clk);
    delayMicroseconds(CLKFULL);
    gohi(_ps2clk);
    delayMicroseconds(CLKHALF);

    delayMicroseconds(BYTEWAIT);

    return 0;
}

static keyEvent last_key;
void sim_key(keyEvent* key) {
    last_key = *key;
    if (key->ps2_keytype == PS2_KEY_TYPE_NORMAL) {
        if (key->status) {
            // key_press
            ps2_write(key->ps2_keycode);
        }
        else {
            ps2_write(0xF0);
            ps2_write(key->ps2_keycode);
        }
    }
    else {
        if (key->status) {
            // key_press
            ps2_write(0xE0);
            ps2_write(key->ps2_keycode);
        }
        else {
            ps2_write(0xE0);
            ps2_write(0xF0);
            ps2_write(key->ps2_keycode);
        }
    }
}

uint8_t ps2_available() {
    return ( (gpio_get_level(_ps2data) == 0) || (gpio_get_level(_ps2clk) == 0) );
}

uint8_t ps2_read(uint8_t* value){
    uint32_t data = 0x00;
    uint32_t bit = 0x01;
    uint8_t calculated_parity = 1;
    uint8_t recv_parity = 0;
    uint8_t timeout_cnt = 0;
    while ((digitalRead(_ps2data) != LOW) || (digitalRead(_ps2clk) != HIGH)) {
        usleep(1);
        timeout_cnt++;
        if (timeout_cnt > TIMEOUT) {
            return 33;
        }
    }

    delayMicroseconds(CLKHALF);
    golo(_ps2clk);
    delayMicroseconds(CLKFULL);
    gohi(_ps2clk);
    delayMicroseconds(CLKHALF);

    while (bit < 0x0100) {
        if (digitalRead(_ps2data) == HIGH)
        {
            data = data | bit;
            calculated_parity = calculated_parity ^ 1;
        } else {
            calculated_parity = calculated_parity ^ 0;
        }

        bit = bit << 1;

        delayMicroseconds(CLKHALF);
        golo(_ps2clk);
        delayMicroseconds(CLKFULL);
        gohi(_ps2clk);
        delayMicroseconds(CLKHALF);

    }
    // we do the delay at the end of the loop, so at this point we have
    // already done the delay for the parity bit

    // parity bit
    if (digitalRead(_ps2data) == HIGH)
    {
        recv_parity = 1;
    }

    // stop bit
    delayMicroseconds(CLKHALF);
    golo(_ps2clk);
    delayMicroseconds(CLKFULL);
    gohi(_ps2clk);
    delayMicroseconds(CLKHALF);


    delayMicroseconds(CLKHALF);
    golo(_ps2data);
    golo(_ps2clk);
    delayMicroseconds(CLKFULL);
    gohi(_ps2clk);
    delayMicroseconds(CLKHALF);
    gohi(_ps2data);


    *value = data & 0x00FF;

#ifdef _PS2DBG
    ESP_LOGI("ps2_read", "ps2 read: %x", *value);
    if (recv_parity != calculated_parity) {
        ESP_LOGI("ps2_read", "recv parity: %x", recv_parity);
        ESP_LOGI("ps2_read", "calac parity: %x", calculated_parity);
    }
#endif
    if (recv_parity == calculated_parity) {
        return 0;
    } else {
        return 99;
    }
}


void init_io() {
    //zero-initialize the config structure.
    gpio_config_t io_conf = {};
    //disable interrupt
    io_conf.intr_type = GPIO_INTR_DISABLE;
    //set as output mode
    io_conf.mode = GPIO_MODE_OUTPUT;
    //bit mask of the pins that you want to set,e.g.GPIO18/19
    io_conf.pin_bit_mask = GPIO_OUTPUT_PIN_SEL;
    //disable pull-down mode
    io_conf.pull_down_en = 0;
    //disable pull-up mode
    io_conf.pull_up_en = 0;
    //configure GPIO with the given settings
    gpio_config(&io_conf);

    gohi(PS2_CLK);
    gohi(PS2_DATA);

    ESP_LOGI("driver", "test delay");
    delayMicroseconds(BYTEWAIT);
    delayMicroseconds(CLKHALF);
    delayMicroseconds(CLKFULL);
    delayMicroseconds(TIMEOUT);
    ESP_LOGI("driver", "test delay ok");
}


void init_keyboard() {
    ESP_LOGI("ps2_init", "init kbk");
    while (ps2_write(0xAA) != 0) {
#ifdef _PS2DBG
    ESP_LOGI("ps2_init", "init kbk fail retry");
#endif
    }
}

static void ps2_ack(){
    while (ps2_write(0xFA)!=0) {
#ifdef _PS2DBG
    ESP_LOGI("ps2_ack", "ack fail retry");
#endif

    }
}

static uint8_t ps2_keyboard_reply(uint8_t cmd) {
    uint8_t val=0;
    switch (cmd) {
    case 0xFF: //reset
        ps2_ack();
        //the while loop lets us wait for the host to be ready
        while(ps2_write(0xAA)!=0);
        break;
    case 0xFE: //resend
        sim_key(&last_key);
        break;
    case 0xF6: //set defaults
        //enter stream mode
        ps2_ack();
        break;
    case 0xF5: //disable data reporting
        //FM
        ps2_ack();
        break;
    case 0xF4: //enable data reporting
        //FM
        ps2_ack();
        break;
    case 0xF3: //set typematic rate
        ps2_ack();
        if(!ps2_read(&val)) ps2_ack(); //do nothing with the rate
        break;
    case 0xF2: //get device id
        ps2_ack();
        ps2_write(0xAB);
        ps2_write(0x83);
        break;
    case 0xF0: //set scan code set
        ps2_ack();
        if(!ps2_read(&val)) ps2_ack(); //do nothing with the rate
        break;
    case 0xEE: //echo
        //ps2_ack();
        ps2_write(0xEE);
        break;
    case 0xED: //set/reset LEDs
        ps2_ack();
        if(!ps2_read(&val)) ps2_ack(); //do nothing with the rate
        break;
    }
#ifdef _PS2DBG
    if (cmd != 0) {
        ESP_LOGI("ps2_reply", "cmd=%x, val=%x", cmd, val);
    }
#endif
    return val;
}

void is_idle() {
    uint8_t c;
    if (ps2_available()) {
        ps2_read(&c);
        ps2_keyboard_reply(c);
    }
}

