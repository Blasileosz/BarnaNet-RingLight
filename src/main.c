#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/event_groups.h>

#include <esp_system.h>
#include <esp_event.h>
#include <esp_log.h>

#include <nvs_flash.h>

#include "B_ledDriver.h"

static const char* tag = "RingLight";

QueueHandle_t commandQueue;

void app_main()
{
	// Init NVS
	esp_err_t flashReturn = nvs_flash_init();
	if (flashReturn == ESP_ERR_NVS_NO_FREE_PAGES || flashReturn == ESP_ERR_NVS_NEW_VERSION_FOUND)
	{
		ESP_ERROR_CHECK(nvs_flash_erase());
		flashReturn = nvs_flash_init();
	}
	ESP_ERROR_CHECK(flashReturn);

	// Init RMT
	B_SetUpRMTChannel();
	ESP_LOGI(tag, "RMT channel set up");

	// Get RMT channel speed
	uint32_t outHz = 0;
	rmt_get_counter_clock(B_USED_CHANNEL, &outHz);
	ESP_LOGI(tag, "Clock speed: %u Hz", (unsigned int)outHz);

	// Update ring
	uint8_t* colorBufferPtr = B_GetColorBufferPointer();
	int counter = 0;
	while (true) {
		colorBufferPtr[counter] = (colorBufferPtr[counter] != 32) * 32;
		B_WriteData();

		counter = (counter + 3) % (B_LED_COUNT * 3);
		vTaskDelay(1000 / portTICK_PERIOD_MS);
	}

	B_CleanupRMT();
}
