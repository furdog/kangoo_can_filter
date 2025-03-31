#include <stdint.h>
#include <stdbool.h>

enum kangoo_manual_charge_event {
	KANGOO_MANUAL_CHARGE_EVENT_NONE,
	KANGOO_MANUAL_CHARGE_BEGIN,
	KANGOO_MANUAL_CHARGE_END
};

enum kangoo_manual_charge_state {
	KANGOO_MANUAL_CHARGE_STATE_IDLE,
	KANGOO_MANUAL_CHARGE_STATE_CHARGING
};

struct kangoo_manual_charge {
	float bms_max_cell_v;
	float bms_ubercharge;
	float bms_limit_charge_kwt_manually;
	bool bms_charger_plugged_in;
	bool bms_limit_charge_kwt;
	bool bms_limit_charge_kwt_manually_enabled;
	bool bms_ubercharge_active;
	
	float max_kwt_limit;
};

void kangoo_manual_charge_set_max_kwt_limit(struct kangoo_manual_charge *self,
					    const float limit)
{
	self->max_kwt_limit = limit;
}

bool kangoo_manual_charge_is_charging(struct kangoo_manual_charge *self)
{
	return false;
}

enum kangoo_manual_charge_event kangoo_manual_charge_update(
					     struct kangoo_manual_charge *self)
{
	if (bms_max_cell_v < bms_ubercharge && bms_charger_plugged_in)
		result = true;
		
	if (bms_max_cell_v > bms_ubercharge)
		bms_limit_charge_kwt -= 0.05;
	else
		bms_limit_charge_kwt += 0.05;
	
	/** Set bounds. */					
	if (bms_limit_charge_kwt < 0.0) {
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
