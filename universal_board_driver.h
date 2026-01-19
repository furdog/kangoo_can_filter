#include "kangoo_can_frame.h"
#include "target.gen.h"

#ifdef CAN_FILTER_V1_NATIVE_ESP32
#warning legacy CAN_FILTER_V1_NATIVE_ESP32 is used!
#endif

/******************************************************************************
 * TASKS
 *****************************************************************************/
void filesystem_task(void *pv_parameters)
{
	filesystem_init();

	while(1) {
		filesystem_update();
		vTaskDelay(30000); /* Update settings every 30 seconds */
	}
}

void web_interface_task(void *pv_parameters)
{
	web_interface_init();

	while(1) {
		web_interface_update();
		vTaskDelay(0);
	}
}

/******************************************************************************
 * DELTA TIME
 *****************************************************************************/
static clock_t timestamp_prev = 0;
static clock_t timestamp      = 0;

clock_t get_delta_time_ms()
{
	clock_t delta;
	
	timestamp = millis();
	delta = timestamp - timestamp_prev;
	timestamp_prev = timestamp;
	
	return delta;
}

/******************************************************************************
 * KANGOO_CAN_FILTER_INIT_ADAFRUIT_MCP2515
 *****************************************************************************/
#ifdef CAN_FILTER_V1_NATIVE_ESP32
#ifdef CAN_FILTER_CAN_ADAFRUIT
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

void kangoo_can_filter_init_mcp2515()
{
	if (!mcp.begin(CAN1_DESIRED_BIT_RATE)) {
		printf("Adafruit_MCP2515 init failed!\n");
		while(1) delay(10);
	}
	
	Serial.println("MCP2515 chip found");
}

/* TODO proper bound checking */
void kangoo_can_filter_mcp2515_send(
					 struct kangoo_can_frame *frame)
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
void kangoo_can_filter_mcp2515_recv(
					 struct kangoo_can_frame *frame)
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
#endif /* CAN_FILTER_CAN_ADAFRUIT */

#ifdef CAN_FILTER_CAN_ACAN
#include <ACAN2515.h>

#define CAN1_SCK   18
#define CAN1_MOSI  23
#define CAN1_MISO  19
#define CAN1_CS    17 
#define CAN1_INT   -1
#define CAN1_RESET -1
#define CAN1_DESIRED_BIT_RATE 1000UL * 500UL // 500 Kb/s
#define CAN1_QUARTZ_FREQUENCY 16UL * 1000UL * 1000UL // 8 MHz
ACAN2515 can1 (CAN1_CS, SPI, CAN1_INT);

void kangoo_can_filter_init_mcp2515()
{
	uint16_t error_code;

	/** ESP32 MCP2515 Shield CAN. */
	SPI.begin(CAN1_SCK, CAN1_MISO, CAN1_MOSI);
	ACAN2515Settings settings1(CAN1_QUARTZ_FREQUENCY, CAN1_DESIRED_BIT_RATE);
	error_code = can1.begin(settings1, NULL);

	if (!error_code) {
		Serial.println ("CAN1 OK") ;
	} else {
		Serial.print ("ERROR CAN1: 0x") ;
		Serial.println (error_code, HEX) ;
	}
}

/* TODO proper bound checking */
void kangoo_can_filter_mcp2515_send(struct kangoo_can_frame *frame)
{
	int i;
	CANMessage acan_frame;
	
	acan_frame.len = frame->len;
	acan_frame.id  = frame->id;

	for (i = 0; i < acan_frame.len; i++) {
		acan_frame.data[i] = frame->data[i];
	}
	
	can1.tryToSend(acan_frame);
}

void kangoo_can_filter_mcp2515_recv(struct kangoo_can_frame *frame)
{
	int i;
	CANMessage acan_frame;

	can1.poll();

	if (can1.receive(acan_frame) && acan_frame.len <= 8) {
		frame->len = acan_frame.len;
		frame->id  = acan_frame.id;

		for (i = 0; i < acan_frame.len; i++) {
			frame->data[i] = acan_frame.data[i];
		}
	} else {
		frame->len = -1;
	}
}

#endif

#endif /* CAN_FILTER_V1_NATIVE_ESP32 */

/******************************************************************************
 * KANGOO_CAN_FILTER_INIT_ESP32_TWAI
 *****************************************************************************/
/* ESP32 specific CAN driver */
#include "driver/gpio.h"
#include "driver/twai.h"

