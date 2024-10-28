#pragma once

#include <esp_log.h>

// https://docs.espressif.com/projects/esp-idf/en/v5.0/esp32s2/api-reference/peripherals/rmt.html#rmt-encoder
#include <driver/rmt_tx.h>

#define B_LED_GPIO 13
#define B_LED_COUNT 16 * 2

#define B_RMT_FREQ 40000000 // 40 MHz

#define B_T0H_uS 0.4
#define B_T1H_uS 0.8
#define B_T0L_uS 0.85
#define B_T1L_uS 0.45
#define B_RST_uS 50

// Could automate this
#define B_T0H_ticks B_T0H_uS * B_RMT_FREQ / 1000000
#define B_T1H_ticks B_T1H_uS * B_RMT_FREQ / 1000000
#define B_T0L_ticks B_T0L_uS * B_RMT_FREQ / 1000000
#define B_T1L_ticks B_T1L_uS * B_RMT_FREQ / 1000000
#define B_RST_ticks B_RST_uS * B_RMT_FREQ / 1000000

static const char* ringTag = "RingLight - LedDriver";

// GRB color scheme
static uint8_t B_colorBuffer[B_LED_COUNT * 3]; // Contains the 3 colors for all the leds

// RMT 
static rmt_channel_handle_t B_RMTChannel = NULL;
static struct {
	rmt_encoder_t defaultEncoder; // Manages the bytes and copy encoders, see B_DefaultEncoder
	rmt_encoder_t* bytesEncoder; // Encodes the dynamic color data
	rmt_encoder_t* copyEncoder; // Encodes the static reset signal
} B_Encoders;

// Stores the constant reset signal for the copy encoder
static rmt_symbol_word_t B_resetSignal;

// Keeps track of the state of encoding for the B_DefaultEncoder dunction
static int B_encodingState = 0;
static enum B_encodingState_t {
	B_COLOR_ENCODE,
	B_RESET_ENCODE
};

esp_err_t B_CreateEncoders();
void B_SetUpRMTChannel();

size_t B_DefaultEncoder(rmt_encoder_t* encoder, rmt_channel_handle_t channel, const void* primary_data, size_t data_size, rmt_encode_state_t* ret_state); // rmt_transmit call this function; Function declaration described in the rmt_encoder_t struct
esp_err_t B_DelEncoder(rmt_encoder_t* encoder); // Function declaration described in the rmt_encoder_t struct
esp_err_t B_ResetEncoder(rmt_encoder_t* encoder); // Function declaration described in the rmt_encoder_t struct

uint8_t* B_GetColorBufferPointer();
void B_TransmitData();
void B_CleanupRMT();
