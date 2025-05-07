#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/pwm.h"

#define SERVO_PIN 26          // GP26 = ADC0 = PWM slice 5 channel A

#define SERVO_MIN_US  500       // 0°  pulse width (µs)
#define SERVO_MAX_US  2500      // 180° pulse width (µs)
#define SERVO_PERIOD_US 20000   // 50 Hz period (µs)

/*--------------- PWM initialisation at 50 Hz ------------------*/
static uint  servo_slice, servo_chan;

void servo_init(void)
{
    gpio_set_function(SERVO_PIN, GPIO_FUNC_PWM);
    servo_slice = pwm_gpio_to_slice_num(SERVO_PIN);
    servo_chan  = pwm_gpio_to_channel(SERVO_PIN);

    pwm_config cfg = pwm_get_default_config();
    pwm_config_set_clkdiv_int_frac(&cfg, 125, 0);              
    pwm_config_set_wrap(&cfg, SERVO_PERIOD_US - 1);            
    pwm_init(servo_slice, &cfg, true);                         
}

void servo_set_angle(float angle_deg)
{
    if (angle_deg < 0)   angle_deg = 0;
    if (angle_deg > 180) angle_deg = 180;

    float us = SERVO_MIN_US + angle_deg * (SERVO_MAX_US - SERVO_MIN_US) / 180.0f;
    pwm_set_chan_level(servo_slice, servo_chan, (uint16_t) us); 
}

int main(void)
{
    stdio_init_all();
    servo_init();

    const uint32_t step_delay_ms = 11;   
    while (true)
    {
        /* Forward sweep */
        for (int a = 0; a <= 180; ++a) {
            servo_set_angle((float)a);
            sleep_ms(step_delay_ms);
        }
        /* Backward sweep */
        for (int a = 180; a >= 0; --a) {
            servo_set_angle((float)a);
            sleep_ms(step_delay_ms);
        }
    }
}