#if defined(CAN_FILTER_ESP32C6_ZERO)
#warning ESP32C6_ZERO is used!
#define TWAI_BUS_0_TX GPIO_NUM_2
#define TWAI_BUS_0_RX GPIO_NUM_1

#define TWAI_BUS_1_TX GPIO_NUM_4
#define TWAI_BUS_1_RX GPIO_NUM_3
#elif defined(CAN_FILTER_V3_NATIVE_ESP32)
#warning ESP32C6_V3 is used!
#define TWAI_BUS_0_TX GPIO_NUM_18
#define TWAI_BUS_0_RX GPIO_NUM_19

#define TWAI_BUS_1_TX GPIO_NUM_14
#define TWAI_BUS_1_RX GPIO_NUM_15
#elif !defined(CAN_FILTER_V1_NATIVE_ESP32) /* ESP32C6 */
#warning ESP32C6 is used!
#define TWAI_BUS_0_TX GPIO_NUM_13 /* Conflicts with USB */
#define TWAI_BUS_0_RX GPIO_NUM_12 /* Conflicts with USB */

#define TWAI_BUS_1_TX GPIO_NUM_14
#define TWAI_BUS_1_RX GPIO_NUM_15
#else /* ADAFRUIT */
#define TWAI_BUS_0_TX GPIO_NUM_4
#define TWAI_BUS_0_RX GPIO_NUM_16

#define TWAI_BUS_1_TX GPIO_NUM_4
#define TWAI_BUS_1_RX GPIO_NUM_16
#endif

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
	
	/* g_config.tx_queue_len = 10; */

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
				       struct kangoo_can_frame *frame)
{
	assert(bus == &twai_bus_0 || bus == &twai_bus_1);

	bool *recovery = (bus == &twai_bus_0) ? &twai_bus_0_recovery :
						&twai_bus_1_recovery ;

	if (frame->len > -1 || *recovery) {
		int8_t i;

		// Configure message to transmit
		twai_message_t msg;
		
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
				       struct kangoo_can_frame *frame)
{
	assert(bus == &twai_bus_0 || bus == &twai_bus_1);
						
