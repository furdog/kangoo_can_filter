#include <DNSServer.h>
#include <WiFi.h>
#include <WebServer.h>
#include <Update.h>
#include "index.gen.h"

DNSServer dns_server;
WebServer web_server(80);

/************************************************
 *  Full web page
 ***********************************************/
void send_ok()
{
	web_server.send(200);
}

/************************************************
 * Web functions
 ***********************************************/

/**
 * @brief Converts a float value to a JSON-compatible string.
 * Handles NaN (Not a Number) and Infinity by returning "null".
 * Otherwise, converts the float to a string with a specified number of decimal places.
 * * @param value The float value to convert.
 * @param decimalPlaces The number of decimal places to include for valid float numbers.
 * Defaults to 3.
 * @return A String representing the float in JSON-compatible format ("null" for NaN/Infinity).
 */
String floatToJsonString(float value, int decimalPlaces = 3) {
    if (isnan(value) || isinf(value)) {
        return "null"; // JSON does not support NaN or Infinity directly. Use "null".
    } else {
        // dtostrf(float_value, min_width, num_decimal_places, char_array_buffer);
        // min_width: Minimum width of the field (0 means no padding).
        // num_decimal_places: Number of digits after the decimal point.
        // char_array_buffer: Buffer to store the resulting string.
        char buffer[32]; // Sufficiently large buffer for most float representations
        dtostrf(value, 0, decimalPlaces, buffer);
        return String(buffer);
    }
}

void ajax_query()
{
	/** If no arguments then it means that page request is queried. */
	if (!web_server.args()) {
		web_server.sendHeader("Content-Encoding", "gzip");
		web_server.sendHeader("Content-Type", "text/html");

		web_server.send_P(200, "text/html",
				  (const char *)index_gen_html_gz,
						index_gen_html_gz_len);
		return;
	}
	
	/** First argument is always identifier of requested entity. */
	switch (web_server.arg(0).toInt()) {
	/** Send status update if 0 requested. */
	case 0: {
		String text = String("[")
		+ floatToJsonString(bms_soh, 3) + ","
		+ floatToJsonString(bms_voltage, 3) + ","
		+ floatToJsonString(bms_soc, 3) + ","
		+ floatToJsonString(bms_current, 3) + ","
		+ floatToJsonString(bms_temp, 3) + ","
		+ floatToJsonString(bms_max_input_kwt, 3) + ","
		+ floatToJsonString(bms_min_cell_v, 3) + ","
		+ floatToJsonString(bms_max_cell_v, 3) + ","
		+ floatToJsonString(bms_kwh, 3) + ","
		+ floatToJsonString(bms_max_recu_kwt, 3) + ","
		+ String(filesystem_is_corrupted())
		+ "]";
		web_server.send(200, "text/plain", text);
		break;
	}
	
	/** enable/disable error filter. */
	case 1:
		if (web_server.args() > 1) {
			bms_filtering_enabled = web_server.arg((size_t)1).toInt();
			commit_settings();
			send_ok();
		} else {
			web_server.send(200, "text/plain", String("[") + (bms_filtering_enabled ? 1 : 0) + "]");
		}
		break;

	/** filter general */
	case 2:
		if (web_server.args() > 1) {
			bms_filter_general_errors_enabled = web_server.arg((size_t)1).toInt();
			commit_settings();
			send_ok();
		} else {
			web_server.send(200, "text/plain", String("[") + (bms_filter_general_errors_enabled ? 1 : 0) + "]");
		}
		break;

	/** filter isolation */
	case 3:	
		if (web_server.args() > 1) {
			bms_filter_isolation_error_enabled = web_server.arg((size_t)1).toInt();
			commit_settings();
			send_ok();
		} else {
			web_server.send(200, "text/plain", String("[") + (bms_filter_isolation_error_enabled ? 1 : 0) + "]");
		}
		break;
		
	/** Recuperation multiplier enabled? */
	case 4:	
		if (web_server.args() > 1) {
			bms_recuperation_multiplier_enabled = web_server.arg((size_t)1).toInt();
			commit_settings();
			send_ok();
		} else {
			web_server.send(200, "text/plain", String("[") + (bms_recuperation_multiplier_enabled ? 1 : 0) + "]");
		}
		break;
		
	/** Recuperation multiplier */
	case 5:	
		if (web_server.args() > 1) {
			bms_recuperation_multiplier = web_server.arg((size_t)1).toFloat();
			commit_settings();
			send_ok();
		} else {
			web_server.send(200, "text/plain", String("[") + floatToJsonString(bms_recuperation_multiplier) + "]");
		}
		break;

	/** SOH multiplier enabled? */
	case 6:	
		if (web_server.args() > 1) {
			bms_soh_multiplier_enabled = web_server.arg((size_t)1).toInt();
			commit_settings();
			send_ok();
		} else {
			web_server.send(200, "text/plain", String("[") + (bms_soh_multiplier_enabled ? 1 : 0) + "]");
		}
		break;
		
	/** SOH multiplier */
	case 7:	
		if (web_server.args() > 1) {
			bms_soh_multiplier = web_server.arg((size_t)1).toFloat();
			commit_settings();
			send_ok();
		} else {
			web_server.send(200, "text/plain", String("[") + floatToJsonString(bms_soh_multiplier) + "]");
		}
		break;
		
	/** Custom capacity enabled? */
	case 8:	
		if (web_server.args() > 1) {
			bms_custom_capacity_enabled = web_server.arg((size_t)1).toInt();
			commit_settings();
			send_ok();
		} else {
			web_server.send(200, "text/plain", String("[") + (bms_custom_capacity_enabled ? 1 : 0) + "]");
		}
		break;
		
	/** Custom capacity */
	case 9:	
		if (web_server.args() > 1) {
			bms_custom_capacity = web_server.arg((size_t)1).toFloat();
			commit_settings();
			send_ok();
		} else {
			web_server.send(200, "text/plain", String("[") + floatToJsonString(bms_custom_capacity) + "]");
		}
		break;

	/** ubercharge enabled? */
	case 10:	
		if (web_server.args() > 1) {
			bms_ubercharge_enabled = web_server.arg((size_t)1).toInt();
			commit_settings();
			send_ok();
		} else {
			web_server.send(200, "text/plain", String("[") + (bms_ubercharge_enabled ? 1 : 0) + "]");
		}
		break;
		
	/** ubercharge limit */
	case 11:	
		if (web_server.args() > 1) {
			bms_ubercharge = web_server.arg((size_t)1).toFloat();
			commit_settings();
			send_ok();
		} else {
			web_server.send(200, "text/plain", String("[") + floatToJsonString(bms_ubercharge) + "]");
		}
		break;

	/** Manually limit charge power kwt? */
	case 12:	
		if (web_server.args() > 1) {
			bms_limit_charge_kwt_manually_enabled = web_server.arg((size_t)1).toInt();
			commit_settings();
			send_ok();
		} else {
			web_server.send(200, "text/plain", String("[") + (bms_limit_charge_kwt_manually_enabled ? 1 : 0) + "]");
		}
		break;
		
	case 13:	
		if (web_server.args() > 1) {
			bms_limit_charge_kwt_manually = web_server.arg((size_t)1).toFloat();
			commit_settings();
			send_ok();
		} else {
			web_server.send(200, "text/plain", String("[") + floatToJsonString(bms_limit_charge_kwt_manually) + "]");
		}
		break;
	
	/** Emulate custom kwt counter? */
	case 15:	
		if (web_server.args() > 1) {
			bms_kwt_counter_enabled = web_server.arg((size_t)1).toInt();
			commit_settings();
			send_ok();
		} else {
			web_server.send(200, "text/plain", String("[") + (bms_kwt_counter_enabled ? 1 : 0) + "]");
		}
		break;
		
	case 16:	
		if (web_server.args() > 1) {
			bms_kwt_counter = web_server.arg((size_t)1).toFloat();
			commit_settings();
			send_ok();
		} else {
			web_server.send(200, "text/plain", String("[") + floatToJsonString(bms_kwt_counter) + "]");
		}
		break;

	case 18:	
		if (web_server.args() > 1) {
			fake_bms_enabled = web_server.arg((size_t)1).toInt();
			commit_settings();
			send_ok();
		} else {
			web_server.send(200, "text/plain", String("[") + (fake_bms_enabled ? 1 : 0) + "]");
		}
		break;

	/** restart. */
	case 17:
		ESP.restart();
		send_ok();
		break;
		
	/** Stop wifi. */
	case 14:
		/** If more args */
		if (web_server.args() > 1) {
			WiFi.disconnect(true);
			WiFi.mode(WIFI_OFF);
		}
		
		send_ok();
		break;

	default:
		send_ok();
		break;
	}
}

