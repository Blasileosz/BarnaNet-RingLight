#include "B_ledDriver.h"

void B_SetUpRMTChannel() {
	// Set up a TX channel
	rmt_config_t txConfig = {
		.channel = B_USED_CHANNEL,
		.clk_div = 2, // Set clock to 40 MHz (80 / 2)
		.gpio_num = B_LED_GPIO,
		.rmt_mode = RMT_MODE_TX,
		.mem_block_num = 3,
		.tx_config = {
			.carrier_en = false,
			.idle_output_en = true,
			.idle_level = 0,
			.loop_en = false
		}
	};

	ESP_ERROR_CHECK(rmt_config(&txConfig));
	ESP_ERROR_CHECK(rmt_driver_install(B_USED_CHANNEL, NULL, NULL));

	// Initialise rmtDataBuffer
	rmtDataBuffer = (rmt_item32_t*)calloc(B_LED_COUNT * B_BITS_PER_LED, sizeof(rmt_item32_t));
	
	if (rmtDataBuffer == NULL)
		assert("Could not initialise memory");
}

uint8_t* B_GetColorBufferPointer() {
	return &colorBuffer;
}

void B_WriteData() {
	const rmt_item32_t TT0 = {
		.duration0 = B_T0H_ticks,
		.level0 = 1,
		.duration1 = B_T0L_ticks,
		.level1 = 0
	};

	const rmt_item32_t TT1 = {
		.duration0 = B_T1H_ticks,
		.level0 = 1,
		.duration1 = B_T1L_ticks,
		.level1 = 0
	};

	ESP_LOG_BUFFER_HEX_LEVEL(ringTag, colorBuffer, B_LED_COUNT * 3, ESP_LOG_INFO);

	// Go though individual color values of the colorBuffer
	for (size_t colorId = 0; colorId < B_LED_COUNT * 3; colorId++)
	{
		uint8_t colorValue = colorBuffer[colorId];
		for (size_t bit = 0; bit < 8; bit++)
		{
			rmtDataBuffer[(8 * colorId) + bit] = (colorValue & (0x80 >> bit)) ? TT1 : TT0;
		}
	}

	// Write to device
	ESP_ERROR_CHECK(rmt_write_items(B_USED_CHANNEL, rmtDataBuffer, B_LED_COUNT * B_BITS_PER_LED, false));
	ESP_ERROR_CHECK(rmt_wait_tx_done(B_USED_CHANNEL, portMAX_DELAY));
}

void B_CleanupRMT() {
	rmt_driver_uninstall(B_USED_CHANNEL);
	free(rmtDataBuffer);
}
