/******************************************************************************
 * HEADERS
 *****************************************************************************/
#include <stdbool.h>
#include <stdalign.h>
#include <stdint.h>
#include <assert.h>

#include "kangoo_can_frame.h"
#include "kangoo_fake_bms.h"

/******************************************************************************
 * CAN TOOLS
 *****************************************************************************/

/******************************************************************************
 * PHYSICAL BUTTON ABSTRACTION
 *****************************************************************************/
#define BUTTON_PRESS_TIME_MS 50

enum button_event {
	BUTTON_EVENT_NONE,
	BUTTON_EVENT_HOLD,
	BUTTON_EVENT_RELEASE
};

struct button
{
	uint8_t  _state;
	uint16_t _timer_ms;

	bool pressed;
};

void button_init(struct button *self)
{
	self->_state    = 0;
	self->_timer_ms = 0;
	
	self->pressed  = false;
}

enum button_event button_update(struct button *self, uint32_t delta_time_ms)
{
	enum button_event ev = BUTTON_EVENT_NONE;

	switch (self->_state) {
	default: /* Button not yet pressed */
	case 0:
		if (self->pressed) {
			self->_timer_ms += delta_time_ms;
		} else {
			self->_timer_ms = 0;
		}
		
		if (self->_timer_ms >= (uint16_t)BUTTON_PRESS_TIME_MS) {
			self->_state = 1;
			ev = BUTTON_EVENT_HOLD;
			break;
		}
		
		break;
	case 1:	/* Wait for release */
		if (self->pressed) {
			break;
		}
		
		self->_state    = 0;
		self->_timer_ms = 0;
		ev = BUTTON_EVENT_RELEASE;
		break;
	}
	
	return ev;
}

/******************************************************************************
 * CAN FILTER ABSTRACTION
 *****************************************************************************/
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

struct kangoo_can_filter_settings
{
	/* Filter errors */
	bool  filt_major_err_en; /* Filter Major errors */
	bool  filt_minor_err_en; /* Filter Minor minor errors */
	bool  filt_insul_err_en; /* Filter Insulation error */

	/* Recuperation */
	bool  recup_mul_en;
	float recup_mul;

	/* Soh control */
	bool  soh_mul_en;
	float soh_mul;

	/* Custom total capacity */
	bool  custom_cap_en;
	float custom_cap_kwh;

	/* Ubercharge */
	bool  ubercharge_en;
	float ubercharge_V; /* Volts */

	/* Charge limits */
	bool  lim_chg_kwt_en;
	float lim_chg_kwt;

	bool  custom_chg_counter_en; /* Emulate charge counter */
};

struct kangoo_can_filter_runtime {
	/* Variables being read from BMS */
	float temp_C;

	float voltage_V;
	float current_A;

	float min_cell_V;
	float max_cell_V;

	float soc_PCT; /* 0% - 100% */
	float soh_PCT; /* 0% - 100% */
	float kwh;     /* Current charge capacity */

	float max_chg_kwt;
	float max_recu_kwt;

	bool ign_key; /* Ignition key */
	bool chg_plugged;

	uint8_t diag_code; /* Diagnostics code read */

	/* Implementation specific variables */
	float    custom_chg_counter_kwh;
	uint32_t custom_chg_counter_delta_ms;

	uint32_t ign_key_delta_ms;
	int      ign_key_counter;

	float lim_chg_kwt;

	bool ubercharge_active; /* Do we perform ubercharge now? */

	uint8_t diag_counter; /* Multi-message counter */
};

struct kangoo_can_filter {
	struct kangoo_can_filter_settings _settings;
	struct kangoo_can_filter_runtime  _runtime;
	struct kangoo_can_frame           _frame;

	bool reset_wifi;
};

