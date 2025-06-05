#include <stdio.h>
#include "pico/stdlib.h"
#include "cam.h"
#include <stdlib.h>  // for abs()

#include "pico/stdlib.h"
#include "hardware/pwm.h"
#include "hw17_config.h"
#include <math.h>

#define PWM_L_PIN   16   // IN1
#define PWM_R_PIN   18   // BIN1
#define PWM_WRAP    10000    // 12.5 kHz  (any value 5k‑15k is fine)
#define MAX_DUTY    PWM_WRAP

// Direction LOW pins (ground these for forward only)
#define DIR_L_LOW   17   // IN2
#define DIR_R_LOW   19   // BIN2
// static volatile bool frame_ready = false;   // flag set from VSYNC ISR in cam.c

// ───────────────────────────────────────────────────────────────
//  Simple helper: set up one GPIO as PWM and return its slice.
// ───────────────────────────────────────────────────────────────
static uint pwm_init_gpio(uint gpio, uint16_t wrap_val)
{
    gpio_set_function(gpio, GPIO_FUNC_PWM);
    uint slice = pwm_gpio_to_slice_num(gpio);
    pwm_set_wrap(slice, wrap_val);
    pwm_set_enabled(slice, true);
    return slice;
}

static inline void drive_left(int duty)   { pwm_set_gpio_level(PWM_L_PIN, duty); }
static inline void drive_right(int duty)  { pwm_set_gpio_level(PWM_R_PIN, duty); }

// ───────────────────────────────────────────────────────────────
//  Controller: camera row → error(px) → two duty cycles
// ───────────────────────────────────────────────────────────────
static inline int clamp(int v,int lo,int hi){ return v<lo?lo:(v>hi?hi:v); }

static void compute_pwm(int err_px, int *duty_l, int *duty_r)
{
    /* 1. dead‑band */
    if (abs(err_px) < DEADBAND_PX) err_px = 0;

    /* 2. delta = GAIN * error  */
    int delta = (int)roundf(GAIN_PX2PWM * err_px);
    delta = clamp(delta, -MAX_DUTY, MAX_DUTY);

    /* 3. distribute to left/right */
    int l = MAX_DUTY, r = MAX_DUTY;
    if (delta > 0)       r -= delta; // need to steer right
    else if (delta < 0)  l += delta; // delta is −ve → steer left

    *duty_l = clamp(l, 0, MAX_DUTY);
    *duty_r = clamp(r, 0, MAX_DUTY);
}

static inline void drive_forward(int duty)
{
    gpio_put(DIR_L_LOW, 0);         // IN2  = LOW  (forward)
    gpio_put(DIR_R_LOW, 0);         // BIN2 = LOW  (forward)

    pwm_set_gpio_level(PWM_L_PIN, duty);  // IN1 PWM
    pwm_set_gpio_level(PWM_R_PIN, duty);  // BIN1 PWM
}

int main()
{ 
    stdio_init_all();

    // while (!stdio_usb_connected()) {
    //     sleep_ms(100);
    // }
    printf("Hello, camera!\n");
    init_camera_pins();                  // sets up MCLK on slice 5
    pwm_init_gpio(PWM_L_PIN, PWM_WRAP);   // slice 0
    pwm_init_gpio(PWM_R_PIN, PWM_WRAP);   // slice 1

    // direction‑LOW pins
    gpio_init(DIR_L_LOW);  gpio_set_dir(DIR_L_LOW, GPIO_OUT);  gpio_put(DIR_L_LOW, 0);
    gpio_init(DIR_R_LOW);  gpio_set_dir(DIR_R_LOW, GPIO_OUT);  gpio_put(DIR_R_LOW, 0);
    /* ── PWM LEDs init ───────────────────────── */
    // gpio_put(16, 1);  gpio_put(18, 1);    // IN1=1, IN2=0
    drive_forward(10000/2);
       /* ============== endless control loop ===================== */
    printf("setup pwms");
    while (true)
    {
        printf("Hello");
        /* 1. kick off a capture */
        setSaveImage(1);
        while(getSaveImage()==1){}

        convertImage();
        int centre = findLine(IMAGESIZEY/2); // calculate the position of the center of the ine
        // setPixel(IMAGESIZEY/2,com,0,255,0); // draw the center so you can see it in python
        // printImage();
        printf("%d\r\n",centre); // comment 

        // /* 3. find the line on chosen row */
        // // // int center = findLine(CAMERA_ROW);    // pixel 0‥79
        // if (centre <= 0 || centre >= IMAGESIZEX) {
        //     pwm_set_gpio_level(LED_L_PIN, 0);
        //     pwm_set_gpio_level(LED_R_PIN, 0);
        //     continue;                          // no line found
        // }

        // /* 4. compute wheel “commands” & drive LEDs */
        // int err_px = centre - (IMAGESIZEX/2);  // −40‥+39
        // int dutyL, dutyR;
        // compute_pwm(err_px, &dutyL, &dutyR);

        // pwm_set_gpio_level(LED_L_PIN, dutyL);
        // pwm_set_gpio_level(LED_R_PIN, dutyR);

        // /* 5. debug print (optional) */
        // printf("err=%d  L=%d  R=%d\n", err_px, dutyL, dutyR);
    }
}