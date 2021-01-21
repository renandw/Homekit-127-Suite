#include <stdio.h>
#include <stdlib.h>
#include <espressif/esp_wifi.h>
#include <espressif/esp_sta.h>
#include <espressif/esp_common.h>
#include <esp/uart.h>
#include <esp8266.h>
#include <FreeRTOS.h>
#include <task.h>
#include <queue.h>
#include <string.h>

#include <homekit/homekit.h>
#include <homekit/characteristics.h>
#include <wifi_config.h>

#include <button.h>
#include <toggle.h>
#include "ota-api.h"

// The GPIO pin that is connected to RELAY#1 on the board.
const int relay_gpio_1 = 0;
// The GPIO pin that is connected to RELAY#2 on the board.
const int relay_gpio_2 = 2;
// The GPIO pin that is connected to RELAY#3 on the board.
const int relay_gpio_3 = 13;
// The GPIO pin that is connected to RELAY#4 on the board.
const int relay_gpio_4 = 16;

// The GPIO pin that is connected to the button on the Board.
#define BUTTON_PIN 3
#ifndef BUTTON_PIN
#error BUTTON_PIN is not specified
#endif

// The GPIO pin that is connected to the header on the board(external switch).
#define TOGGLE_PIN_1 4
#ifndef TOGGLE_PIN_1
#error TOGGLE_PIN_1 is not specified
#endif

#define TOGGLE_PIN_2 5
#ifndef TOGGLE_PIN_2
#error TOGGLE_PIN_2 is not specified
#endif

#define TOGGLE_PIN_3 12
#ifndef TOGGLE_PIN_3
#error TOGGLE_PIN_3 is not specified
#endif

#define TOGGLE_PIN_4 14
#ifndef TOGGLE_PIN_4
#error TOGGLE_PIN_4 is not specified
#endif



void lightbulb_on_1_callback(homekit_characteristic_t *_ch, homekit_value_t on, void *context);
void lightbulb_on_2_callback(homekit_characteristic_t *_ch, homekit_value_t on, void *context);
void lightbulb_on_3_callback(homekit_characteristic_t *_ch, homekit_value_t on, void *context);
void lightbulb_on_4_callback(homekit_characteristic_t *_ch, homekit_value_t on, void *context);


void relay_write_1(bool on) {
    gpio_write(relay_gpio_1, on ? 0 : 1);
}

void relay_write_2(bool on) {
    gpio_write(relay_gpio_2, on ? 0 : 1);
}

void relay_write_3(bool on) {
    gpio_write(relay_gpio_3, on ? 0 : 1);
}

void relay_write_4(bool on) {
    gpio_write(relay_gpio_4, on ? 0 : 1);
}


void reset_configuration_task() {
    printf("Resetting Wifi due to button long at GPIO %2d\n", BUTTON_PIN);
    wifi_config_reset();
    vTaskDelay(1000 / portTICK_PERIOD_MS);
    printf("Resetting HomeKit Config\n");
    homekit_server_reset();
    vTaskDelay(1000 / portTICK_PERIOD_MS);
    printf("Restarting\n");
    sdk_system_restart();
    vTaskDelete(NULL);
}

void reset_configuration() {
    printf("Resetting ESP12F configuration\n");
    xTaskCreate(reset_configuration_task, "Reset configuration", 256, NULL, 2, NULL);
}

homekit_characteristic_t lightbulb_on_1 = HOMEKIT_CHARACTERISTIC_(
    ON, true, .callback=HOMEKIT_CHARACTERISTIC_CALLBACK(lightbulb_on_1_callback)
);

homekit_characteristic_t lightbulb_on_2 = HOMEKIT_CHARACTERISTIC_(
    ON, true, .callback=HOMEKIT_CHARACTERISTIC_CALLBACK(lightbulb_on_2_callback)
);

homekit_characteristic_t lightbulb_on_3 = HOMEKIT_CHARACTERISTIC_(
    ON, true, .callback=HOMEKIT_CHARACTERISTIC_CALLBACK(lightbulb_on_3_callback)
);

homekit_characteristic_t lightbulb_on_4 = HOMEKIT_CHARACTERISTIC_(
    ON, true, .callback=HOMEKIT_CHARACTERISTIC_CALLBACK(lightbulb_on_4_callback)
);


