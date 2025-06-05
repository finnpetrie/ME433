#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "pico/stdlib.h"
#include "hardware/pwm.h"
#include "hardware/clocks.h"

#define IN1     16
#define IN2     17
#define BIN1    18
#define BIN2    19
#define NSLEEP  20
#define NFAULT  21

/* ------------ globals ------------ */
#define PWM_FREQ_HZ  20000u
static uint16_t pwm_wrap;
static volatile int speed_pct = 0;       // −100 … +100

static void pwm_gpio_init_20k(uint gpio) {
    gpio_set_function(gpio, GPIO_FUNC_PWM);
    uint slice = pwm_gpio_to_slice_num(gpio);
    pwm_set_wrap(slice, pwm_wrap);
    pwm_set_chan_level(slice, pwm_gpio_to_channel(gpio), 0);
    pwm_set_enabled(slice, true);
}

static void motor_set(int motor, int pct) {
    if (pct > 100)  pct = 100;
    if (pct < -100) pct = -100;

    uint fwd = motor ? BIN1 : IN1;
    uint rev = motor ? BIN2 : IN2;

    uint16_t level = (uint32_t)abs(pct) * pwm_wrap / 100u;
    pwm_set_gpio_level(fwd, (pct >= 0) ? level : 0);
    pwm_set_gpio_level(rev, (pct <  0) ? level : 0);
}

static void motors_init(void) {
    pwm_wrap = clock_get_hz(clk_sys) / PWM_FREQ_HZ - 1u;
    uint gpios[] = {IN1, IN2, BIN1, BIN2};
    for (size_t i = 0; i < sizeof(gpios)/sizeof(gpios[0]); ++i)
        pwm_gpio_init_20k(gpios[i]);

    gpio_init(NSLEEP); gpio_set_dir(NSLEEP, GPIO_OUT); gpio_put(NSLEEP, 1);
    gpio_init(NFAULT); gpio_pull_up(NFAULT);           gpio_set_dir(NFAULT, GPIO_IN);
}

int main(void) {
    stdio_init_all();
    setvbuf(stdin, NULL, _IONBF, 0);
    motors_init();
    printf("Interactive DRV8833 throttle:\n  + / =  speed++\n  -       speed--\n  space   stop\n");

    while (true) {
    int ch;                      

    printf("> ");                
    fflush(stdout);              

    if (scanf(" %c", (char *)&ch) != 1)  
        continue;              

    switch (ch) {
        case '+':
        case '=':  if (speed_pct <  100) speed_pct++; break;
        case '-':  if (speed_pct > -100) speed_pct--; break;
        case ' ':  speed_pct = 0;               break;
        default:   printf("Unknown key '%c'\n", ch); continue;
    }

    motor_set(0, speed_pct);     // both motors same speed
    motor_set(1, speed_pct);
    printf("speed = %d%%\n", speed_pct);

    if (!gpio_get(NFAULT)) {
        puts("FAULT!  speed set to 0");
        speed_pct = 0;
        motor_set(0, 0); motor_set(1, 0);
        gpio_put(NSLEEP, 0); sleep_ms(5); gpio_put(NSLEEP, 1);
    }
}
}