/************************************************
 * OTA
 ***********************************************/
#include <Ticker.h>

Ticker restart_ticker;
bool rapid_blink = false;

void safe_restart()
{
	ESP.restart();
}

void download_firmware()
{
	uint32_t free_space = (ESP.getFreeSketchSpace() - 0x1000) & 0xFFFFF000;
	HTTPUpload& upload = web_server.upload();
	
	if (upload.status == UPLOAD_FILE_START) {
		if (Update.begin(free_space)) {
			Serial.println("Update begin success");
		} {
			Serial.println("Update begin fail");
		}
	} else if (upload.status == UPLOAD_FILE_WRITE) {
		if (Update.write(upload.buf, upload.currentSize))
			Serial.println("Update write ok...");
	} else if(upload.status == UPLOAD_FILE_END) {
		if (Update.end(true)) {
			Serial.println("Update end success");
			send_ok();
			rapid_blink = true;
			restart_ticker.once(1.0, safe_restart);
		} else {
			Serial.println("Update end fail");
		}
	}
}

/************************************************
 * Main functions
 ***********************************************/
void web_interface_init()
{
	//WiFi.mode(WIFI_AP_STA);
	WiFi.mode(WIFI_AP);
	//WiFi.config(IPAddress(192, 168, 1, 37), IPAddress(192, 168, 1, 1), IPAddress(255, 255, 255, 0));
	//WiFi.begin("netis", "novaservis");
	WiFi.softAPConfig(IPAddress(7, 7, 7, 7), IPAddress(7, 7, 7, 7), IPAddress(255, 255, 255, 0));
	WiFi.softAP("CanBOX");

	web_server.on("/", ajax_query);
	web_server.on("/update", HTTP_POST, send_ok, download_firmware);
	web_server.onNotFound(ajax_query);
	
	dns_server.start(53, "*", WiFi.softAPIP());  
	web_server.begin();
}

void web_interface_update()
{
	dns_server.processNextRequest();
	web_server.handleClient();
}
