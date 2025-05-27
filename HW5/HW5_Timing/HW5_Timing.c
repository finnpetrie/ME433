#include <stdio.h>
#include "pico/stdlib.h"

#define REPS        1000U               // how many times to repeat each op
#define CPU_HZ      150000000UL         // 150 MHz default

int main(void)
{
    stdio_init_all();
    while(!stdio_usb_connected()){
        sleep_ms(100);
    }
    volatile float f1, f2;
    printf("Enter two floats to use: ");
    while (scanf("%f %f", (float*)&f1, (float*)&f2) != 2)
        ;

    volatile float f_add, f_sub, f_mult, f_div;
    uint64_t start, stop;
    double   cycles_per_us = (double)CPU_HZ / 1e6;   // 1 µs = 150 cycles

    // ── ADD ───────────────────────────────────────────────
    start = to_us_since_boot(get_absolute_time());
    for (uint32_t i = 0; i < REPS; ++i)
        f_add = f1 + f2;
    stop  = to_us_since_boot(get_absolute_time());

    double add_cycles = ((stop - start) * cycles_per_us) / REPS;

    // ── SUB ───────────────────────────────────────────────
    start = to_us_since_boot(get_absolute_time());
    for (uint32_t i = 0; i < REPS; ++i)
        f_sub = f1 - f2;
    stop  = to_us_since_boot(get_absolute_time());

    double sub_cycles = ((stop - start) * cycles_per_us) / REPS;

    // ── MULT ──────────────────────────────────────────────
    start = to_us_since_boot(get_absolute_time());
    for (uint32_t i = 0; i < REPS; ++i)
        f_mult = f1 * f2;
    stop  = to_us_since_boot(get_absolute_time());

    double mul_cycles = ((stop - start) * cycles_per_us) / REPS;

    // ── DIV ───────────────────────────────────────────────
    start = to_us_since_boot(get_absolute_time());
    for (uint32_t i = 0; i < REPS; ++i)
        f_div = f1 / f2;
    stop  = to_us_since_boot(get_absolute_time());

    double div_cycles = ((stop - start) * cycles_per_us) / REPS;

    // ── RESULTS ───────────────────────────────────────────
    printf("\n===== Results (%u repetitions) =====\n", REPS);
    printf("ADD : %.1f cycles\n", add_cycles);
    printf("SUB : %.1f cycles\n", sub_cycles);
    printf("MUL : %.1f cycles\n", mul_cycles);
    printf("DIV : %.1f cycles\n", div_cycles);

    printf("\nVerification:\n");
    printf("%f + %f = %f\n", f1, f2, f_add);
    printf("%f - %f = %f\n", f1, f2, f_sub);
    printf("%f * %f = %f\n", f1, f2, f_mult);
    printf("%f / %f = %f\n", f1, f2, f_div);

    while (true) tight_loop_contents();
}
