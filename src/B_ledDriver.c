#include "B_ledDriver.h"

esp_err_t B_CreateEncoders()
{
	B_Encoders.defaultEncoder.encode = B_DefaultEncoder;
	B_Encoders.defaultEncoder.del = B_DelEncoder;
	B_Encoders.defaultEncoder.reset = B_ResetEncoder;

	// Create bytes encoder
	rmt_bytes_encoder_config_t bytesEncoderConfig = {
		.bit0 = {
			.level0 = 1,
			.duration0 = B_T0H_ticks,
			.level1 = 0,
			.duration1 = B_T0L_ticks,
		},
		.bit1 = {
			.level0 = 1,
			.duration0 = B_T1H_ticks,
			.level1 = 0,
			.duration1 = B_T1L_ticks,
		},
		.flags.msb_first = 1 // Most Significant Bit first
	};
	if (rmt_new_bytes_encoder(&bytesEncoderConfig, &B_Encoders.bytesEncoder) != ESP_OK) {
		ESP_LOGE(ringTag, "Failed to create bytes encoder");
		return ESP_FAIL;
	}

	// Create copy encoder
	rmt_copy_encoder_config_t copyEncoderConfig = {};
	if (rmt_new_copy_encoder(&copyEncoderConfig, &B_Encoders.copyEncoder)){
		ESP_LOGE(ringTag, "Failed to create copy encoder");
		rmt_del_encoder(B_Encoders.bytesEncoder);
		return ESP_FAIL;
	}

	// Create the reset signal for the copy encoder
	B_resetSignal = (rmt_symbol_word_t) {
		.level0 = 0,
		.duration0 = B_RST_ticks / 2,
		.level1 = 0,
		.duration1 = B_RST_ticks / 2,
	};

	return ESP_OK;
}

void B_SetUpRMTChannel()
{
	// Create TX channel
	rmt_tx_channel_config_t txChannelConfig = {
		.clk_src = RMT_CLK_SRC_DEFAULT,
		.gpio_num = B_LED_GPIO,
		.mem_block_symbols = 64, // TODO: might have to increase for more leds
		.resolution_hz = B_RMT_FREQ,
		.trans_queue_depth = 4,
	};
	ESP_ERROR_CHECK(rmt_new_tx_channel(&txChannelConfig, &B_RMTChannel));

	// Create encoders
	ESP_ERROR_CHECK(B_CreateEncoders());

	// Enable channel
	ESP_ERROR_CHECK(rmt_enable(B_RMTChannel));
}

size_t B_DefaultEncoder(rmt_encoder_t* encoder, rmt_channel_handle_t tx_channel, const void* primary_data, size_t data_size, rmt_encode_state_t* ret_state)
{
	rmt_encode_state_t outState = 0;
	rmt_encode_state_t currentState = 0; // Return state of the downstream encoders
	size_t encodedSymbols = 0; // The function needs to return the succesfully encoded RMT symbol count

	switch (B_encodingState) {
		case B_COLOR_ENCODE: { // Encode color data
			encodedSymbols += B_Encoders.bytesEncoder->encode(B_Encoders.bytesEncoder, tx_channel, primary_data, data_size, &currentState);

			// Allow fall through
			if (currentState & RMT_ENCODING_COMPLETE)
				B_encodingState = B_RESET_ENCODE;

			if (currentState & RMT_ENCODING_MEM_FULL) {
				outState |= RMT_ENCODING_MEM_FULL;
				break;
			}
		}

		case B_RESET_ENCODE: { // Encode reset signal
			encodedSymbols += B_Encoders.copyEncoder->encode(B_Encoders.copyEncoder, tx_channel, &B_resetSignal, sizeof(B_resetSignal), &currentState);

			if (currentState & RMT_ENCODING_COMPLETE) {
				B_encodingState = B_COLOR_ENCODE;
				outState |= RMT_ENCODING_COMPLETE;
			}

			if (currentState & RMT_ENCODING_MEM_FULL) {
				outState |= RMT_ENCODING_MEM_FULL;
				break;
			}
		}
	}

	*ret_state = outState;
	return encodedSymbols;
}

esp_err_t B_DelEncoder(rmt_encoder_t* encoder)
{
	rmt_del_encoder(B_Encoders.bytesEncoder);
	rmt_del_encoder(B_Encoders.copyEncoder);
	return ESP_OK;
}

esp_err_t B_ResetEncoder(rmt_encoder_t* encoder)
{
	rmt_encoder_reset(B_Encoders.bytesEncoder);
	rmt_encoder_reset(B_Encoders.copyEncoder);
	B_encodingState = B_COLOR_ENCODE;
	return ESP_OK;
}

uint8_t* B_GetColorBufferPointer()
{
	return B_colorBuffer;
}

void B_TransmitData()
{
	rmt_transmit_config_t transmitConfig = {.loop_count = 0};
	ESP_ERROR_CHECK(rmt_transmit(B_RMTChannel, &B_Encoders.defaultEncoder, B_colorBuffer, sizeof(B_colorBuffer), &transmitConfig));
}

void B_CleanupRMT()
{
	rmt_disable(B_RMTChannel);
	rmt_del_encoder(B_Encoders.bytesEncoder);
	rmt_del_encoder(B_Encoders.copyEncoder);
}
