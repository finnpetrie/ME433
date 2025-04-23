#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/adc.h"

#define BUTTON 2
#define PIN 3
volatile bool BUTTON_PRESSED = false;

void SET_LED(bool LED_VALUE){
    gpio_put(PIN, LED_VALUE);

}

void gpio_callback(uint gpio, uint32_t events) {
    // Put the GPIO event(s) that just happened into event_str
    // so we can print it
    BUTTON_PRESSED = true;
    SET_LED(false);
}


int main()
{
    stdio_init_all();
    gpio_init(PIN);
    gpio_set_dir(PIN, GPIO_OUT);
    gpio_init(BUTTON);
    gpio_set_dir(BUTTON, GPIO_IN);
    gpio_pull_up(BUTTON);

    gpio_set_irq_enabled_with_callback(BUTTON, GPIO_IRQ_EDGE_FALL, true, &gpio_callback);

    while(!stdio_usb_connected()){
        sleep_ms(100);
    }
    printf("Start!\n");
    
    SET_LED(true);

    while(!BUTTON_PRESSED){
        // SET_LED(false);
    }

    adc_init();
    adc_gpio_init(26);
    adc_select_input(0);

    while(true)
    while (true) {
        printf("Enter the number of samples to take\n");
        char message[100];
        scanf("%s", message);
        int num_samples;
        sscanf(message, "%d", &num_samples);
        printf("Num samples: %d\n", num_samples);
        int sample_rate_hz = 100;
        // int hz = 1/sample_rate_hz;
        float samples[num_samples];
        for(int i = 0; i < num_samples; i++){
            uint16_t result = adc_read();
            // printf("Result: %d\n", result);
            float analog_result = (float)result/4096;
            analog_result *= 3.3f;
            samples[i] = analog_result;
            sleep_ms(10);
        }

        for(int i = 0; i < num_samples; i++){
            printf("Sample %d = %f\n", i, samples[i]);
        }

       
    }
}
