/******************************************************************************
 * HEADERS
 *****************************************************************************/
#include <stdbool.h>
#include <stdalign.h>
#include <stdint.h>
#include <assert.h>

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
