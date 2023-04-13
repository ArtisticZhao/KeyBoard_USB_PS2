#include "esp_log.h"
#include "fifo.h"


extern QueueHandle_t key_event_fifo;


void put_key(const Ps2Key* key, uint8_t status) {
    keyEvent new_key_event;
    new_key_event.ps2_keycode = key->key;
    new_key_event.ps2_keytype = key->type;
    new_key_event.status = status;
    if( xQueueSendToBack(key_event_fifo, &new_key_event, 10) != pdPASS ) {
        ESP_LOGI("FIFO", "FIFO put failed!");
    }
}


void get_key(const Ps2Key* key, uint8_t status) {

}