	twai_message_t msg;

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
 * LEDS / WIFI / BUTTONS
 *****************************************************************************/
#ifndef CAN_FILTER_V1_NATIVE_ESP32

#include <Adafruit_NeoPixel.h>
#include "dev_timeout_led_indicator.h"

#define PIN_WS2812B 8
#define NUM_PIXELS 1

struct dev_timeout_led_indicator led_indicator;
Adafruit_NeoPixel ws2812b(NUM_PIXELS, PIN_WS2812B, NEO_GRB + NEO_KHZ800);

#endif

struct button bms_recuperation_button_minus;
struct button bms_recuperation_button_plus;

void kangoo_can_filter_init_other()
{
#ifdef CAN_FILTER_V1_NATIVE_ESP32
	pinMode(13, INPUT_PULLUP); //disable wifi on boot
	pinMode(RECUPERATION_BUTTON_MINUS, INPUT_PULLUP);
	pinMode(RECUPERATION_BUTTON_PLUS, INPUT_PULLUP);
	pinMode(LED_PIN, OUTPUT);
	pinMode(LED2_PIN, OUTPUT);

	button_init(&bms_recuperation_button_minus);
	button_init(&bms_recuperation_button_plus);
#else
	dev_timeout_led_indicator_init(&led_indicator);
	dev_timeout_led_indicator_set_count(&led_indicator, 2);

	ws2812b.begin();  // initialize WS2812B strip object (REQUIRED)
#endif
}

void kangoo_can_filter_led_update()
{
#ifndef CAN_FILTER_V1_NATIVE_ESP32
	ws2812b.setPixelColor(0, ws2812b.Color(led_state ? 255 : 0, 0, led2_state ? 255 : 0));
	ws2812b.show();
#else
	digitalWrite(LED_PIN, led_state);
	digitalWrite(LED2_PIN, led2_state);
#endif
}

void kangoo_can_filter_update_other(uint32_t delta_time_ms)
{
	static uint32_t delta_time_errors = 0;
	static uint32_t ticks = 0;
	static uint32_t timestamp;
	
	if (delta_time_ms > 1)
		delta_time_errors++;
	ticks++;

	if (millis() - timestamp >= 5000) {
#ifdef CAN_FILTER_V1_NATIVE_ESP32
		static bool wifi_on = true;
		if (wifi_on && !digitalRead(13)) {
			WiFi.disconnect(true);
			WiFi.mode(WIFI_OFF);
			wifi_on = false;
		}

		digitalWrite(LED_PIN, led_state);
		led_state = !led_state;
		digitalWrite(LED2_PIN, led2_state);
		led2_state = !led2_state;
#else
		/* led_state = !led_state;
		led2_state = !led2_state;
		kangoo_can_filter_led_update(); */
#endif

		timestamp += 5000;
		
		//Print twai status to monitor errors (uncomment)
		kangoo_can_filter_esp32_twai_print_status();
		
		printf("dt_err: %lu\n", delta_time_errors);
		printf("ticks:  %lu\n", ticks);
		delta_time_errors = 0;
		ticks = 0;
	}
	
	if (rapid_blink) {
		static uint8_t rapid_blink_timer = 0;

		rapid_blink_timer += delta_time_ms;

		if (rapid_blink_timer >= 50) {
			rapid_blink_timer = 0;
			led_state = !led_state;
			led2_state = !led2_state;
			kangoo_can_filter_led_update();
		}
	}

#ifdef CAN_FILTER_V1_NATIVE_ESP32
	bms_recuperation_button_minus.pressed = !digitalRead(RECUPERATION_BUTTON_MINUS);
	bms_recuperation_button_plus.pressed  = !digitalRead(RECUPERATION_BUTTON_PLUS);
#else
	if (dev_timeout_led_indicator_update(&led_indicator, delta_time_ms)) {
		ws2812b.setPixelColor(0, led_indicator.c.r, led_indicator.c.g,
					 led_indicator.c.b);
		ws2812b.show();
	}
#endif
}

/******************************************************************************
 * Abstraction over specific driver
 *****************************************************************************/
void kangoo_can_filter_dri_init()
{
	Serial.begin(115200);
	//delay(5000);

#ifdef WEB_INTERFACE_ENABLED
		xTaskCreate(filesystem_task, "filesystem_task", 1024*4, NULL, 1, NULL);

	#ifdef CAN_FILTER_V1_NATIVE_ESP32
		xTaskCreate(web_interface_task, "web_interface_task", 10000, NULL, 0, NULL);
	#else
		xTaskCreate(web_interface_task, "web_interface_task", 10000, NULL, 1, NULL);
	#endif
#endif

	kangoo_can_filter_init_other();

	kangoo_can_filter_init_esp32_twai(&twai_bus_0);

#ifndef CAN_FILTER_V1_NATIVE_ESP32
	kangoo_can_filter_init_esp32_twai(&twai_bus_1);
#else
	kangoo_can_filter_init_mcp2515();
#endif
}

void kangoo_can_filter_dri_update(uint32_t delta_time_ms)
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

#ifndef CAN_FILTER_V1_NATIVE_ESP32
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
#endif

	kangoo_can_filter_update_other(delta_time_ms);
}

void kangoo_can_filter_send_frame(uint8_t bus_id,
				  struct kangoo_can_frame *frame)
{
	if (bus_id == 0) {
		kangoo_can_filter_esp32_twai_send(&twai_bus_0, frame);
	} else if (bus_id == 1) {
#ifndef CAN_FILTER_V1_NATIVE_ESP32
		kangoo_can_filter_esp32_twai_send(&twai_bus_1, frame);
#else
		kangoo_can_filter_mcp2515_send(frame);
#endif
	} else {
		frame->len = -1;
	}
}

void kangoo_can_filter_recv_frame(uint8_t bus_id,
				  struct kangoo_can_frame *frame)
{
	if (bus_id == 0) {
		kangoo_can_filter_esp32_twai_recv(&twai_bus_0, frame);
	} else if (bus_id == 1) {
#ifndef CAN_FILTER_V1_NATIVE_ESP32
		kangoo_can_filter_esp32_twai_recv(&twai_bus_1, frame);
#else
		kangoo_can_filter_mcp2515_recv(frame);
#endif
	} else {
		frame->len = -1;
	}

#ifndef CAN_FILTER_V1_NATIVE_ESP32
	if (frame->len > -1) {
		dev_timeout_led_indicator_update_timer(
						 &led_indicator, bus_id, 5000);
	}
#else
	if (bus_id == 0) {
		led_state = !led_state;
		kangoo_can_filter_led_update();
	} else if (bus_id == 1) {
		led2_state = !led2_state;
		kangoo_can_filter_led_update();
	}
#endif
}
