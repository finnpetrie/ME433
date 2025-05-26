#include <stdio.h>
#include <string.h>

#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include <stdlib.h>


#include "ssd1306.h"
#include "font.h"
#define BUS     i2c0
#define LED_BUS i2c1
#define MPU_ADDR        0x68      
#define REG_PWR_MGMT_1  0x6B
#define REG_PWR_MGMT_2  0x6C
#define REG_ACCEL_CFG   0x1C
#define REG_GYRO_CFG    0x1B
#define REG_DATA_START  0x3B

#define LED_SDA 14
#define LED_SCL 15
#define SDA_PIN 4
#define SCL_PIN 5



static bool mpu_write_reg(uint8_t reg, uint8_t val)
{
    int rc1 = i2c_write_blocking(BUS, MPU_ADDR, &reg, 1, true);
    int rc2 = i2c_write_blocking(BUS, MPU_ADDR, &val, 1, false);
    printf("write reg 0x%02X: rc1=%d rc2=%d\n", reg, rc1, rc2);
    return (rc1 == 1 && rc2 == 1);
}

static bool mpu_read_reg(uint8_t reg, uint8_t *val)
{
    if (i2c_write_blocking(BUS, MPU_ADDR, &reg, 1, true)  != 1) return false;
    if (i2c_read_blocking (BUS, MPU_ADDR,  val, 1, false) != 1) return false;
    return true;
}

typedef struct { int16_t ax, ay, az, t, gx, gy, gz; } frame_t;

static bool mpu_read_frame(frame_t *f)
{
    uint8_t reg = REG_DATA_START, b[14];
    if (i2c_write_blocking(BUS, MPU_ADDR, &reg, 1, true)  != 1) return false;
    if (i2c_read_blocking (BUS, MPU_ADDR,  b,   14, false)!=14) return false;

    f->ax = (b[0]<<8)|b[1];  f->ay = (b[2]<<8)|b[3];  f->az = (b[4]<<8)|b[5];
    f->t  = (b[6]<<8)|b[7];
    f->gx = (b[8]<<8)|b[9];  f->gy = (b[10]<<8)|b[11]; f->gz = (b[12]<<8)|b[13];
    return true;
}


static void debug_wakeup(void)
{
    uint8_t who;
    uint8_t reg = 0x75;
    int rc;

    rc = i2c_write_blocking(BUS, MPU_ADDR, &reg, 1, true);
    rc = i2c_read_blocking (BUS, MPU_ADDR, &who, 1, false);
    printf("WHO_AM_I before wake = 0x%02X (rc=%d)\n", who, rc);

    uint8_t addr = 0x6B;
    rc = i2c_write_blocking(BUS, MPU_ADDR, &addr, 1, true);
    printf("addr‑byte rc = %d (expect 1)\n", rc);

    uint8_t val = 0x01;
    rc = i2c_write_blocking(BUS, MPU_ADDR, &val, 1, false);
    printf("data‑byte rc = %d (expect 1)\n", rc);

    sleep_ms(50);

    uint8_t pwr;
    mpu_read_reg(0x6B, &pwr);
    printf("PWR_MGMT_1 now = 0x%02X\n", pwr);
}

void drawLine(int x0, int y0, int x1, int y1, uint8_t color) {
    int dx =  abs(x1 - x0), sx = x0 < x1 ? 1 : -1;
    int dy = -abs(y1 - y0), sy = y0 < y1 ? 1 : -1;
    int err = dx + dy;  // error term

    while (true) {
        ssd1306_drawPixel(x0, y0, color);
        if (x0 == x1 && y0 == y1) break;
        int e2 = 2 * err;
        if (e2 >= dy) { err += dy; x0 += sx; }
        if (e2 <= dx) { err += dx; y0 += sy; }
    }
}

