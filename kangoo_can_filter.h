/******************************************************************************
 * HEADERS
 *****************************************************************************/
#include <stdbool.h>
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
	return (self->_state == 1);
}

enum button_event button_update(struct button *self, clock_t delta_time_ms)
{		
	switch (self->_state) {
	case 0: default: /* Button not yet pressed */
		if (self->pressed)
			self->_timer_ms += delta_time_ms;
		
		if (self->_timer_ms >= BUTTON_LONG_PRESS_TIME_MS) {
			self->_state = 1;
			return BUTTON_EVENT_LONG_PRESS;
		}

		/* Do not exec code below if button is being held */
		if (self->pressed)
			break;

		if (self->_timer_ms >= BUTTON_SHORT_PRESS_TIME_MS) {
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
/* Monitored variables. */
#define MON_VAR_DATA_SIZE_YET_UNDEFINED 1

enum mon_var_type {
	MON_VAR_TYPE_UNDEFINED,

	MON_VAR_TYPE_INT,
	MON_VAR_TYPE_FLOAT,
	MON_VAR_TYPE_STRING
};

const uint8_t mon_var_type_size[] = {
	0,
	sizeof(int),
	sizeof(float),
	-1
};

struct mon_var {
	bool    _changed;
	uint8_t _len;

	const char       *name;
	enum mon_var_type type;
	uint8_t           flags;   /* User specified flags. */

	uint8_t data[MON_VAR_DATA_SIZE_YET_UNDEFINED];
};

void mon_var_init(struct mon_var *self)
{
	self->_changed = false;
	self->_len     = 0;
	
	self->name  = NULL;
	self->type  = MON_VAR_TYPE_UNDEFINED;
	self->flags = 0;
}

/* Simple storage for monitored variables. */
struct mon_var_storage {
	uint8_t _state; /* FSM state */

	struct mon_var *_it; /* Iterator */
	
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

struct mon_var *mon_var_storage_alloc(struct mon_var_storage *self,
				      uint8_t data_size)
{
	struct mon_var *var;

	size_t total_data_size = sizeof(struct mon_var) - 1 + data_size;

	assert(self->_size + total_data_size <= self->_capacity);

	/* Get variable address */
	var = (struct mon_var *)(self->_buffer + self->_size);

	/* Initialize variable */
	mon_var_init(var);

	/* Update storage size */
	self->_size += total_data_size;

	return var;
}

/* Iterates storage and returns pointer to mon_var if it was changed. */
struct mon_var *mon_var_storage_update(struct mon_var_storage *self)
{
	struct mon_var *changed_var = NULL;
	
	switch (self->_state) {
	default: case 0: /* INITIAL */
		if (self->_size == 0)
			break;

		self->_it = (struct mon_var *)self->_buffer;
		self->_state = 1;

		break;
		
	case 1:
		/* Go to the next mon_var element */
		self->_it = (struct mon_var *)(
			(uint8_t *)self->_it + sizeof(struct mon_var) - 1 +
								self->_it->_len
		);
	
		/* If element is invalid (lies beyond allocated space) */
		if ((uint8_t *)self->_it > (self->_buffer + self->_capacity)) {
			/* Set element as first */
			self->_it = (struct mon_var *)self->_buffer;
		}

		if (self->_it->_changed) {
			changed_var = self->_it;
			self->_it->_changed = false;
		}

		break;
	}
	
	return changed_var;
}
