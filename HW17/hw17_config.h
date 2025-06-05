#ifndef HW17_CONFIG_H
#define HW17_CONFIG_H

//----------------------------------------------
//  PIN MAP  (adjust if you used other pins)
//----------------------------------------------
#define LED_L_PIN      16     // left‑motor “LED”   (GP2 → series‑resistor → LED → GND)
#define LED_R_PIN      17     // right‑motor “LED”  (GP3 → …)

#define PWM_WRAP   2000     // ~20 kHz / 5; plenty fine for LEDs
#define MAX_DUTY   1800                             // (duty = 0‥10000 → 0‥100 %)

// #define CAMERA_ROW     50    // which row (0‑79) to track in 80×60 mode

//----------------------------------------------
//  CONTROLLER GAINS  (play with these live)
//----------------------------------------------
// #define MAX_DUTY       9000  // top speed (LED brightest)
#define DEADBAND_PX    4     // pixels
#define GAIN_PX2PWM    120.0f  // duty delta per pixel error

#endif