/**
 * Copyright (c) 2020 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/multicore.h"
#include "hardware/adc.h"

#define FLAG_VOLTAGE 1
#define FLAG_LED 2
#define FLAG_VALUE 123

#define GPIO_PIN 15
#define ADC_PIN 26
volatile bool led_on = false;
void core1_entry() {

    multicore_fifo_push_blocking(FLAG_VALUE);
    while(true){
    uint32_t g = multicore_fifo_pop_blocking();
    switch(g){
        case FLAG_VOLTAGE:
            //read voltage and printf
            int voltage = adc_read();
            float v_converted = (float)voltage/(4095);
            v_converted *= 3.3f;
            printf("Voltage: %f\n", v_converted);
        break;
        case FLAG_LED:
        gpio_put(GPIO_PIN, led_on);
        led_on = !led_on;
            //change voltage
        break;
        default:
        break;
        }
    }
    // if (g != FLAG_VALUE)
    //     printf("Hmm, that's not right on core 1!\n");
    // else
    //     printf("Its all gone well on core 1!");

    // while (1)
    //     tight_loop_contents();
}

int main() {
    stdio_init_all();
    printf("Hello, multicore!\n");
    while(!stdio_usb_connected()){
        sleep_ms(100);
    }
    adc_init();
    adc_gpio_init(ADC_PIN);
    adc_select_input(0);
    gpio_init(GPIO_PIN);
    gpio_set_dir(GPIO_PIN, GPIO_OUT);    
    // while (true) {
    //     gpio_put(15, 1);  // Set pin high
    //     sleep_ms(500);
    //     gpio_put(15, 0);  // Set pin low
    //     sleep_ms(500);
    // }

    multicore_launch_core1(core1_entry);
    uint32_t g = multicore_fifo_pop_blocking();

    /// \tag::setup_multicore[]
    while(true){
        int message;
        printf("Enter a command\n");
        scanf("%d", &message);
        multicore_fifo_push_blocking(message);

    }
    
    // Wait for it to start up


    // if (g != FLAG_VALUE)
    //     printf("Hmm, that's not right on core 0!\n");
    // else {
    //     multicore_fifo_push_blocking(FLAG_VALUE);
    //     printf("It's all gone well on core 0!");
    // }

    /// \end::setup_multicore[]
}
