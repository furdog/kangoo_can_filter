#ifndef KANGOO_FAKE_BMS_GUARD
#define KANGOO_FAKE_BMS_GUARD

#include "kangoo_data_extractor.h"

/* Kangoo has 2 bms, this code only emulates second bms messages
 * see kango_fake_bms_original.h for both bms messages
 * WARNING: needs heavy refactoring, so both bms can be emulated separately */

enum kangoo_fake_bms_state
{
	KANGOO_FAKE_BMS_STATE_NONE,
	KANGOO_FAKE_BMS_STATE_START,
	KANGOO_FAKE_BMS_STATE_STOP,
	KANGOO_FAKE_BMS_STATE_SEND_MESSAGES,
	KANGOO_FAKE_BMS_STATE_WAIT_CONTROL_MSG
};

struct kangoo_fake_bms {
	uint8_t _state;

	uint8_t _bms_control_msg;
	uint8_t _lbc_key_answer;
	uint8_t _lbc2_key_answer;
	uint8_t _OT;

	/* x424*/
	int16_t _max_batt_temp_C;

	/* x425*/
	float _highest_cell_V;
	float _lowest_cell_V;

	uint32_t _msg_0_timer_ms;
	uint32_t _msg_1_timer_ms;
	uint32_t _msg_2_timer_ms;

	uint8_t _frames_available;
	struct kangoo_can_frame _input;
	struct kangoo_can_frame _frames[3];
};

/******************************************************************************
 * PRIVATE
 *****************************************************************************/
void _kangoo_fake_bms_start_communication(struct kangoo_fake_bms *self)
{
	if (self->_state != (uint8_t)KANGOO_FAKE_BMS_STATE_SEND_MESSAGES) {
		self->_msg_0_timer_ms = 0U;
		self->_msg_1_timer_ms = 0U;
		self->_msg_2_timer_ms = 0U;

		self->_frames_available = 0U;
		self->_input.len     = -1;
		self->_frames[0].len = -1;
		self->_frames[1].len = -1;
		self->_frames[2].len = -1;

		self->_state = KANGOO_FAKE_BMS_STATE_SEND_MESSAGES;
	}
}

void _kangoo_fake_bms_stop_communication(struct kangoo_fake_bms *self)
{
	self->_frames_available = 0U;

	self->_state = KANGOO_FAKE_BMS_STATE_WAIT_CONTROL_MSG;
}

void _kangoo_fake_bms_parse_control_msg(struct kangoo_fake_bms *self)
{
	switch (self->_bms_control_msg) {
	case 0x07U:
		_kangoo_fake_bms_start_communication(self);
		break;
		
	case 0x04U:
		/* Start fake bms communication */
		_kangoo_fake_bms_start_communication(self);
		self->_bms_control_msg = 0x00U;
		break;
		
	case 0x00U:
		/* Don't send messages. */
		self->_state = KANGOO_FAKE_BMS_STATE_WAIT_CONTROL_MSG;
		break;
	
	default:
		break;
	}
}

void _kangoo_parse_input_frames(struct kangoo_fake_bms *self)
{
	struct kangoo_can_frame *frame = &self->_input;

	switch (frame->id) {
	case 0x423U:
		self->_bms_control_msg = frame->data[0];

		if (frame->data[4] == 0x5DU) {
			self->_lbc_key_answer = 0x55U;
		} else if (frame->data[4] == 0xB2U) {
			self->_lbc_key_answer = 0xAAU;
		} else {}

		/*if (frame->data[6] == 0x5DU) {
			self->_lbc2_key_answer = 0x55U;
		} else if (frame->data[6] == 0xB2U) {
			self->_lbc2_key_answer = 0xAAU;
		} else {}*/

		break;

	case 0x424U:
		self->_max_batt_temp_C =
				  kangoo_x424_get_max_batt_temp_C(frame->data);

		/* Now in sync with lbc1 key  */
		self->_lbc2_key_answer = frame->data[6];
		break;

	case 0x425U:
		self->_highest_cell_V =
				  kangoo_x425_get_highest_cell_V(frame->data);

		self->_lowest_cell_V =
				  kangoo_x425_get_lowest_cell_V(frame->data);

		break;

	case 0x426U:
		self->_OT = frame->data[1];
		break;
	
	default:
		break;
	}
}

