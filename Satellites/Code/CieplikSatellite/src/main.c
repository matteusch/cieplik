#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/i2c_master.h"
#include "esp_err.h"


i2c_master_bus_config_t bus_cfg = {
    .clk_source = I2C_CLK_SRC_DEFAULT,
    .i2c_port = 0,
    .scl_io_num = 5,
    .sda_io_num = 4,
    .glitch_ignore_cnt = 7,
    .flags.enable_internal_pullup = true,
};



void app_main(void){
    i2c_master_bus_handle_t bus;
    ESP_ERROR_CHECK(i2c_new_master_bus(&bus_cfg, &bus));
    while (true){
        for (uint8_t addr = 0x01; addr <= 0x7E; addr++) {
            esp_err_t ret = (i2c_master_probe(bus, addr, -1));
            if(ret == ESP_OK){
                printf("0x%02X\n", addr);
            };
        };
        vTaskDelay(pdMS_TO_TICKS(2000));
    };  
};