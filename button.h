#include <stdbool.h>
#include <time.h>

/** List of events that can be emited. */
enum button_event_type {
	BUTTON_EVENT_TYPE_NONE,
	BUTTON_EVENT_TYPE_HOLD_TIMEOUT,
	BUTTON_EVENT_TYPE_RELEASE_TIMEOUT
};

/** List of possible button states. */
enum button_state {
	BUTTON_STATE_WAIT_FOR_HOLD,
	BUTTON_STATE_WAIT_FOR_RELEASE
};

/** Main instance. */
struct button {
	enum button_state state;

	clock_t hold_timeout;
	clock_t release_timeout;
	clock_t press_timestamp;
};

/** 
 * Set button intervals manually.
 * 
 * @param self pointer to main instance.
 * @param hold_timeout how long button should held until timeout.
 * @param release_timeout how long should button be released until timeout.
 */
void button_set_intervals(struct button *self,
			  const clock_t hold_timeout,
			  const clock_t release_timeout)
{
	self->hold_timeout = hold_timeout;
	self->release_timeout = release_timeout;
}

/** 
 * Init everything by default before start.
 * 
 * @param self a pointer to main instance.
 * @param timestamp current time in milliseconds.
 * @param hold_timeout how long button should held until timeout.
 * @param release_timeout how long should button be released until timeout.
 */
void button_init(struct button *self, const clock_t timestamp,
		 const clock_t hold_timeout,
		 const clock_t release_timeout)
{
	self->state = BUTTON_STATE_WAIT_FOR_HOLD;
	
	self->hold_timeout = hold_timeout;
	self->release_timeout = release_timeout;
	self->press_timestamp = timestamp;
}

/** 
 * Update main instance cycle.
 * 
 * @param self a pointer to main instance.
 * @param timestamp current time in milliseconds.
 * @param pressed_physically tells if button is pressed physically
 * @return event
 */
enum button_event_type button_update(struct button *self,
				     const clock_t timestamp,
				     const bool pressed_physically)
{
	enum button_event_type event = BUTTON_EVENT_TYPE_NONE;
	
	switch (self->state) {
	case BUTTON_STATE_WAIT_FOR_HOLD:
		if (!pressed_physically) {
			self->press_timestamp = timestamp;
			break;
		}

		/** Break if button not exceed hold timeout. */
		if (timestamp - self->press_timestamp < self->hold_timeout)
			break;
		
		event = BUTTON_EVENT_TYPE_HOLD_TIMEOUT;
		
		self->state = BUTTON_STATE_WAIT_FOR_RELEASE;
		self->press_timestamp = timestamp;
		break;
	
	case BUTTON_STATE_WAIT_FOR_RELEASE:
		if (pressed_physically) {
			self->press_timestamp = timestamp;
			break;
		}

		/** Break if button not exceed release timeout. */
		if (timestamp - self->press_timestamp < self->release_timeout)
			break;

		event = BUTTON_EVENT_TYPE_RELEASE_TIMEOUT;
		
		self->state = BUTTON_STATE_WAIT_FOR_HOLD;
		self->press_timestamp = timestamp;
		break;
	
	default:
		break;
	}
	
	return event;
}
