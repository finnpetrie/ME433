#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/spi.h"
#include <stdlib.h>
#include <math.h>       
#define SPI_PORT spi0
#define PIN_MISO 16
#define PIN_CS 20
#define PIN_SCK 18
#define PIN_MOSI 19

#define FS        100.0f       // 100 samples-per-second (10 ms period)
#define FREQ      2.0f         // target sine frequency, 2 Hz
#define DT_MS     (1000.0f/FS) // = 10 ms
#define PH_STEP   (2.0f * M_PI * FREQ / FS) 


static inline void cs_select(uint cs_pin){
    asm volatile("nop \n nop \n nop");
    gpio_put(cs_pin, 0);
    asm volatile("nop \n nop \n nop");
}

static inline void cs_deselect(uint cs_pin){
    asm volatile("nop \n nop \n nop");
    gpio_put(cs_pin, 1);
    asm volatile("nop \n nop \n nop");
}

void print_binary(uint8_t n) {
    for (int i = sizeof(n) * 8 - 1; i >= 0; i--) {
        printf("%d", (n >> i) & 1);
    }
}

#define LUT_LEN 256
uint16_t tri_lut[LUT_LEN];

void build_triangle_lut(void)
{
    for (int i = 0; i < LUT_LEN; ++i) {
        float phase = (float)i / (float)LUT_LEN; // 0…1
        float tri   = 2.0f * fabsf(2.0f * (phase - 0.5f)) - 1.0f;
        tri_lut[i]  = (uint16_t)((tri + 1.0f) * 0.5f * 1023.0f);
    }
}

void Write_Wave(int channel, float voltage, uint cs_pin){
    uint8_t data[2];
    int len = 2;
    // data[0] = 0b000000000;
    // data[1] = 0b000000000;

    // data[0] = data[0] | (channel << 7);
    // data[0] = data[0] | (0b111<<6);
    // print_binary(data[0]);
    // printf("Initial data: %d\n", data[0]);
    //voltage is a 10 bit number that goes at the last 4 bits of the first 8 bit # and the first 6 bits of the second 8 bit #
    float v_ref = 3.3;      
    voltage += 1.0f;
    voltage *= 0.5f;
    voltage *= 1023;
    uint16_t v = (uint16_t)(voltage);   
    // printf("%d\n", v);  
    // %u for uns    //'voltage * some number divided by some number

    data[0] = (data[0]) | ((v >> 6));

    data[1] = (data[1])| ((v ) << 2);

        uint16_t word  = 0;
        word |= (channel & 0x01) << 15;         // A/B
        word |= (1     & 0x01) << 14;         // BUF
        word |= (1    & 0x01) << 13;         // GA
        word |= (1           ) << 12;           // SHDN = 1 (active)
        word |= (v & 0x03FF) << 2;              // D9-D0 into bits 11-2
      
        data[0] = (word >> 8) & 0xFF;           // MSB first
        data[1] =  word       & 0xFF;           // LSB second
    // data[0] = 0b01111000;
    // data[1] = 0b00000000;
    cs_select(cs_pin);
    spi_write_blocking(SPI_PORT, data, len);
    cs_deselect(cs_pin);

}
int main()
{
    stdio_init_all();
    spi_init(SPI_PORT, 100*1000);
    gpio_set_function(PIN_MISO, GPIO_FUNC_SPI);
    gpio_set_function(PIN_CS, GPIO_FUNC_SIO);
    gpio_set_function(PIN_SCK, GPIO_FUNC_SPI);
    gpio_set_function(PIN_MOSI, GPIO_FUNC_SPI);

    // gpio_init(PICO_DEFAULT_SPI_CSN_PIN);
    // gpio_put(PICO_DEFAULT_SPI_CSN_PIN, 1);
    gpio_set_dir(PIN_CS, GPIO_OUT);
    gpio_put(PIN_CS, 1);

    //need to send two 8 bits number
    //need to create a 16 bit number
    //put bits 
    //buf = 1
    //gain = 1
    //shdn = 
    build_triangle_lut();
    while (true) {
        printf("Hello, world!\n");
        // Write_Wave(0, 0.0f, PIN_CS);
        int i;
        float t = 0.0f;
        float v;
        float phase = 0.0f;
        // for(i =0 ; i < 100000; i++){
        //     sleep_ms((uint32_t)DT_MS);
        //     // t += 0.1f;
        //     v = sinf(phase);
        //     phase += PH_STEP;

        //     // printf("%f\n", v);
        //     Write_Wave(0, v, PIN_CS);
        // }

        for(i =0 ;i < 256; i++){
            sleep_ms(10);
            v = tri_lut[i];
            Write_Wave(0, v, PIN_CS);
        }
        sleep_ms(10);
    }
}
