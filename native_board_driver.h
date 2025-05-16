/* TODO NOT BELONGS HERE */
/* TODO MAKE SAFE READ/WRITE */
struct kangoo_can_filter_frame {
	uint16_t id;
	int8_t   len;
	uint8_t  data[8];	
};

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

/* TODO proper bound checking */
void kangoo_can_filter_adafruit_mcp2515_send(
					 struct kangoo_can_filter_frame *frame)
{
	int8_t i = 0;

	if (frame->len > -1) {
		mcp.beginPacket(frame->id);

		for (i = 0; i < frame->len; i++) {
			mcp.write(frame->data[i]);
		}
		
		mcp.endPacket();
	}
}

/* TODO proper bound checking */
void kangoo_can_filter_adafruit_mcp2515_recv(
					 struct kangoo_can_filter_frame *frame)
{
	int8_t i = 0;
	int8_t len = mcp.parsePacket();

	if (len && len <= 8) {
		frame->id  = mcp.packetId();

		while (mcp.available() && i < 8) {
			frame->data[i] = mcp.read();
			i++;
		}
		
		frame->len = i;
	} else {
		frame->len = -1;
	}
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
bool twai_bus_0_recovery = false;
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

	twai_reconfigure_alerts_v2(twai_bus_0, TWAI_ALERT_BUS_OFF, NULL);
}

/* TWAI TEST */
void kangoo_can_filter_esp32_twai_print_status()
{
	esp_err_t code;
	twai_status_info_t info;

	code = twai_get_status_info_v2(twai_bus_0, &info);

	if (code == ESP_OK) {
		const char *state_str;
		
		printf("kangoo_can_filter_esp32_twai_print_status (twai_bus_0):\n");
		
		//Current state of TWAI controller (Stopped/Running/Bus-Off/Recovery)
		switch (info.state) {
		case TWAI_STATE_STOPPED:
			state_str = "TWAI_STATE_STOPPED";
			break;
		case TWAI_STATE_RUNNING:
			state_str = "TWAI_STATE_RUNNING";
			break;
		case TWAI_STATE_BUS_OFF:
			state_str = "TWAI_STATE_BUS_OFF";
			break;
		case TWAI_STATE_RECOVERING:
			state_str = "TWAI_STATE_RECOVERING";
			break;
		default:
			state_str = "TWAI_STATE_UNKNOWN";
			break;
		}

		printf("\t state: %s\n", state_str);
		
		//Number of messages queued for transmission or awaiting transmission completion
		printf("\t msgs_to_tx:       %lu\n", info.msgs_to_tx);

		//Number of messages in RX queue waiting to be read
		printf("\t msgs_to_rx:       %lu\n", info.msgs_to_rx);

		//Current value of Transmit Error Counter
		printf("\t tx_error_counter: %lu\n", info.tx_error_counter);

		//Current value of Receive Error Counter
		printf("\t rx_error_counter: %lu\n", info.rx_error_counter);

		//Number of messages that failed transmissions
		printf("\t tx_failed_count:  %lu\n", info.tx_failed_count);

		//Number of messages that were lost due to a full RX queue (or errata workaround if enabled)
		printf("\t rx_missed_count:  %lu\n", info.rx_missed_count);

		//Number of messages that were lost due to a RX FIFO overrun
		printf("\t rx_overrun_count: %lu\n", info.rx_overrun_count);

		//Number of instances arbitration was lost
		printf("\t arb_lost_count:   %lu\n", info.arb_lost_count);

		//Number of instances a bus error has occurred		
		printf("\t bus_error_count:  %lu\n", info.bus_error_count);
	} else {
		printf("No valid status for twai_bus_0 (%s)\n",
			esp_err_to_name(code));
	}
}

/* TODO proper bound checking */
void kangoo_can_filter_esp32_twai_send(struct kangoo_can_filter_frame *frame)
{
	if (frame->len > -1 || twai_bus_0_recovery) {
		int8_t i;

		// Configure message to transmit
		twai_message_t msg = {0};
		
		// Message type and format settings
		msg.extd = 0;         // Standard vs extended format
		msg.rtr  = 0;         // Data vs RTR frame
		msg.ss   = 0;         // Whether the message is single shot (i.e., does not repeat on error)
		msg.self = 0;         // Whether the message is a self reception request (loopback)
		msg.dlc_non_comp = 0; // DLC is less than 8
		    
		// Message ID and payload
		msg.identifier = frame->id;
		msg.data_length_code = frame->len;

		for (i = 0; i < frame->len; i++) {
			msg.data[i] = frame->data[i];
		}

		// Queue message for transmission
		twai_transmit_v2(twai_bus_0, &msg, 0);
	}
}

/* TODO proper bound checking */
void kangoo_can_filter_esp32_twai_recv(struct kangoo_can_filter_frame *frame)
{
	twai_message_t msg = {0};

	if (twai_receive(&msg, 0) == ESP_OK && msg.data_length_code <= 8) {
		int8_t i = 0;

		frame->id = msg.identifier;

		if (!(msg.rtr)) {
			for (i = 0; i < msg.data_length_code; i++) {
				frame->data[i] = msg.data[i];
			}
		}
		
		frame->len = i;
	} else {
		frame->len = -1;
	}
}

/******************************************************************************
 * Abastaction over specific driver
 *****************************************************************************/
void kangoo_can_filter_dri_init()
{
	kangoo_can_filter_init_adafruit_mcp2515();
	kangoo_can_filter_init_esp32_twai();
}

void kangoo_can_filter_dri_update()
{
	uint32_t alerts;

	twai_read_alerts_v2(twai_bus_0, &alerts, 0);
	
	if (alerts & TWAI_ALERT_BUS_OFF) {
		twai_bus_0_recovery = true;
		printf("TWAI_ALERT_BUS_OFF\n");
		twai_reconfigure_alerts_v2(twai_bus_0, TWAI_ALERT_BUS_RECOVERED, NULL);
		twai_initiate_recovery_v2(twai_bus_0);    //Needs 128 occurrences of bus free signal
	}
	
	if (alerts & TWAI_ALERT_BUS_RECOVERED) {
		twai_bus_0_recovery = false;
		printf("TWAI_ALERT_BUS_RECOVERED\n");
		twai_reconfigure_alerts_v2(twai_bus_0, TWAI_ALERT_BUS_OFF, NULL);
	}
}

void kangoo_can_filter_send_frame(uint8_t bus_id,
				  struct kangoo_can_filter_frame *frame)
{
	if (bus_id == 0) {
		kangoo_can_filter_adafruit_mcp2515_send(frame);
	} else if (bus_id == 1) {
		kangoo_can_filter_esp32_twai_send(frame);
	} else {
		frame->len = -1;
	}
}

void kangoo_can_filter_recv_frame(uint8_t bus_id,
				  struct kangoo_can_filter_frame *frame)
{
	if (bus_id == 0) {
		kangoo_can_filter_adafruit_mcp2515_recv(frame);
	} else if (bus_id == 1) {
		kangoo_can_filter_esp32_twai_recv(frame);
	} else {
		frame->len = -1;
	}
}
