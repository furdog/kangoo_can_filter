#include <stdio.h>
#include "kangoo_can_filter.h"
#include "fake_bms.h"

struct button b;

void button_test()
{
	/* Check initial event */
	assert(button_update(&b, 0) == BUTTON_EVENT_NONE);

	/* Check no events without press */
	assert(button_update(&b, BUTTON_PRESS_TIME_MS * 2) ==
				 BUTTON_EVENT_NONE);

	/* Check no events before press timeout */
	b.pressed = true;
	assert(button_update(&b, BUTTON_PRESS_TIME_MS - 1) ==
				 BUTTON_EVENT_NONE);

	/* Check if event after press timeout */
	assert(button_update(&b, 1)  == BUTTON_EVENT_HOLD);

	/* Event should fire only once */
	assert(button_update(&b, 1)  == BUTTON_EVENT_NONE);

	/* Check release event */
	b.pressed = false;
	assert(button_update(&b, 1)  == BUTTON_EVENT_RELEASE);

	/* Event should fire only once */
	assert(button_update(&b, 1)  == BUTTON_EVENT_NONE);
}

int main()
{
	button_init(&b);

	/* Run two times to confirm behaviour. */
	button_test();
	button_test();
	
	return 0;
}
