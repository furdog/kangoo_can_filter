#include "LittleFS.h"
#include "target.gen.h"

bool settings_committed = false;
bool filesystem_corrupted = false;

struct settings {
	bool  bms_filtering_enabled;
	bool  bms_filter_general_errors_enabled;
	bool  bms_filter_isolation_error_enabled;
	bool  bms_recuperation_multiplier_enabled;
	float bms_recuperation_multiplier;
	bool  bms_soh_multiplier_enabled;
	float bms_soh_multiplier;
	bool  bms_custom_capacity_enabled;
	float bms_custom_capacity;
	bool  bms_ubercharge_enabled;
	float bms_ubercharge;
	bool  bms_limit_charge_kwt_manually_enabled;
	float bms_limit_charge_kwt_manually;
	bool  bms_kwt_counter_enabled;
	float bms_kwt_counter;
	bool  fake_bms_enabled;
};

void save_settings()
{
	struct settings settings;
	
	/** TODO, import struct instead of this */
	settings.bms_filtering_enabled                 = bms_filtering_enabled;
	settings.bms_filter_general_errors_enabled     = bms_filter_general_errors_enabled;
	settings.bms_filter_isolation_error_enabled    = bms_filter_isolation_error_enabled;
	settings.bms_recuperation_multiplier_enabled   = bms_recuperation_multiplier_enabled;
	settings.bms_recuperation_multiplier           = bms_recuperation_multiplier;
	settings.bms_soh_multiplier_enabled            = bms_soh_multiplier_enabled;
	settings.bms_soh_multiplier                    = bms_soh_multiplier;
	settings.bms_custom_capacity_enabled           = bms_custom_capacity_enabled;
	settings.bms_custom_capacity                   = bms_custom_capacity;
	settings.bms_ubercharge_enabled                = bms_ubercharge_enabled;
	settings.bms_ubercharge                        = bms_ubercharge;
	settings.bms_limit_charge_kwt_manually_enabled = bms_limit_charge_kwt_manually_enabled;
	settings.bms_limit_charge_kwt_manually         = bms_limit_charge_kwt_manually;
	settings.bms_kwt_counter_enabled               = bms_kwt_counter_enabled;
	settings.bms_kwt_counter                       = bms_kwt_counter;
	settings.fake_bms_enabled                      = fake_bms_enabled;
	
	printf("saving...\n");
	File f = LittleFS.open("/settings.bin", "w");
	if (!f) {
		printf("failed opening '/settings.bin' for write\n");
		return;
	}
	f.write((uint8_t*)&settings, sizeof(struct settings));
	f.close();
}

void save_version_on_first_start() 
{
	if (LittleFS.exists("/version.txt")) {
		// Open for reading
		File f = LittleFS.open("/version.txt", "r");
		if (f) {
			String current_ver = f.readString();
			printf("System already initialized."
			       " Current version: %s\n", current_ver.c_str());
			f.close();
		}
		return; 
	}

	printf("First start detected. Writing version: %s\n", __CAN_FILTER_VERSION__);
    
	File f = LittleFS.open("/version.txt", "w");
	if (!f) {
		printf("Failed to create version file\n");
		return;
	}
    
	f.print(__CAN_FILTER_VERSION__); 
	f.close();
}

void load_settings()
{
	struct settings settings;
	
	/** First run. */
	if (!LittleFS.exists("/settings.bin"))
		save_settings();

	save_version_on_first_start();
	
	printf("loading...\n");
	File f = LittleFS.open("/settings.bin", "r");
	if (!f) {
		printf("failed opening '/settings.bin' for read\n");
		return;
	}
	
	f.read((uint8_t*)&settings, sizeof(struct settings));
	
	bms_filtering_enabled                 = settings.bms_filtering_enabled;
	bms_filter_general_errors_enabled     = settings.bms_filter_general_errors_enabled;
	bms_filter_isolation_error_enabled    = settings.bms_filter_isolation_error_enabled;
	bms_recuperation_multiplier_enabled   = settings.bms_recuperation_multiplier_enabled;
	bms_recuperation_multiplier           = settings.bms_recuperation_multiplier;
	bms_soh_multiplier_enabled            = settings.bms_soh_multiplier_enabled;
	bms_soh_multiplier                    = settings.bms_soh_multiplier;
	bms_custom_capacity_enabled           = settings.bms_custom_capacity_enabled;
	bms_custom_capacity                   = settings.bms_custom_capacity;
	bms_ubercharge_enabled                = settings.bms_ubercharge_enabled;
	bms_ubercharge                        = settings.bms_ubercharge;
	bms_limit_charge_kwt_manually_enabled = settings.bms_limit_charge_kwt_manually_enabled;
	bms_limit_charge_kwt_manually         = settings.bms_limit_charge_kwt_manually;
	bms_kwt_counter_enabled               = settings.bms_kwt_counter_enabled;
	bms_kwt_counter                       = settings.bms_kwt_counter;
	fake_bms_enabled                      = settings.fake_bms_enabled;
	
	f.close();
}

void commit_settings()
{
	settings_committed = true;
}

bool filesystem_is_corrupted()
{
	return filesystem_corrupted;
}

void filesystem_init()
{
	if(!LittleFS.begin(true)) {
		printf("failed to mount filesystem (corrupted)\n");
		filesystem_corrupted = true;
	}
	
	load_settings();
}

void filesystem_update()
{
	if (filesystem_is_corrupted()) {
		printf("failed to update filesystem (corrupted)\n");
		return;
	}
	
	if (settings_committed) {
		save_settings();
		settings_committed = false;
	}
}
