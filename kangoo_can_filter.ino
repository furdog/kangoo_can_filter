#include <ACAN_ESP32.h>
#include <ACAN2515.h>

#define WEB_INTERFACE_ENABLED
#define USE_NATIVE_CAN
//#define USE_DUAL_MCP

//Початкові налаштування системи (Не працюють коли веб інтерфейс увімкнений)
#define BMS_FILTERING_ENABLED                 true
#define BMS_FILTER_GENERAL_ERRORS_ENABLED     true
#define BMS_FILTER_ISOLATION_ERROR_ENABLED    false
#define BMS_RECUPERATION_MULTIPLIER_ENABLED   false
#define BMS_RECUPERATION_MULTIPLIER           1.0
#define BMS_SOH_MULTIPLIER_ENABLED            false
#define BMS_SOH_MULTIPLIER                    1.0
#define BMS_CUSTOM_CAPACITY_ENABLED           false
#define BMS_CUSTOM_CAPACITY                   34
#define BMS_UBERCHARGE_ENABLED                true
#define BMS_UBERCHARGE                        4.18
#define BMS_LIMIT_CHARGE_KWT_MANUALLY_ENABLED false
#define BMS_LIMIT_CHARGE_KWT_MANUALLY         10
#define BMS_KWT_COUNTER_ENABLED               false

#define RECUPERATION_BUTTON_MINUS 25
#define RECUPERATION_BUTTON_PLUS 33
#define LED_PIN 2
#define LED2_PIN 15
bool led_state = false;
bool led2_state = false;

#ifdef USE_NATIVE_CAN
#define CAN1_SCK   18
#define CAN1_MOSI  23
#define CAN1_MISO  19
#define CAN1_CS    17 
#define CAN1_INT   -1
#define CAN1_RESET -1
#define CAN1_DESIRED_BIT_RATE 1000UL * 500UL // 500 Kb/s
#define CAN1_QUARTZ_FREQUENCY 16UL * 1000UL * 1000UL // 8 MHz
ACAN2515 can1 (CAN1_CS, SPI, CAN1_INT);

/** When native can used, it replaces can2. */
#define CAN2_NATIVE_RX GPIO_NUM_16
#define CAN2_NATIVE_TX GPIO_NUM_4
#define CAN2_NATIVE_DESIRED_BIT_RATE 1000UL * 500UL // 500 Kb/s
#define  can2 ACAN_ESP32::can
#endif

#ifdef USE_DUAL_MCP
#define CAN1_SCK   18
#define CAN1_MOSI  23
#define CAN1_MISO  19
#define CAN1_CS     5 
#define CAN1_INT   -1
#define CAN1_RESET -1
#define CAN1_DESIRED_BIT_RATE 1000UL * 500UL // 500 Kb/s
#define CAN1_QUARTZ_FREQUENCY 16UL * 1000UL * 1000UL // 8 MHz
ACAN2515 can1 (CAN1_CS, SPI, CAN1_INT);

#define CAN2_SCK   18
#define CAN2_MOSI  23
#define CAN2_MISO  19
#define CAN2_CS     17 
#define CAN2_INT   -1
#define CAN2_RESET -1
#define CAN2_DESIRED_BIT_RATE 1000UL * 500UL // 500 Kb/s
#define CAN2_QUARTZ_FREQUENCY 16UL * 1000UL * 1000UL // 8 MHz
ACAN2515 can2 (CAN2_CS, SPI, CAN2_INT);
#endif

static bool  bms_filtering_enabled                   = BMS_FILTERING_ENABLED;
static bool  bms_filter_general_errors_enabled       = BMS_FILTER_GENERAL_ERRORS_ENABLED;
static bool  bms_filter_isolation_error_enabled      = BMS_FILTER_ISOLATION_ERROR_ENABLED;
static bool  bms_recuperation_multiplier_enabled     = BMS_RECUPERATION_MULTIPLIER_ENABLED;
static float bms_recuperation_multiplier             = BMS_RECUPERATION_MULTIPLIER;
static bool  bms_soh_multiplier_enabled              = BMS_SOH_MULTIPLIER_ENABLED;
static float bms_soh_multiplier                      = BMS_SOH_MULTIPLIER;
static bool  bms_custom_capacity_enabled             = BMS_CUSTOM_CAPACITY_ENABLED;
static float bms_custom_capacity                     = BMS_CUSTOM_CAPACITY;
static bool  bms_ubercharge_enabled                  = BMS_UBERCHARGE_ENABLED;
static float bms_ubercharge                          = BMS_UBERCHARGE;
static bool  bms_limit_charge_kwt_manually_enabled   = BMS_LIMIT_CHARGE_KWT_MANUALLY_ENABLED;
static float bms_limit_charge_kwt_manually           = BMS_LIMIT_CHARGE_KWT_MANUALLY;
static bool  bms_kwt_counter_enabled                 = BMS_KWT_COUNTER_ENABLED;

