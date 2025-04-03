/******************************************************************************
 * HEADERS
 *****************************************************************************/
#include <stdbool.h>
#include <stdalign.h>
#include <stdint.h>
#include <assert.h>
#include <time.h>

/******************************************************************************
 * PHYSICAL BUTTON ABSTRACTION
 *****************************************************************************/
#define BUTTON_SHORT_PRESS_TIME_MS 50
#define BUTTON_LONG_PRESS_TIME_MS  2000

enum button_event {
	BUTTON_EVENT_NONE,
	BUTTON_EVENT_SHORT_PRESS,
	BUTTON_EVENT_LONG_PRESS,
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

bool button_is_long_pressed(struct button *self)
{
	return self->_state == (uint8_t)1;
}

enum button_event button_update(struct button *self, clock_t delta_time_ms)
{		
	switch (self->_state) {
	case 0: default: /* Button not yet pressed */
		if (self->pressed)
			self->_timer_ms += delta_time_ms;
		
		if (self->_timer_ms >= (uint16_t)BUTTON_LONG_PRESS_TIME_MS) {
			self->_state = 1;
			return BUTTON_EVENT_LONG_PRESS;
		}

		/* Do not exec code below if button is being held */
		if (self->pressed)
			break;

		if (self->_timer_ms >= (uint16_t)BUTTON_SHORT_PRESS_TIME_MS) {
			self->_state = 1;
			return BUTTON_EVENT_SHORT_PRESS;
		}
		
		self->_timer_ms = 0;
		
		break;
	case 1: /* Wait for release */
		if (self->pressed)
			break;
		
		self->_state    = 0;
		self->_timer_ms = 0;
		return BUTTON_EVENT_RELEASE;
	}
	
	return BUTTON_EVENT_NONE;
}

/******************************************************************************
 * MONITORED VARIABLES ABSTRACTION
 *****************************************************************************/
struct mon_var_header {
	const char *name;

	bool    _changed;
	uint8_t _len;
};

void mon_var_change(void *self)
{
	struct mon_var_header *hdr = self;
	
	hdr->_changed = true;
}

/* Simple storage for monitored variables. */
struct mon_var_storage {
	uint8_t _state; /* FSM state */

	struct mon_var_header *_it; /* Iterator */
	
	uint8_t *_buffer;
	size_t   _capacity; /* Total buffer capacity */
	size_t   _size;     /* Current buffer size */
};

void mon_var_storage_init(struct mon_var_storage *self,
			  void *buf, size_t buf_size)
{
	self->_state = 0;
	
	self->_it = NULL;

	self->_buffer   = (uint8_t*)buf;
	self->_capacity = buf_size;
	self->_size     = 0;
}

void *mon_var_storage_alloc(struct mon_var_storage *self, uint8_t size)
{
	struct mon_var_header *hdr;

	uint8_t align = alignof(struct mon_var_header);
	uint8_t aligned_size = (size + (align - (uint8_t)1)) & 
			     (uint8_t)~(align - (uint8_t)1);

	assert(self->_size + size <= self->_capacity);

	/* Get header address */
	hdr = (struct mon_var_header *)(self->_buffer + self->_size);

	/* Initialize header */
	hdr->name     = NULL;
	hdr->_changed = false;
	hdr->_len     = size;

	/* Update storage size */
	self->_size += size;

	/* Return header address */
	return hdr;
}

/* Iterates storage and returns pointer to mon_var_header if it was changed. */
void *mon_var_storage_update(struct mon_var_storage *self)
{
	struct mon_var_header *changed_var = NULL;

	switch (self->_state) {
	default: case 0: /* INITIAL */
		if (self->_size == (size_t)0)
			break;

		self->_it = (struct mon_var_header *)self->_buffer;
		self->_state = 1;

		break;
	
	case 1:
		/* Go to the next mon_var element */
		self->_it = (struct mon_var_header *)(
			(uint8_t *)self->_it + self->_it->_len);

		/* If element is invalid (lies beyond allocated space) */
		if ((uint8_t *)self->_it >= (self->_buffer + self->_size)) {
			/* Set element as first */
			self->_it = (struct mon_var_header *)self->_buffer;
		}

		if (self->_it->_changed) {
			changed_var = self->_it;
			self->_it->_changed = false;
		}

		break;
	}

	return changed_var;
}