void _kangoo_fake_bms_send_messages(struct kangoo_fake_bms *self,
				   uint32_t delta_time_ms)
{
	self->_msg_1_timer_ms += delta_time_ms;

	if (self->_msg_1_timer_ms >= 100U) {
		self->_msg_1_timer_ms = 0U;
		
		self->_frames[0].id      = 0x445U;
		self->_frames[0].len     = 7U;
		self->_frames[0].data[0] = 0x40U; /* ready to start? 0x80-0x40*/
		self->_frames[0].data[1] = 0x55U; /* ??? */
		self->_frames[0].data[2] = self->_lbc2_key_answer;/* heartbeat */
		self->_frames[0].data[3] = 0x00U; /* max batt temp -40c */
		self->_frames[0].data[4] = 0x00U; /* hi cell V x0.01 + 1v */
		self->_frames[0].data[5] = 0x00U; /* lo cell V x0.01 + 1v */
		self->_frames[0].data[6] = 0x00U; /* ??? */

		kangoo_x445_set_max_batt_temp_C(self->_frames[0].data,
						self->_max_batt_temp_C);
		kangoo_x445_set_highest_cell_V(self->_frames[0].data,
						self->_highest_cell_V);
		kangoo_x445_set_lowest_cell_V(self->_frames[0].data,
						self->_lowest_cell_V);

		self->_frames_available = 1U;
	}
}

/******************************************************************************
 * PUBLIC
 *****************************************************************************/
void kangoo_fake_bms_init(struct kangoo_fake_bms *self)
{
	self->_state = KANGOO_FAKE_BMS_STATE_NONE;
	self->_bms_control_msg = 0x00U;

	/* x424*/
	self->_max_batt_temp_C = 0;

	/* x425*/
	self->_highest_cell_V  = 0.0f;
	self->_lowest_cell_V   = 0.0f;

	self->_msg_0_timer_ms = 0U;
	self->_msg_1_timer_ms = 0U;
	self->_msg_2_timer_ms = 0U;

	self->_frames_available = 0U;
	self->_input.len = -1;
	self->_frames[0].len = -1;
	self->_frames[1].len = -1;
	self->_frames[2].len = -1;
}

void kangoo_fake_bms_start(struct kangoo_fake_bms *self)
{
	self->_state = KANGOO_FAKE_BMS_STATE_WAIT_CONTROL_MSG;
}

void kangoo_fake_bms_stop(struct kangoo_fake_bms *self)
{
	self->_state = KANGOO_FAKE_BMS_STATE_NONE;
}

struct kangoo_can_frame *kangoo_fake_bms_can_frame_read(
						struct kangoo_fake_bms *self)
{
	struct kangoo_can_frame *result = NULL;

	if (self->_frames_available > 0U) {
		result = &self->_frames[self->_frames_available - 1U];
		
		self->_frames_available--;
	}

	return result;
}

void kangoo_fake_bms_can_frame_write(struct kangoo_fake_bms *self,
				     struct kangoo_can_frame frame)
{
	self->_input = frame;
}

bool kangoo_fake_bms_ready_for_write(struct kangoo_fake_bms *self)
{
	return self->_state > (uint8_t)KANGOO_FAKE_BMS_STATE_NONE;
}

void kangoo_fake_bms_update(struct kangoo_fake_bms *self,
			    uint32_t delta_time_ms)
{
	switch (self->_state) {
	case KANGOO_FAKE_BMS_STATE_NONE:
		break;

	case KANGOO_FAKE_BMS_STATE_WAIT_CONTROL_MSG:
		if (self->_input.len > -1) {
			_kangoo_parse_input_frames(self);
			_kangoo_fake_bms_parse_control_msg(self);

			/* Invalidate input frame */
			self->_input.len = -1;
		}
		break;

	case KANGOO_FAKE_BMS_STATE_SEND_MESSAGES:
		if (self->_input.len > -1) {
			_kangoo_parse_input_frames(self);
			_kangoo_fake_bms_parse_control_msg(self);

			/* Invalidate input frame */
			self->_input.len = -1;
		}

		_kangoo_fake_bms_send_messages(self, delta_time_ms);
		break;
	
	default:
		break;
	}
}

#endif /* KANGOO_FAKE_BMS_GUARD */
