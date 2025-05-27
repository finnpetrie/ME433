#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/spi.h"
#include <stdlib.h>
#define M_PI 3.14159265358979323846  
#include <math.h>     
#define SPI_PORT spi0
#define PIN_MISO 16
#define PIN_CS 20
#define PIN_SCK 18
#define PIN_MOSI 19


#define SRAM_CMD_WREN   0x06   // not needed for SRAM, just for EEPROM
#define SRAM_CMD_WR    0x02
#define SRAM_CMD_RD    0x03
#define SRAM_CMD_MODE  0x01

#define N_SAMPLES 1000
static float lut[N_SAMPLES];
#define PIN_CS1 15
#define FS        100.0f       // 100 samples-per-second (10 ms period)
#define FREQ      2.0f         // target sine frequency, 2 Hz
#define DT_MS     (1000.0f/FS) // = 10 ms
#define PH_STEP   (2.0f * M_PI * FREQ / FS) 
#define STATUS_SEQ_DISABLE_HOLD  0x41   // 01 = sequential, bit0=1 disables HOLD
static inline void sram_cs_low()  { gpio_put(PIN_CS1, 0); }
static inline void sram_cs_high() { gpio_put(PIN_CS1, 1); }
static float  read_back[N_SAMPLES];

void sram_set_mode(uint8_t mode) {
    uint8_t cmd[2] = { SRAM_CMD_MODE, mode };
    sram_cs_low();
      spi_write_blocking(spi0, cmd, 2);
    sram_cs_high();
}

// write `len` bytes from `buf` at 16-bit address `addr`
void sram_write_bytes(uint16_t addr, uint8_t *buf, size_t len) {
    uint8_t header[3] = {
      SRAM_CMD_WR,
      (uint8_t)(addr >> 8),
      (uint8_t)(addr & 0xFF)
    };
    sram_cs_low();
      spi_write_blocking(spi0, header, 3);
      spi_write_blocking(spi0, buf,    len);
    sram_cs_high();
}

// read `len` bytes into `buf` from 16-bit address `addr`
void sram_read_bytes(uint16_t addr, uint8_t *buf, size_t len) {
    uint8_t header[3] = {
      SRAM_CMD_RD,
      (uint8_t)(addr >> 8),
      (uint8_t)(addr & 0xFF)
    };
    sram_cs_low();
      spi_write_blocking(spi0, header, 3);
      spi_read_blocking (spi0, 0, buf,    len);
    sram_cs_high();
}



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
    // float v_ref = 3.3;      
    // voltage += 1.0f;
    // voltage *= 0.5f;
    // voltage *= 1023;
    uint16_t v = (uint16_t)((voltage / 3.3f) * 1023.0f + 0.5f); // round correctly
    // uint16_t v = (uint16_t)(voltage);   
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

void read_sram_block(uint32_t base_addr, float *dst, size_t n_floats) {
    uint8_t cmd[3] = {
        0x03,                       // READ opcode
        (uint8_t)(base_addr >> 8), // high-byte of 16-bit address
        (uint8_t) base_addr        // low-byte
    };
    gpio_put(PIN_CS1, 0);
      spi_write_blocking(SPI_PORT, cmd,       3);
      spi_read_blocking (SPI_PORT, 0x00,     (uint8_t*)dst, n_floats * sizeof(float));
    gpio_put(PIN_CS1, 1);
}

int main()
{   
    stdio_init_all();
    spi_init(SPI_PORT, 100*1000);
    gpio_set_function(PIN_MISO, GPIO_FUNC_SPI);
    gpio_set_function(PIN_CS, GPIO_FUNC_SIO);
    gpio_set_function(PIN_SCK, GPIO_FUNC_SPI);
    gpio_set_function(PIN_MOSI, GPIO_FUNC_SPI);
    
    gpio_set_function(PIN_CS1, GPIO_FUNC_SIO);
    gpio_set_dir(PIN_CS1, GPIO_OUT);
    gpio_put(PIN_CS1, 1);
    // gpio_init(PICO_DEFAULT_SPI_CSN_PIN);
    // gpio_put(PICO_DEFAULT_SPI_CSN_PIN, 1);
    gpio_set_dir(PIN_CS, GPIO_OUT);
    gpio_put(PIN_CS, 1);

    // float     lut[N_SAMPLES];
    uint8_t   bytes[4];

    // 1) build your 0→3.3 V sine lookup
    for(int i = 0; i < N_SAMPLES; i++) {
        float phase = 2.0f * M_PI * i / (float)N_SAMPLES;
        float v     = (sinf(phase) + 1.0f) * 1.65f;  // puts sine into [0,3.3]
        lut[i]      = v;
    }
    sram_set_mode(0x41);   // MODE = 01 (sequential), HOLD disabled

    // 3) write all samples in one burst
    //    address 0 .. (N_SAMPLES*4−1)
    sram_write_bytes(
    0,
    (uint8_t*)lut,            // SRAM is just bytes; cast the float array
    N_SAMPLES * sizeof(float) // 1000×4 = 4000 bytes
    );
    //need to send two 8 bits number
    //need to create a 16 bit number
    //put bits 
    //buf = 1
    //gain = 1
    //shdn = 
    uint8_t inbuf[4];
    int     idx = 0;
    build_triangle_lut();
    float total_time_s   = 1.0f;/* duration of one LUT round, e.g. 1.0f for 1 s */
    uint32_t dt_us       = (uint32_t)((total_time_s / N_SAMPLES) * 1e6f);
 while(!stdio_usb_connected()){
        sleep_ms(100);
    }
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

        // 2) Do one big sequential read
          read_sram_block(0, read_back, N_SAMPLES);
        // sleep_ms(1000);

    for (int i = 0; i < N_SAMPLES; i++){
        // sleep_ms(10);
        // v = read_back[i];
        // printf("%4d: %f\n", i, read_back[i]);

        Write_Wave(0, read_back[i], PIN_CS);
        printf("%4d: %f\n", i, read_back[i]);
        sleep_ms(10);
        // printf("%4d: %f\n", i, read_back[i]);
        // sleep_ms(10);
    }
    }

    // 2) push it to your DAC
    // Write_Wave(0, v, PIN_CS_DAC);

    // 3) advance sample (wrap at N_SAMPLES)
    // if (++idx >= N_SAMPLES) idx = 0;

    //     for(i =0 ;i < 256; i++){
    //         sleep_ms(10);
    //         v = tri_lut[i];
    //         Write_Wave(0, v, PIN_CS);
    //     }
    //     sleep_ms(10);
    // }
}