void DrawVectors(float ax, float ay) {
 const float SCREEN_W = 128.0f, SCREEN_H =  32.0f;
    const int   CX = SCREEN_W/2, CY = SCREEN_H/2;
    const float PIXELS_PER_G = SCREEN_H/2.0f;  // =16.0f
    int dx = (int)(ax * PIXELS_PER_G);
    int dy = (int)(-ay * PIXELS_PER_G);

    // Compute end point
    int x1 = CX + dx;
    int y1 = CY + dy;

    ssd1306_clear();
    // Draw gravity vector
    drawLine(CX, CY, x1, y1, 1);
    drawLine(CX, CY, CX - dx, CY - dy, 1);
    ssd1306_update();
}
int main(void)
{
    stdio_init_all();
    i2c_init(BUS, 50*1000);                    /* 100 kHz for first bring‑up */
    gpio_set_function(SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(SCL_PIN, GPIO_FUNC_I2C);
    
    i2c_init(LED_BUS, 400*1000);
    gpio_set_function(LED_SDA,GPIO_FUNC_I2C);
    gpio_set_function(LED_SCL, GPIO_FUNC_I2C); 
    gpio_pull_up(LED_SCL);
    gpio_pull_up(LED_SDA);
    ssd1306_setup();

    // gpio_pull_up(SDA_PIN);  gpio_pull_up(SCL_PIN);
    sleep_ms(100);
    // while(!stdio_usb_connected()){
    //     sleep_ms(100);
    // }
        printf("Scanning…\n");
    for (uint8_t addr = 1; addr < 0x7F; ++addr) {
        int rc = i2c_write_blocking(i2c0, addr, NULL, 0, false);
        if (rc >= 0)  printf("Device at 0x%02X\n", addr);
    }
    printf("Done.\n");

     for (uint8_t addr = 1; addr < 0x7F; ++addr) {
    if (i2c_write_blocking(LED_BUS, addr, NULL, 0, false) >= 0)
        printf("i2c1: device at 0x%02X\n", addr);
    }

    int8_t dummy = 0x00;
    for (uint8_t addr = 1; addr < 0x7F; ++addr) {
        int rc = i2c_write_blocking(i2c0, addr, &dummy, 1, false);
        if (rc == 1)  printf("Device ACKed at 0x%02X\n", addr);
    }
    debug_wakeup();
    uint8_t pwr;
    uint8_t wake[2] = { 0x6B, 0x01 };
    int rc = i2c_write_blocking(BUS, MPU_ADDR, wake, 2, false);
    printf("one‑shot wake rc = %d\n", rc);   
    sleep_ms(50);
    mpu_read_reg(0x6B, &pwr);
    printf("PWR_MGMT_1 now = 0x%02X\n", pwr); // should now be 0x01
//     for (int i=0; i<5; ++i) {
//     mpu_write_reg(0x6B, 0x80);   // reset
//     sleep_ms(10);
//     mpu_write_reg(0x6B, 0x01);   // wake
//     sleep_ms(10);
//     uint8_t pwr; mpu_read_reg(0x6B, &pwr);
//     printf("[%d] PWR_MGMT_1 = 0x%02X\n", i, pwr);
// }
//     /* --- hard reset then wake --- */
//     mpu_write_reg(REG_PWR_MGMT_1, 0x80);         /* DEVICE_RESET */
//     sleep_ms(100);

    
    // if (!mpu_write_reg(REG_PWR_MGMT_1, 0x01)) {  /* clear SLEEP, clk = PLL_X */
    //     printf("wake write failed\n");  while (1) tight_loop_contents();
    // }
    mpu_write_reg(REG_PWR_MGMT_2, 0x00);         /* enable all axes */

    // /* verify */
    // uint8_t pwr;
    // mpu_read_reg(REG_PWR_MGMT_1, &pwr);
    // printf("PWR_MGMT_1 = 0x%02X (expect 0x01)\n", pwr);
    // // if (pwr & 0x40) { printf("still asleep!\n"); while(1){} }

    // /* scales */
    mpu_write_reg(REG_ACCEL_CFG, 0x00);          /* ±2 g  */
    mpu_write_reg(REG_GYRO_CFG,  0x18);          /* ±2000 °/s */
    // drawLetter(10, 10, 'A');
    // ssd1306_update();

    printf("Streaming data…\n");
    frame_t f;
    while (true) {
        // ssd1306_drawPixel(10, 10, 1);
        // ssd1306_update();
        // printf("Hello, world!\n");
        // sleep_ms(1000);
        // ssd1306_drawPixel(10, 10, 0);
        // ssd1306_update();
        // DrawVectors();
        if (mpu_read_frame(&f)) {
            printf("AX:%6d AY:%6d AZ:%6d | GX:%6d GY:%6d GZ:%6d\r\n",
                   f.ax, f.ay, f.az, f.gx, f.gy, f.gz);

                   float ax_g = (float)f.ax / 16384.0f;
                    float ay_g = (float)f.ay / 16384.0f;
                    float az_g = (float)f.az / 16384.0f;
                    printf("ax_g, ay_g = %f %f", ax_g, ay_g);
                    DrawVectors(ax_g, ay_g);
            } else {
            printf("read error\r\n");
        }
        sleep_ms(10);
    }
}