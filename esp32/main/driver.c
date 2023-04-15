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

#define gohi(pin) gpio_set_level(pin, 1)
#define golo(pin) gpio_set_level(pin, 0)
#define delayMicroseconds(time) usleep(time)

void ps2_write(uint8_t data) {
    delayMicroseconds(BYTEWAIT);

    unsigned char i;
    unsigned char parity = 1;

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
}

void key_press(uint8_t keycode, uint8_t mode) {
    if (mode == PS2_KEY_TYPE_NORMAL) {

    }

}

void sim_key(keyEvent* key) {
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
void is_idle() {
    // 1. set to input
    gpio_set_direction(PS2_CLK, GPIO_MODE_INPUT);
    gpio_set_direction(PS2_DATA, GPIO_MODE_INPUT);
    if (ps2_available()) {
        ESP_LOGI("drive", "ps2 available!");
    }
    gpio_set_direction(PS2_CLK, GPIO_MODE_OUTPUT);
    gpio_set_direction(PS2_DATA, GPIO_MODE_OUTPUT);
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
