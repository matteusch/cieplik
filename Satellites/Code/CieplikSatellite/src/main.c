#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/i2c_master.h"

#define SDA_PIN 4
#define SCL_PIN 5
#define ADDR   0x40   // 0x41 if not

static uint8_t crc8(const uint8_t *d, int len) {
    uint8_t crc = 0;
    for (int i = 0; i < len; i++) {
        crc ^= d[i];
        for (int b = 0; b < 8; b++)
            crc = (crc & 0x80) ? (crc << 1) ^ 0x31 : crc << 1;
    }
    return crc;
}

void app_main(void) {
    i2c_master_bus_config_t bus_cfg = {
        .i2c_port = I2C_NUM_0,
        .sda_io_num = SDA_PIN,
        .scl_io_num = SCL_PIN,
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .glitch_ignore_cnt = 7,
        .flags.enable_internal_pullup = true, 
    };
    i2c_master_bus_handle_t bus;
    ESP_ERROR_CHECK(i2c_new_master_bus(&bus_cfg, &bus));

    printf("I2C scan:\n");
    for (uint8_t a = 1; a < 127; a++) {
        if (i2c_master_probe(bus, a, 100) == ESP_OK)
            printf("  device at 0x%02X\n", a);
    }

    i2c_device_config_t dev_cfg = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = ADDR,
        .scl_speed_hz = 100000,
    };
    i2c_master_dev_handle_t dev;
    ESP_ERROR_CHECK(i2c_master_bus_add_device(bus, &dev_cfg, &dev));

    uint8_t rst = 0x1E;
    i2c_master_transmit(dev, &rst, 1, -1);
    vTaskDelay(pdMS_TO_TICKS(20));

    while (1) {
        uint8_t conv = 0x44;
        i2c_master_transmit(dev, &conv, 1, -1);
        vTaskDelay(pdMS_TO_TICKS(20));

        uint8_t read_cmd = 0x00;
        i2c_master_transmit(dev, &read_cmd, 1, -1);

        uint8_t r[6];
        i2c_master_receive(dev, r, 6, -1);

        if (crc8(r, 2) == r[2] && crc8(r + 3, 2) == r[5]) {
            float t = -45.0f + 175.0f * ((r[0] << 8 | r[1]) / 65535.0f);
            float h = 100.0f * ((r[3] << 8 | r[4]) / 65535.0f);
            printf("T = %.2f C   RH = %.1f %%\n", t, h);
        } else {
            printf("CRC fail\n");
        }
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}