void gpio_init() {
    gpio_enable(relay_gpio_1, GPIO_OUTPUT);
    relay_write_1(lightbulb_on_1.value.bool_value);

    gpio_enable(relay_gpio_2, GPIO_OUTPUT);
    relay_write_2(lightbulb_on_2.value.bool_value);

    gpio_enable(relay_gpio_3, GPIO_OUTPUT);
    relay_write_3(lightbulb_on_3.value.bool_value);

    gpio_enable(relay_gpio_4, GPIO_OUTPUT);
    relay_write_4(lightbulb_on_4.value.bool_value);

    gpio_enable(BUTTON_PIN,  GPIO_INPUT);
    gpio_enable(TOGGLE_PIN_1, GPIO_INPUT);
    gpio_enable(TOGGLE_PIN_2, GPIO_INPUT);
    gpio_enable(TOGGLE_PIN_3, GPIO_INPUT);
    gpio_enable(TOGGLE_PIN_4, GPIO_INPUT);
}

void lightbulb_on_1_callback(homekit_characteristic_t *_ch, homekit_value_t on, void *context) {
    relay_write_1(lightbulb_on_1.value.bool_value);
}

void lightbulb_on_2_callback(homekit_characteristic_t *_ch, homekit_value_t on, void *context) {
    relay_write_2(lightbulb_on_2.value.bool_value);
}

void lightbulb_on_3_callback(homekit_characteristic_t *_ch, homekit_value_t on, void *context) {
    relay_write_3(lightbulb_on_3.value.bool_value);
}

void lightbulb_on_4_callback(homekit_characteristic_t *_ch, homekit_value_t on, void *context) {
    relay_write_4(lightbulb_on_4.value.bool_value);
}


void button_callback(button_event_t event, void* context) {
    switch (event) {
        case button_event_long_press:
            reset_configuration();
            break;
        default:
            printf("Unknown button event: %d\n", event);
    }
}

void toggle_callback_1(bool high, void *context) {
    printf("toggle is %s\n", high ? "high" : "low");
    lightbulb_on_1.value.bool_value = !lightbulb_on_1.value.bool_value;
    relay_write_1(lightbulb_on_1.value.bool_value);
    homekit_characteristic_notify(&lightbulb_on_1, lightbulb_on_1.value);
}

void toggle_callback_2(bool high, void *context) {
    printf("toggle is %s\n", high ? "high" : "low");
    lightbulb_on_2.value.bool_value = !lightbulb_on_2.value.bool_value;
    relay_write_2(lightbulb_on_2.value.bool_value);
    homekit_characteristic_notify(&lightbulb_on_2, lightbulb_on_2.value);
}

void toggle_callback_3(bool high, void *context) {
    printf("toggle is %s\n", high ? "high" : "low");
    lightbulb_on_3.value.bool_value = !lightbulb_on_3.value.bool_value;
    relay_write_3(lightbulb_on_3.value.bool_value);
    homekit_characteristic_notify(&lightbulb_on_3, lightbulb_on_3.value);
}

void toggle_callback_4(bool high, void *context) {
    printf("toggle is %s\n", high ? "high" : "low");
    lightbulb_on_4.value.bool_value = !lightbulb_on_4.value.bool_value;
    relay_write_4(lightbulb_on_4.value.bool_value);
    homekit_characteristic_notify(&lightbulb_on_4, lightbulb_on_4.value);
}

void light_identify(homekit_value_t _value) {
    printf("Light identify\n");
}

homekit_characteristic_t name = HOMEKIT_CHARACTERISTIC_(NAME, "5switch");
homekit_characteristic_t ota_trigger  = API_OTA_TRIGGER;
homekit_characteristic_t manufacturer = HOMEKIT_CHARACTERISTIC_(MANUFACTURER,  "X");
homekit_characteristic_t serial       = HOMEKIT_CHARACTERISTIC_(SERIAL_NUMBER, "1");
homekit_characteristic_t model        = HOMEKIT_CHARACTERISTIC_(MODEL,         "Z");
homekit_characteristic_t revision     = HOMEKIT_CHARACTERISTIC_(FIRMWARE_REVISION,  "0.0.0");

