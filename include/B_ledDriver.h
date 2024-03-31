#pragma once

#include <esp_log.h>

#include <driver/rmt.h>

#define B_LED_GPIO 13

#define B_USED_CHANNEL RMT_CHANNEL_0

#define B_T0H_uS 0.4
#define B_T1H_uS 0.8
#define B_T0L_uS 0.85
#define B_T1L_uS 0.45

// Could automate this
#define B_T0H_ticks 14 //B_T0H_uS * 80 / 2
#define B_T1H_ticks 40 //B_T1H_uS * 80 / 2
#define B_T0L_ticks 40 //B_T0L_uS * 80 / 2
#define B_T1L_ticks 14 //B_T1L_uS * 80 / 2

#define B_LED_COUNT 16
#define B_BITS_PER_LED 3 * 8

static const char* ringTag = "RingLight - LedDriver";

// GRB color scheme
static uint8_t colorBuffer[B_LED_COUNT * 3]; // Contains the 3 colors for all the leds
static rmt_item32_t* rmtDataBuffer = NULL; // RMT data filled by B_WriteData function

void B_SetUpRMTChannel();
uint8_t* B_GetColorBufferPointer();
void B_WriteData();
void B_CleanupRMT();
