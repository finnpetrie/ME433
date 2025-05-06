#include <stdio.h>
#include "pico/stdlib.h"
#include "ssd1306.h"
#include "hardware/i2c.h"
#include <string.h>
#include "hardware/adc.h"
#include "font.h"
#define LED_PICO_PIN 25
#define I2C_PORT i2c0
#define I2C_SDA 8
#define I2C_SCL 9


static uint32_t t_prev_us = 0;   // previous display‑update time stamp
static float    fps        = 0;  // most recent frame‑rate

void adc_setup(void)
{
    adc_init();            // enable the ADC block
    adc_gpio_init(26);     // GP26 = ADC0
    adc_select_input(0);   // pick ADC0 as the current channel
}

/* Return a single‑shot voltage reading (blocking)               */
float adc0_read_voltage(void)
{
    const float VREF   = 3.3f;             // Pico's analogue reference
    const float SCALE  = VREF / 4095.0f;   // 12‑bit → volts per count
    uint16_t raw = adc_read();             // 0 … 4095
    return raw * SCALE;
}

void drawLetter(int x, int y, char letter){
    // ssd1306_clear();
    int i;
    int j;
    int idx = letter - 32;
    for(i =0 ; i < 5; i++){
        uint8_t column_bits = ASCII[idx][i]; // This column's 8-bit bitmap
        
        for(j = 0; j < 8; j++){
            //draw pixel at x + i, j + y
            if(column_bits & (1 << j)){
                ssd1306_drawPixel(x + i, y + j, 1);
            }
        }
    }
}


void DrawMessage(int x, int y, char* message){

    int len = strlen(message);
    int i;
    for(i = 0; i < len; i++){
       
        char letter = message[i];
        drawLetter(x,y, letter);
        x += 6;
    }
    ssd1306_update();
}

void DrawVolts(int x, int y){
    static char line[50];  // small buffer for formatted text
    while(true){
        float voltage = adc0_read_voltage();
        sprintf(line, "%.1f V", voltage);
        printf(line);

            uint32_t t_now_us = to_us_since_boot(get_absolute_time());
        if (t_prev_us) {                       // skip first pass
            uint32_t dt = t_now_us - t_prev_us;
            if (dt) fps = 1e6f / dt;           // µs → Hz
        }
        t_prev_us = t_now_us;

        char  line2[16];
        snprintf(line2, sizeof line2, "FPS : %.1f", fps);

    /* ------------ Draw to OLED ---------- */
        // gpio_put(LED_PICO_PIN, 1);
        // sleep_ms(250);
        // gpio_put(LED_PICO_PIN, 0);
        // sleep_ms(250);
        // snprintf(line, sizeof line, "ADC0: %.3f V", volts);
        ssd1306_clear();

        DrawMessage(x, y, line);
        DrawMessage(50, 15, line2);
        ssd1306_update();

        // sleep_ms(200);
    }
    
}

int main()
{

    stdio_init_all();
    printf("Hello world");
    i2c_init(i2c_default, 400 * 1000); // 400kHz
    gpio_set_function(4, GPIO_FUNC_I2C); // SDA
    gpio_set_function(5, GPIO_FUNC_I2C); // SCL
    gpio_pull_up(4);
    gpio_pull_up(5);
    ssd1306_setup();

    gpio_init(LED_PICO_PIN);
    gpio_set_dir(LED_PICO_PIN, GPIO_OUT);

    adc_setup();
    int i = 15;
    char message[50]; 
    // sprintf(message, "my var = %d", i); 
    // DrawMessage(20,10,message);

    // // drawLetter(10, 10, 'A');
    // ssd1306_update();

    DrawVolts(0, 0);
    // while (true) {

    //     gpio_put(LED_PICO_PIN, 1);
    //     sleep_ms(250);
    //     gpio_put(LED_PICO_PIN, 0);
    //     sleep_ms(250);
    //     // ssd1306_drawPixel(10, 10, 1);
    //     // ssd1306_update();
    //     // printf("Hello, world!\n");
    //     // sleep_ms(1000);
    //     // ssd1306_drawPixel(10, 10, 0);
     

    // }
}
