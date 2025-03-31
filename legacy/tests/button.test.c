#include <time.h>
#include <stdio.h>

/** Timers */
clock_t timestamp = 0;
#include "../button.h"

struct button btn;

void button_test()
{
	enum button_event_type event;
	
	button_init(&btn, timestamp, 100, 200);
	
	/** If we hold button for 500 msecs, then BUTTON_EVENT_TYPE_HOLD_TIMEOUT
	 *  should be triggered at 100msec */
	for (timestamp = 0; timestamp <= 500; timestamp++)
	{
		event = button_update(&btn, timestamp, true);
		
		if (event == BUTTON_EVENT_TYPE_HOLD_TIMEOUT)
			printf("event hold timeout at %lums\n", timestamp);
	}
	
	/** If we release button for 500 msecs, then BUTTON_EVENT_TYPE_RELEASE_TIMEOUT
	 *  should be triggered at 700 msec */
	for (timestamp = 500; timestamp <= 1000; timestamp++)
	{
		event = button_update(&btn, timestamp, false);
		
		if (event == BUTTON_EVENT_TYPE_RELEASE_TIMEOUT)
			printf("event release timeout at %lums\n", timestamp);
	}
}

int main()
{
	button_test();
	
	return 0;
}