homekit_accessory_t *accessories[] = {
    HOMEKIT_ACCESSORY(
          .id=1,
          .category=homekit_accessory_category_switch,
          .services=(homekit_service_t*[]){
            HOMEKIT_SERVICE(ACCESSORY_INFORMATION, .characteristics=(homekit_characteristic_t*[]){
            HOMEKIT_CHARACTERISTIC(IDENTIFY, light_identify),
            &name,
            &manufacturer,
            &serial,
            &model,
            &revision,
            NULL
        }),

        HOMEKIT_SERVICE(LIGHTBULB, .primary=true, .characteristics=(homekit_characteristic_t*[]){
	         HOMEKIT_CHARACTERISTIC(NAME, "Lâmpada 1"),
	          &lightbulb_on_1,
            &ota_trigger,
            NULL
        }),
        NULL,
      }),

    HOMEKIT_ACCESSORY(
          .id=2,
          .category=homekit_accessory_category_switch,
          .services=(homekit_service_t*[]){
            HOMEKIT_SERVICE(ACCESSORY_INFORMATION, .characteristics=(homekit_characteristic_t*[]){
            HOMEKIT_CHARACTERISTIC(IDENTIFY, light_identify),
            &name,  
            &manufacturer,
            &serial,
            &model,
            &revision,
            NULL
        }),  

        HOMEKIT_SERVICE(LIGHTBULB, .characteristics=(homekit_characteristic_t*[]){
            HOMEKIT_CHARACTERISTIC(NAME, "Lâmpada 2"),
            &lightbulb_on_2,
            NULL
        }),
        NULL,
      }),
      
      HOMEKIT_ACCESSORY(
            .id=3,
            .category=homekit_accessory_category_switch,
            .services=(homekit_service_t*[]){
              HOMEKIT_SERVICE(ACCESSORY_INFORMATION, .characteristics=(homekit_characteristic_t*[]){
              HOMEKIT_CHARACTERISTIC(IDENTIFY, light_identify),
              &name,  
              &manufacturer,
              &serial,
              &model,
              &revision,
              NULL
          }),
        HOMEKIT_SERVICE(LIGHTBULB, .characteristics=(homekit_characteristic_t*[]){
            HOMEKIT_CHARACTERISTIC(NAME, "Lâmpada 3"),
            &lightbulb_on_3,
            NULL
        }),
        NULL,
      }),

      HOMEKIT_ACCESSORY(
            .id=4,
            .category=homekit_accessory_category_switch,
            .services=(homekit_service_t*[]){
              HOMEKIT_SERVICE(ACCESSORY_INFORMATION, .characteristics=(homekit_characteristic_t*[]){
              HOMEKIT_CHARACTERISTIC(IDENTIFY, light_identify),
              &name,  
              &manufacturer,
              &serial,
              &model,
              &revision,
              NULL
          }),      

        HOMEKIT_SERVICE(LIGHTBULB, .characteristics=(homekit_characteristic_t*[]){
            HOMEKIT_CHARACTERISTIC(NAME, "Lâmpada 4"),
            &lightbulb_on_4,
            NULL
        }),
        NULL
    }),
    NULL
};

homekit_server_config_t config = {
    .accessories = accessories,
    .password = "736-24-212",
    .setupId="7EN2",
};

void on_wifi_ready() {
}

void create_accessory_name() {
    uint8_t macaddr[6];
    sdk_wifi_get_macaddr(STATION_IF, macaddr);

    int name_len = snprintf(NULL, 0, "127-suite-%02X%02X%02X",
                            macaddr[3], macaddr[4], macaddr[5]);
    char *name_value = malloc(name_len+1);
    snprintf(name_value, name_len+1, "127-suite-%02X%02X%02X",
             macaddr[3], macaddr[4], macaddr[5]);

    name.value = HOMEKIT_STRING(name_value);

    char *serial_value = malloc(13);
    snprintf(serial_value, 13, "%02X%02X%02X%02X%02X%02X", macaddr[0], macaddr[1], macaddr[2], macaddr[3], macaddr[4], macaddr[5]);
    serial.value = HOMEKIT_STRING(serial_value);
}


void user_init(void) {
    uart_set_baud(0, 115200);
    create_accessory_name();
    gpio_init();

    int c_hash=ota_read_sysparam(&manufacturer.value.string_value,&serial.value.string_value,
                                      &model.value.string_value,&revision.value.string_value);
    //c_hash=1; revision.value.string_value="0.0.1"; //cheat line
    config.accessories[0]->config_number=c_hash;

    homekit_server_init(&config);

    button_config_t config = BUTTON_CONFIG(
        button_active_low,
        .long_press_time = 10000,
        .max_repeat_presses = 3,
    );


    if (button_create(BUTTON_PIN, config, button_callback, NULL)) {
        printf("Failed to initialize button\n");
    }

    if (toggle_create(TOGGLE_PIN_1, toggle_callback_1, NULL)) {
        printf("Failed to initialize toggle 1 \n");
    }

    if (toggle_create(TOGGLE_PIN_2, toggle_callback_2, NULL)) {
        printf("Failed to initialize toggle 2 \n");
    }

    if (toggle_create(TOGGLE_PIN_3, toggle_callback_3, NULL)) {
        printf("Failed to initialize toggle 3 \n");
    }

    if (toggle_create(TOGGLE_PIN_4, toggle_callback_4, NULL)) {
        printf("Failed to initialize toggle 4 \n");
    }
}