static float   bms_kwt_counter = 0.0;
static clock_t bms_ignition_key_timestamp = 0;
static int     bms_ignition_key_counter = 0;
static bool    bms_ignition_key = false;
static clock_t bms_kwt_counter_elapsed = 0;
static float   bms_limit_charge_kwt = 0.0f;
static float   bms_voltage = 0.0f;
static float   bms_current = 0.0f;
static float   bms_max_input_kwt = 0.0f;
static float   bms_max_recu_kwt = 0.0f;
static float   bms_kwh = 0.0f;
static float   bms_soc = 0.0f;
static float   bms_soh = 0.0f;
static float   bms_temp = 0.0f;
static float   bms_min_cell_v = 0.0f;
static float   bms_max_cell_v = 0.0f;
static uint8_t diag_code = 0;
static uint8_t diag_counter = 0;
static bool    bms_charger_plugged_in = 0.0f;
static bool    bms_ubercharge_active = false;

#include "button.h"
#include "filesystem.h"
#include "web_interface.h"

struct button bms_recuperation_button_minus;
struct button bms_recuperation_button_plus;
void bms_process_recuperation_buttons()
{
	if (button_update(&bms_recuperation_button_minus, millis(),
			  !digitalRead(RECUPERATION_BUTTON_MINUS)) ==
	    BUTTON_EVENT_TYPE_HOLD_TIMEOUT) {
		bms_recuperation_multiplier += 0.5;
		if (bms_recuperation_multiplier > 2.0)
			bms_recuperation_multiplier = 2.0;
		else
			commit_settings();
	}

	if (button_update(&bms_recuperation_button_plus, millis(),
			  !digitalRead(RECUPERATION_BUTTON_PLUS)) ==
	    BUTTON_EVENT_TYPE_HOLD_TIMEOUT) {
		bms_recuperation_multiplier -= 0.5;
		if (bms_recuperation_multiplier < 0.0)
			bms_recuperation_multiplier = 0.0;
		else
			commit_settings();
	}
}

void bms_wifi_reset_sequence(bool key)
{	
	/** Reset key counter if key was not turned for too long */
	if (millis() - bms_ignition_key_timestamp > 2000)
		bms_ignition_key_counter = 0;
	
	/** If the key state remain the same - exit. */
	if (bms_ignition_key != key)
		bms_ignition_key = key;
	else
		return;
	
	/** If the key not turned on - exit. */
	if (!key)
		return;
	
	/** Reset timestamp and increment key turn counter. */		
	bms_ignition_key_timestamp = millis();
	bms_ignition_key_counter++;
	
	/** If ignition key was turned 5 times,
	 *  with less or equal than 2000ms intervals - reset wifi. */
	if (bms_ignition_key_counter >= 5)
		ESP.restart();
}

#ifdef BMS_UBERCHARGE
bool ubercharge()
{
	bool result = false;
	bool reached_target = false;
	
	//if (bms_max_cell_v < bms_ubercharge && bms_charger_plugged_in)
	if (bms_charger_plugged_in)
		result = true;
		
	if (bms_max_cell_v > bms_ubercharge) {
		reached_target = true;
		bms_limit_charge_kwt -= 0.01;
	} else {
		bms_limit_charge_kwt += 0.01;
	}
	
	/** Set bounds. */					
	if (bms_limit_charge_kwt < 0.0) {
		if (bms_custom_capacity_enabled && reached_target)
			bms_kwt_counter = bms_custom_capacity;
			
		bms_limit_charge_kwt = 0.0;	
		result = false;
	} else if (bms_limit_charge_kwt_manually_enabled &&
		   bms_limit_charge_kwt > bms_limit_charge_kwt_manually) {
		bms_limit_charge_kwt = bms_limit_charge_kwt_manually;
	} else if (bms_limit_charge_kwt > 10.0) {
		bms_limit_charge_kwt = 10.0;
	}
	
	bms_ubercharge_active = result;
	return result;
}
#endif

