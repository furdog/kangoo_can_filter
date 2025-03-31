#include <time.h>
#include <stdio.h>
#include <stdbool.h>

#define MSECS_IN_HOUR (1000 * 60 * 60)

/** Timers */
clock_t timestamp = 0;

static clock_t bms_ignition_key_timestamp = 0;
static int bms_ignition_key_counter = 0;
static bool bms_ignition_key = false;

void bms_wifi_reset_sequence(bool key)
{	
	/** Reset key counter if key was not turned for too long */
	if (timestamp - bms_ignition_key_timestamp > 2000)
		bms_ignition_key_counter = 0;
	
	/** If the ignition key state remain the same - exit. */
	if (bms_ignition_key != key)
		bms_ignition_key = key;
	else
		return;
	
	/** If key not turned on - exit. */
	if (!key)
		return;
	
	/** Reset timestamp and increment key turn counter. */		
	bms_ignition_key_timestamp = timestamp;
	bms_ignition_key_counter++;
	
	/** If ignition key was turned 5 times,
	 *  with less or equal than 2000ms intervals - reset wifi. */
	if (bms_ignition_key_counter >= 5) {
		bms_ignition_key_counter = 0;
		printf("esp_restart\n");
	}
}

/** Vehicle params. */
void test_ignition_key()
{
	timestamp = 0;
	
	for (timestamp = 0; timestamp <= 2000 * 5; timestamp += 10)
	{
		bms_wifi_reset_sequence(timestamp % 2000 == 0);
	}
}

int main()
{		
	test_ignition_key();
}