void kangoo_can_filter_init(struct kangoo_can_filter *self)
{
	struct kangoo_can_filter_settings *s = &self->_settings;
	struct kangoo_can_filter_runtime  *r = &self->_runtime;
	struct kangoo_can_frame           *f = &self->_frame;

	/* Initial settings (first run) */
	s->filt_major_err_en = BMS_FILTERING_ENABLED;
	s->filt_minor_err_en = BMS_FILTER_GENERAL_ERRORS_ENABLED;
	s->filt_insul_err_en = BMS_FILTER_ISOLATION_ERROR_ENABLED;

	s->recup_mul_en = BMS_RECUPERATION_MULTIPLIER_ENABLED;
	s->recup_mul    = BMS_RECUPERATION_MULTIPLIER;

	s->soh_mul_en = BMS_SOH_MULTIPLIER_ENABLED;
	s->soh_mul    = BMS_SOH_MULTIPLIER;

	s->custom_cap_en  = BMS_CUSTOM_CAPACITY_ENABLED;
	s->custom_cap_kwh = BMS_CUSTOM_CAPACITY;

	s->ubercharge_en = BMS_UBERCHARGE_ENABLED;
	s->ubercharge_V  = BMS_UBERCHARGE;

	s->lim_chg_kwt_en = BMS_LIMIT_CHARGE_KWT_MANUALLY_ENABLED;
	s->lim_chg_kwt    = BMS_LIMIT_CHARGE_KWT_MANUALLY;

	s->custom_chg_counter_en = BMS_KWT_COUNTER_ENABLED;

	/* Runtime settings */
	r->temp_C = 0.0f;

	r->voltage_V = 0.0f;
	r->current_A = 0.0f;

	r->min_cell_V = 0.0f;
	r->max_cell_V = 0.0f;

	r->soc_PCT = 0.0f;
	r->soh_PCT = 0.0f;
	r->kwh     = 0.0f;

	r->max_chg_kwt  = 0.0f;
	r->max_recu_kwt = 0.0f;

	r->ign_key     = false;
	r->chg_plugged = false;

	r->diag_code = 0U;

	r->custom_chg_counter_kwh      = 0.0f;
	r->custom_chg_counter_delta_ms = 0U;

	r->ign_key_delta_ms = 0U;
	r->ign_key_counter  = 0U;

	r->lim_chg_kwt = 0.0f;

	r->ubercharge_active = false; /* Do we perform ubercharge now? */

	r->diag_counter = 0U; /* Multi-message counter */

	f->len = -1;

	/* Other settings */
	self->reset_wifi = false;
}

void kangoo_can_filter_update(struct kangoo_can_filter *self)
{
	/*struct kangoo_can_filter_settings *s = &self->_settings;
	struct kangoo_can_filter_runtime    *r = &self->_runtime;*/
	struct kangoo_can_frame             *f = &self->_frame;
	
	switch (f->id) {
	case 0x155:
		/*r->voltage_V   = (f->data[6] << 8 | f->data[7]) / 2;
		r->current_A   = (((f->data[1] & 0x0F) << 8 | f->data[2]) - 0x7D0) / 4.0;
		r->max_chg_kwt = (f->data[0] / 3.0);
		r->soc_PCT     = (((uint16_t)f->data[4] << 8) | f->data[5]) / 400.0f;*/
		
		/* If filtering major and minor errors */
		/*if (s->filt_major_err_en && s->filt_minor_err_en) {
			f->data[3] = 0x54;
		}*/

		/* If manual charhing limits enabled */
		/*if (s->lim_chg_kwt_en) {
			f->data[0] = s->lim_chg_kwt * 3;
		}*/
	
		/* Limit by ubercharge */
		/*if (s->ubercharge_en && r->ubercharge_active) {
			f->data[0] = r->lim_chg_kwt * 3;
		}*/
	
		/*if (s->custom_cap_en) {
			r->soc_PCT = (r->kwh / s->custom_cap_kwh) * 100.0;

			f->data[4] = (uint16_t)(r->soc_PCT * 400.0f) >> 8 & 0x00FF;
			f->data[5] = (uint16_t)(r->soc_PCT * 400.0f) >> 0 & 0x00FF;
		}*/
		break;

	default:
		break;
	}
}

/******************************************************************************
 * AUTOMATIC FAKE BMS ACTIVATION
 *****************************************************************************/
#define KANGOO_CAN_FILTER_FAKE_BMS_START_TIME_MS 5000U

struct kangoo_can_filter_fake_bms {
	struct kangoo_fake_bms fbms;

	uint8_t _state;

	uint32_t _real_bms_last_seen_timeout_ms;
};

void kangoo_can_filter_fake_bms_init(struct kangoo_can_filter_fake_bms *self)
{
	kangoo_fake_bms_init(&self->fbms);

	self->_state = 0U;

	self->_real_bms_last_seen_timeout_ms = 0U;
}

void kangoo_can_filter_fake_bms_report_real_bms_message_triggered(
			struct kangoo_can_filter_fake_bms *self)
{
	/* Reset timer (prevents fake bms start due to timeout) */
	self->_real_bms_last_seen_timeout_ms = 0U;
}

void kangoo_can_filter_fake_bms_update(
				struct kangoo_can_filter_fake_bms *self,
				uint32_t delta_time_ms)
{
	kangoo_fake_bms_update(&self->fbms, delta_time_ms);

	switch (self->_state) {
	case 0U: /* Check bms presence (increment timer until timeout) */
		if (self->_real_bms_last_seen_timeout_ms >=
		    KANGOO_CAN_FILTER_FAKE_BMS_START_TIME_MS) {
			kangoo_fake_bms_start(&self->fbms);
			self->_state = 1;
		} else {
			self->_real_bms_last_seen_timeout_ms += delta_time_ms;
		}
		
		break;
	
	case 1U:
		if (self->_real_bms_last_seen_timeout_ms == 0U) {
			kangoo_fake_bms_stop(&self->fbms);
			self->_state = 0U;
		}
		
		break;

	default:
		assert(0U);
		break;
	}
}