void can_filter(CANMessage *frame)
{	
	switch (frame->id) {
	case 0x155:
		bms_voltage = (frame->data[6] << 8 | frame->data[7]) / 2;
		bms_current = (((frame->data[1] & 0x0F) << 8 | frame->data[2]) - 0x7D0) / 4.0;
		bms_max_input_kwt = (frame->data[0] / 3.0);
		bms_soc = (((uint16_t)frame->data[4] << 8) | frame->data[5]) / 400.0f;
		
		if (bms_filtering_enabled && bms_filter_general_errors_enabled)
			frame->data[3] = 0x54;

		if (bms_limit_charge_kwt_manually_enabled)
			frame->data[0] = bms_limit_charge_kwt_manually * 3;
			
		if (bms_ubercharge_enabled && bms_ubercharge_active)
			frame->data[0] = bms_limit_charge_kwt * 3;
	
		if (bms_custom_capacity_enabled) {
			bms_soc = (bms_kwh / bms_custom_capacity) * 100.0;

			frame->data[4] = (uint16_t)(bms_soc * 400.0f) >> 8 & 0x00FF;
			frame->data[5] = (uint16_t)(bms_soc * 400.0f) >> 0 & 0x00FF;
		}
		break;

	case 0x424:
		bms_temp = frame->data[4] - 40;
		bms_soh = frame->data[5];
			
		if (bms_ubercharge_enabled)
			frame->data[0] = ubercharge() ? 0x11 : 0x12;
		
		if (bms_recuperation_multiplier_enabled) {
			float new_recuperation;
			new_recuperation = frame->data[2] * bms_recuperation_multiplier;
			frame->data[2] = (new_recuperation > 0xFF) ? 0xFF : new_recuperation;
		}

		bms_max_recu_kwt = frame->data[2] / 2;
		
		/** Fix power limit error if batt health < 50% and temp < 50 */
		if (bms_filtering_enabled && bms_filter_general_errors_enabled &&
		    bms_soh < 50 && bms_temp < 50) {
			/** mul max output power by 4 */
			float new_max_output_power = frame->data[3] * 4.0;
			
			/** max output power can't be higher than 87kwt */
			frame->data[3] = (new_max_output_power > 0xAF) ? 0xAF : new_max_output_power;
		}

		if (bms_soh_multiplier_enabled) {
			bms_soh *= bms_soh_multiplier;
			if (bms_soh > 127)
				bms_soh = 127;
				frame->data[5] = bms_soh;
		}
		break;

	case 0x425:
		bms_kwh = (((uint16_t)frame->data[0] & 0x0001) << 8 | frame->data[1]) / 10.0;		
		bms_min_cell_v = ((((uint16_t)frame->data[6] & 0x0001) << 8 | frame->data[7]) + 100) / 100.0;
		bms_max_cell_v = (((((uint16_t)frame->data[4] & 0x0003) << 8 | frame->data[5]) >> 1) + 100) / 100.0;
	
		/** Ignore bms isolation resistance. */
		if (bms_filtering_enabled && bms_filter_isolation_error_enabled) {
			frame->data[3] = 0x9C;
			frame->data[4] = 0x40 | (frame->data[4] & 3);
		}
		
		/** Custom kwt counter */
		bms_kwt_counter_elapsed = millis() - bms_kwt_counter_elapsed;

		if (bms_kwt_counter_enabled) {
			
			double per_hour = ((double)bms_kwt_counter_elapsed / (1000 * 60 * 60));
			/* printf("%lu\n", elapsed); */

			bms_kwt_counter += (double)(bms_voltage * bms_current) / 1000.0 * per_hour;
			if (bms_kwt_counter < 0)
				bms_kwt_counter = 0;
						
			bms_kwh = bms_kwt_counter;
		}
		
		bms_kwt_counter_elapsed = millis();
			
		break;

	case 0x426:
		/** Try wifi reset sequence based on ignition key message. */
		bms_wifi_reset_sequence(frame->data[1] & 0x60);
		
		break;
	
	case 0x428:
  		bms_charger_plugged_in = (frame->data[6] > 0x00) ? true : false;
		break;
  
	case 0x659:
		/** Break if no filtering allowed. */
		if (!bms_filtering_enabled)
			break;		

		frame->data[0] = 0;
		frame->data[1] = 0;
		frame->data[2] = 0;
		frame->data[3] = 0;
		break;
	
	case 0x79B:
		/** Diagnostics request. */
		if (frame->data[0] == 0x2 &&
		    frame->data[1] == 0x21) {
			diag_code = frame->data[2];
			diag_counter = 0;
		}
	
		break;
	
	case 0x7BB:
		/** Diagnostics response (multimessage). */
		if (diag_code == 0x61 && diag_counter == 1) {
			frame->data[5] = bms_soh * 2;
		}
	
		diag_counter++;
	
		break;
	}
}

