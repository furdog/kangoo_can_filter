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


struct kangoo_fake_bms fbms;

void fake_bms_test_start(struct kangoo_fake_bms *self)
{
	uint16_t i;
	struct kangoo_fake_bms_can_frame frame;

	kangoo_fake_bms_start(self);

	frame.id = 0x423U;
	frame.len = 1;
	frame.data[0] = 0x07U;
	kangoo_fake_bms_can_frame_write(self, frame);

	/* Now test message reception from every frame */
	for (i = 0; i < 9000; i++) {
		kangoo_fake_bms_update(self, 1);

		if ((i % 10) == 0 && i != 0) {		
			assert(kangoo_fake_bms_can_frame_read(self) != NULL);
			assert(kangoo_fake_bms_can_frame_read(self) == NULL);

			/* Update immediately, so we can proccess other
			 * message. */
			kangoo_fake_bms_update(self, 0);
		}

		if ((i % 100) == 0 && i != 0) {
			assert(kangoo_fake_bms_can_frame_read(self) != NULL);
			assert(kangoo_fake_bms_can_frame_read(self) != NULL);
			assert(kangoo_fake_bms_can_frame_read(self) != NULL);
			assert(kangoo_fake_bms_can_frame_read(self) == NULL);

			kangoo_fake_bms_update(self, 0);
		}

		if ((i % 3000) == 0 && i != 0) {
			assert(kangoo_fake_bms_can_frame_read(self) != NULL);
			assert(kangoo_fake_bms_can_frame_read(self) == NULL);
		
			kangoo_fake_bms_update(self, 0);
		}
	}
}

void fake_bms_test()
{
	uint16_t i;

	kangoo_fake_bms_init(&fbms);

	kangoo_fake_bms_update(&fbms, 0);

	/* Bms not yet started, no messages should be received */
	assert(kangoo_fake_bms_can_frame_read(&fbms) == NULL);

	/* Not even after delay */
	kangoo_fake_bms_update(&fbms, 300);
	assert(kangoo_fake_bms_can_frame_read(&fbms) == NULL);

	fake_bms_test_start(&fbms);

	/* Now test stop */
	kangoo_fake_bms_stop(&fbms);
		
	for (i = 0; i < 9000; i++) {
		kangoo_fake_bms_update(&fbms, 1);

		assert(kangoo_fake_bms_can_frame_read(&fbms) == NULL);
	}

	/* Test start again */
	fake_bms_test_start(&fbms);
}

int main()
{
	button_init(&b);

	/* Run two times to confirm behaviour. */
	button_test();
	button_test();

	fake_bms_test();

	return 0;
}
