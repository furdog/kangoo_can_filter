/* TODO NOT BELONGS HERE */
/* TODO MAKE SAFE READ/WRITE */
struct kangoo_can_filter_frame {
	uint16_t id;
	int8_t   len;
	uint8_t  data[8];	
};

/******************************************************************************
 * KANGOO_CAN_FILTER_INIT_ESP32_TWAI
 *****************************************************************************/
/* ESP32 specific CAN driver */
#include "driver/gpio.h"
#include "driver/twai.h"

#define TWAI_BUS_0_TX GPIO_NUM_19
#define TWAI_BUS_0_RX GPIO_NUM_20

#define TWAI_BUS_1_TX GPIO_NUM_22
#define TWAI_BUS_1_RX GPIO_NUM_21

twai_handle_t twai_bus_0;
twai_handle_t twai_bus_1;
bool twai_bus_0_recovery = false;
bool twai_bus_1_recovery = false;

void kangoo_can_filter_init_esp32_twai(twai_handle_t *bus)
{
	esp_err_t code;

	assert(bus == &twai_bus_0 || bus == &twai_bus_1);

	//Config
	//twai_handle_t *bus    = &twai_bus_0;
	gpio_num_t     bus_tx = (bus == &twai_bus_0) ?
				TWAI_BUS_0_TX : TWAI_BUS_1_TX;
	gpio_num_t     bus_rx = (bus == &twai_bus_0) ?
				TWAI_BUS_0_RX : TWAI_BUS_1_RX;
	uint8_t        bus_id = (bus == &twai_bus_0) ? 0 : 1;

	twai_general_config_t g_config =
		TWAI_GENERAL_CONFIG_DEFAULT(bus_tx, bus_rx, TWAI_MODE_NORMAL);
	twai_timing_config_t t_config = TWAI_TIMING_CONFIG_500KBITS  ();
	twai_filter_config_t f_config = TWAI_FILTER_CONFIG_ACCEPT_ALL();

	// Install driver for TWAI bus 0
	g_config.controller_id = bus_id;
	code = twai_driver_install_v2(&g_config, &t_config, &f_config, bus);
	if (code == ESP_OK) {
		printf("TWAI driver installed\n");
	} else {
		printf("Failed to install driver (%s)\n",
			esp_err_to_name(code));
		return;
	}
	// Start TWAI driver
	code = twai_start_v2(*bus);
	if (code == ESP_OK) {
		printf("TWAI driver started\n");
	} else {
		printf("Failed to start driver (%s)\n", esp_err_to_name(code));
		return;
	}

	twai_reconfigure_alerts_v2(*bus, TWAI_ALERT_BUS_OFF, NULL);
}

/* Call only in bus off state */
void kangoo_can_filter_kill_esp32_twai(twai_handle_t *bus)
{
	assert(bus == &twai_bus_0 || bus == &twai_bus_1);

	twai_driver_uninstall_v2(*bus);
}

/* TWAI TEST */
void kangoo_can_filter_esp32_twai_print_status()
{
	//NOT IMPLEMENTED
}

/* TODO proper bound checking */
void kangoo_can_filter_esp32_twai_send(twai_handle_t *bus,
				       struct kangoo_can_filter_frame *frame)
{
	assert(bus == &twai_bus_0 || bus == &twai_bus_1);

	bool *recovery = (bus == &twai_bus_0) ? &twai_bus_0_recovery :
						&twai_bus_1_recovery ;

	if (frame->len > -1 || *recovery) {
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
		twai_transmit_v2(*bus, &msg, 0);
	}
}

/* TODO proper bound checking */
void kangoo_can_filter_esp32_twai_recv(twai_handle_t *bus,
				       struct kangoo_can_filter_frame *frame)
{
	assert(bus == &twai_bus_0 || bus == &twai_bus_1);
						
	twai_message_t msg = {0};

	if (twai_receive_v2(*bus, &msg, 0) == ESP_OK &&
	    msg.data_length_code <= 8) {
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
	kangoo_can_filter_init_esp32_twai(&twai_bus_0);
	kangoo_can_filter_init_esp32_twai(&twai_bus_1);
}

void kangoo_can_filter_dri_update()
{
	uint32_t alerts;

	alerts = 0;
	twai_read_alerts_v2(twai_bus_0, &alerts, 0);
	
	if (alerts & TWAI_ALERT_BUS_OFF) {
		twai_bus_0_recovery = true;
		printf("TWAI_ALERT_BUS_OFF\n");

		//Commented out due to driver BUG
		//twai_clear_transmit_queue_v2(twai_bus_0);
		//twai_reconfigure_alerts_v2(twai_bus_0, TWAI_ALERT_BUS_RECOVERED, NULL);
		//twai_initiate_recovery_v2(twai_bus_0);    //Needs 128 occurrences of bus free signal

		//Workaround
		twai_reconfigure_alerts_v2(twai_bus_0, 0, NULL);
		kangoo_can_filter_kill_esp32_twai(&twai_bus_0);
		kangoo_can_filter_init_esp32_twai(&twai_bus_0);
		twai_bus_0_recovery = false;
	}

	//Commented out due to driver BUG
	/*
	if (alerts & TWAI_ALERT_BUS_RECOVERED) {
		twai_bus_0_recovery = false;
		printf("TWAI_ALERT_BUS_RECOVERED\n");
		twai_reconfigure_alerts_v2(twai_bus_0, TWAI_ALERT_BUS_OFF, NULL);
	}
	*/

	alerts = 0;
	twai_read_alerts_v2(twai_bus_1, &alerts, 0);

	if (alerts & TWAI_ALERT_BUS_OFF) {
		twai_bus_1_recovery = true;
		printf("TWAI_ALERT_BUS_OFF\n");

		twai_reconfigure_alerts_v2(twai_bus_1, 0, NULL);
		kangoo_can_filter_kill_esp32_twai(&twai_bus_1);
		kangoo_can_filter_init_esp32_twai(&twai_bus_1);
		twai_bus_1_recovery = false;
	}
}

void kangoo_can_filter_send_frame(uint8_t bus_id,
				  struct kangoo_can_filter_frame *frame)
{
	if (bus_id == 0) {
		kangoo_can_filter_esp32_twai_send(&twai_bus_0, frame);
	} else if (bus_id == 1) {
		kangoo_can_filter_esp32_twai_send(&twai_bus_1, frame);
	} else {
		frame->len = -1;
	}
}

void kangoo_can_filter_recv_frame(uint8_t bus_id,
				  struct kangoo_can_filter_frame *frame)
{
	if (bus_id == 0) {
		kangoo_can_filter_esp32_twai_recv(&twai_bus_0, frame);
	} else if (bus_id == 1) {
		kangoo_can_filter_esp32_twai_recv(&twai_bus_1, frame);
	} else {
		frame->len = -1;
	}
}