/** Read dynamic parameters every 100ms. */
void filesystem_task(void *pv_parameters)
{
	filesystem_init();

	while(1) {
		filesystem_update();
		vTaskDelay(10000);
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

void setup()
{
	uint16_t error_code;

	pinMode(13, INPUT_PULLUP); //disable wifi on boot
	pinMode(RECUPERATION_BUTTON_MINUS, INPUT_PULLUP);
	pinMode(RECUPERATION_BUTTON_PLUS, INPUT_PULLUP);
	pinMode(LED2_PIN, OUTPUT);
	pinMode(LED_PIN, OUTPUT);
	pinMode(LED2_PIN, OUTPUT);
	Serial.begin(115200);
	delay(100);

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
	#ifdef USE_DUAL_MCP
	/** ESP32 MCP2515 Shield CAN. */
	SPI.begin(CAN2_SCK, CAN2_MISO, CAN2_MOSI);
	ACAN2515Settings settings2(CAN2_QUARTZ_FREQUENCY, CAN2_DESIRED_BIT_RATE);
	error_code = can2.begin(settings2, NULL);
	#endif

	#ifdef USE_NATIVE_CAN
	/** ESP32 Native CAN. */
	ACAN_ESP32_Settings settings(CAN2_NATIVE_DESIRED_BIT_RATE);
	settings.mRequestedCANMode = ACAN_ESP32_Settings::NormalMode;
	settings.mRxPin = CAN2_NATIVE_RX; // Optional, default Tx pin is GPIO_NUM_4
	settings.mTxPin = CAN2_NATIVE_TX; // Optional, default Rx pin is GPIO_NUM_5
	error_code = can2.begin(settings);
	#endif
	
	if (!error_code) {
		Serial.println ("CAN2 OK") ;
	} else {
		Serial.print ("ERROR CAN2: 0x") ;
		Serial.println (error_code, HEX) ;
	}

	//filesystem_init();

	#ifdef WEB_INTERFACE_ENABLED
		xTaskCreate(filesystem_task, "filesystem_task", 1024*4, NULL, 1, NULL);
		xTaskCreate(web_interface_task, "web_interface_task", 10000, NULL, tskIDLE_PRIORITY, NULL);
	#endif
	
	/** BUTTONS. */
	button_init(&bms_recuperation_button_minus, millis(), 100, 100);
	button_init(&bms_recuperation_button_plus, millis(), 100, 100);
}

void loop()
{
	CANMessage frame;
	
	can1.poll();

	#ifdef USE_DUAL_MCP
	can2.poll();
	#endif

	#ifdef USE_NATIVE_CAN
	// TODO FIXME (not working in new version)
	/* if (can2.recoverFromBusOff()) {
		TWAI_MODE_REG = ACAN_ESP32_Settings::NormalMode | TWAI_RESET_MODE;
		do {
			TWAI_MODE_REG = ACAN_ESP32_Settings::NormalMode;
		} while ((TWAI_MODE_REG & TWAI_RESET_MODE) != 0);
	} */
	#endif

	if (can1.receive(frame)) {
		can_filter(&frame);
		can2.tryToSend(frame);
		
		digitalWrite(LED_PIN, led_state);
		led_state = !led_state;
	}

	if (can2.receive(frame)) {
		can_filter(&frame);
		can1.tryToSend(frame);
		
		digitalWrite(LED2_PIN, led2_state);
		led2_state = !led2_state;
	}

	static unsigned long timestamp;
	if (millis() - timestamp >= 5000) {
		static bool wifi_on = true;
		if (wifi_on && !digitalRead(13)) {
			WiFi.disconnect(true);
			WiFi.mode(WIFI_OFF);
			wifi_on = false;
		}
		
		//filesystem_update();
		timestamp += 5000;

		digitalWrite(LED_PIN, led_state);
		led_state = !led_state;
		digitalWrite(LED2_PIN, led2_state);
		led2_state = !led2_state;

		#ifdef USE_NATIVE_CAN
		/*Serial.print (" STATUS 0x");
		Serial.print (TWAI_STATUS_REG, HEX);
		Serial.print (" RXERR ");
		Serial.print (TWAI_RX_ERR_CNT_REG);
		Serial.print (" TXERR ");
		Serial.println (TWAI_TX_ERR_CNT_REG);
		Serial.print (" ACAN_ESP32::statusFlags() ");
		Serial.println (can2.statusFlags());*/
		#endif
	}
	
	bms_process_recuperation_buttons();
}
