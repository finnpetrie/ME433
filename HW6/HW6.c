#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"

// I2C defines
// This example will use I2C0 on GPIO8 (SDA) and GPIO9 (SCL) running at 400KHz.
// Pins can be changed, see the GPIO function select table in the datasheet for information on GPIO assignments
#define I2C_PORT i2c0
#define I2C_SDA 8
#define I2C_SCL 9
#define MCP_ADDR 0x20
#define LED_PICO_PIN 25




// i2c_write_blocking(i2c_default, ADDR, buf, 2, false);

// where ADDR is the 7bit address of the chip, buf is an array of 8bit data you are sending, 2 is the length of the array, and false means you are only writing, not trying to read from the chip. The first byte in buf is the value of the register you are trying to change, the second byte is the value to want to write to that register.

// To read data, use:

// i2c_write_blocking(i2c_default, ADDR, &reg, 1, true);  // true to keep master control of bus
// i2c_read_blocking(i2c_default, ADDR, &buf, 1, false); 

static void i2c_write_register(uint8_t reg, uint8_t val) {
    // Start, write addr + reg, then data, stop
    uint8_t buf[2] = {reg, val};
    /* stop = false because we only write */
    i2c_write_blocking(I2C_PORT, MCP_ADDR, buf, 2, false);
}

static uint8_t i2c_read_register(uint8_t reg) {
    // Start, write addr + reg, restart, read, stop
    i2c_write_blocking(I2C_PORT, MCP_ADDR, &reg, 1, true);
    uint8_t val = 0;
    i2c_read_blocking(I2C_PORT, MCP_ADDR, &val, 1, false);
    return val;
}


int main()
{
    stdio_init_all();

    gpio_init(LED_PICO_PIN);
    gpio_set_dir(LED_PICO_PIN, GPIO_OUT);

    // I2C Initialisation. Using it at 400Khz.
    i2c_init(I2C_PORT, 400*1000);
    
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);

    unsigned char iodir = 0b01111111;  
    i2c_write_register(0x00, 0b01111111);    

    i2c_write_register(0x06, 0b00000001);    

    /* quick blink on GP7 to confirm configuration worked */
    i2c_write_register(0x0A, 0b10000000);    // GP7 high
    sleep_ms(200);
    i2c_write_register(0x0A, 0);              

    while (true)
    {
        static absolute_time_t next_hb = {0};
        if (absolute_time_diff_us(get_absolute_time(), next_hb) <= 0) {
            gpio_xor_mask(1u << LED_PICO_PIN);
            next_hb = make_timeout_time_ms(250);
        }

        uint8_t gpio_state = i2c_read_register(0x09);

        bool button = !(gpio_state & 0x01);          // active-low button
        uint8_t out = button ? 0x80 : 0x00;          // GP7 high/low
        i2c_write_register(0x0A, out);

        sleep_ms(2);  
    }

}
