/******************************************************************************
 * KANGOO_CAN_FILTER_INIT_ADAFRUIT_MCP2515
 *****************************************************************************/
/* USE Adafruit CAN implementation */
#include <Adafruit_MCP2515.h>

#define CAN1_SCK   18
#define CAN1_MOSI  23
#define CAN1_MISO  19
#define CAN1_CS    17 
#define CAN1_INT   -1
#define CAN1_RESET -1
#define CAN1_DESIRED_BIT_RATE 1000UL * 500UL // 500 Kb/s
#define CAN1_QUARTZ_FREQUENCY 16UL * 1000UL * 1000UL // 8 MHz

Adafruit_MCP2515 mcp(CAN1_CS, CAN1_MOSI, CAN1_MISO, CAN1_SCK);

void kangoo_can_filter_init_adafruit_mcp2515()
{
	if (!mcp.begin(CAN1_DESIRED_BIT_RATE)) {
		printf("Adafruit_MCP2515 init failed!\n");
		while(1) delay(10);
	}
	
	Serial.println("MCP2515 chip found");
}

/******************************************************************************
 * KANGOO_CAN_FILTER_INIT_ESP32_TWAI
 *****************************************************************************/
/* ESP32 specific CAN driver */
#include "driver/gpio.h"
#include "driver/twai.h"

#define TWAI_BUS_0_TX GPIO_NUM_4
#define TWAI_BUS_0_RX GPIO_NUM_16

//only ESP32-C5, C6, P4 support more than 1 twai buses
//#define TWAI_BUS_1_TX GPIO_NUM_23
//#define TWAI_BUS_1_RX GPIO_NUM_19

twai_handle_t twai_bus_0;
//twai_handle_t twai_bus_1;

void kangoo_can_filter_init_esp32_twai()
{
	esp_err_t code;
	//printf("Number of controllers: %i\n", SOC_TWAI_CONTROLLER_NUM);

	// Initialize configuration structures using macro initializers
	twai_general_config_t g_config =
	TWAI_GENERAL_CONFIG_DEFAULT(TWAI_BUS_0_TX,
					TWAI_BUS_0_RX, TWAI_MODE_NORMAL);
	twai_timing_config_t t_config = TWAI_TIMING_CONFIG_500KBITS  ();
	twai_filter_config_t f_config = TWAI_FILTER_CONFIG_ACCEPT_ALL();

	// Install driver for TWAI bus 0
	g_config.controller_id = 0;
	code = twai_driver_install_v2(&g_config, &t_config, &f_config, &twai_bus_0);
	if (code == ESP_OK) {
		printf("TWAI driver installed\n");
	} else {
		printf("Failed to install driver (%s)\n", esp_err_to_name(code));
		return;
	}
	// Start TWAI driver
	code = twai_start_v2(twai_bus_0);
	if (code == ESP_OK) {
		printf("TWAI driver started\n");
	} else {
		printf("Failed to start driver (%s)\n", esp_err_to_name(code));
		return;
	}
}
