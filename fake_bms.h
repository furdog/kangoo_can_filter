struct kangoo_fake_bms_can_frame
{
	uint32_t id;
	int8_t   len;
	uint8_t  data[8];
};

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

	uint32_t _msg_0_timer_ms;
	uint32_t _msg_1_timer_ms;
	uint32_t _msg_2_timer_ms;

	uint8_t _frames_available;
	struct kangoo_fake_bms_can_frame _input;
	struct kangoo_fake_bms_can_frame _frames[3];
};

/******************************************************************************
 * PRIVATE
 *****************************************************************************/
void _kangoo_fake_bms_start_communication(struct kangoo_fake_bms *self)
{
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
		/* Stop fake bms communication */
		_kangoo_fake_bms_stop_communication(self);
		break;
	
	default:
		break;
	}
}

void _kangoo_parse_input_frames(struct kangoo_fake_bms *self)
{
	struct kangoo_fake_bms_can_frame *frame = &self->_input;

	switch (frame->id) {
	case 0x423U:
		self->_bms_control_msg = frame->data[0];

		if (frame->data[4] == 0x5DU) {
			self->_lbc_key_answer = 0x55U;
		} else if (frame->data[6] == 0xB2U) {
			self->_lbc_key_answer = 0xAAU;
		} else {}

		if (frame->data[6] == 0x5DU) {
			self->_lbc2_key_answer = 0x55U;
		} else if (frame->data[6] == 0xB2U) {
			self->_lbc2_key_answer = 0xAAU;
		} else {}

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
	self->_msg_0_timer_ms += delta_time_ms;
	self->_msg_1_timer_ms += delta_time_ms;
	self->_msg_2_timer_ms += delta_time_ms;

	/* Invalidate frames before send. */
	self->_frames_available = 0U;
	self->_frames[0].len = -1;
	self->_frames[1].len = -1;
	self->_frames[2].len = -1;

	if (self->_msg_0_timer_ms >= 10U)
	{
		self->_msg_0_timer_ms = 0U;

		self->_frames[0].id      = 0x155U;
		self->_frames[0].len     = 8U;
		/* can_frame_1.data[0] = 0x85; */
		self->_frames[0].data[0] = 0x00U; /* charging power */
		self->_frames[0].data[1] = 0x87U; /* ???            */
		self->_frames[0].data[2] = 0xD0U; /* current + 400  */
		self->_frames[0].data[3] = 0x54U; /*                */
		self->_frames[0].data[4] = 0x50U; /* soc + 400      */
		self->_frames[0].data[5] = 0x00U; /*                */
		self->_frames[0].data[6] = 0x02U; /* voltage * 2    */
		self->_frames[0].data[7] = 0xBCU; /*                */

		self->_frames_available = 1U;
	} else if (self->_msg_1_timer_ms >= 100U) {
		self->_msg_1_timer_ms = 0U;

		self->_frames[0].id      = 0x424U;
		self->_frames[0].len     = 8U;
		self->_frames[0].data[0] = 0x12U; /* ? */
		self->_frames[0].data[1] = 0x40U; 
		self->_frames[0].data[2] = 0x23U; /* max input power * 2  */
		self->_frames[0].data[3] = 0xAFU; /* max output power * 2 */
		self->_frames[0].data[4] = 0x28U; /* batt temp - 40c      */
		self->_frames[0].data[5] = 0x50U; /* batt health 0-100    */
		self->_frames[0].data[6] = self->_lbc_key_answer; /* heartbeat */
		self->_frames[0].data[7] = 0x43U; /* max batt temp - 40	*/
		
		self->_frames[1].id      = 0x425U;
		self->_frames[1].len     = 8U;
		self->_frames[1].data[0] = 0x2AU; /* (OT ? 0x2A : 0x24); ? */
		self->_frames[1].data[1] = 0x44U; /* available kwh * 10 */
		self->_frames[1].data[2] = 0x44U; /* ? */
		self->_frames[1].data[3] = 0x9CU; /* hv iso resist ohm / 100 */
		self->_frames[1].data[4] = 0x42U; /*                         */
		self->_frames[1].data[5] = 0x4AU; /* highest cell V x100 - 1 */
		self->_frames[1].data[6] = 0x01U; /* Lowest cell V x100 - 1  */
		self->_frames[1].data[7] = 0x24U; /*                         */	
		
		self->_frames[2].id      = 0x445U;
		self->_frames[2].len     = 7U;
		self->_frames[2].data[0] = 0x40U; /* ready to start? 0x80-0x40*/
		self->_frames[2].data[1] = 0x55U; /* ??? */
		self->_frames[2].data[2] = self->_lbc2_key_answer;/* heartbeat */
		self->_frames[2].data[3] = 0x85U; /* max batt temp -40c */
		self->_frames[2].data[4] = 0x25U; /* hi cell V x0.01 + 1v */
		self->_frames[2].data[5] = 0x91U; /* lo cell V x0.01 + 1v */
		self->_frames[2].data[6] = 0x80U; /* ??? */

		self->_frames_available = 3U;

	} else if (self->_msg_2_timer_ms >= 3000U) {
		self->_msg_2_timer_ms = 0U;

		self->_frames[0].id      = 0x659U;
		self->_frames[0].len     = 4U;
		self->_frames[0].data[0] = 0x0U;
		self->_frames[0].data[1] = 0x0U;
		self->_frames[0].data[2] = 0x0U;
		self->_frames[0].data[3] = 0x0U;
		
		self->_frames_available = 1U;
	} else {}
}

/******************************************************************************
 * PUBLIC
 *****************************************************************************/
void kangoo_fake_bms_init(struct kangoo_fake_bms *self)
{
	self->_state = KANGOO_FAKE_BMS_STATE_NONE;
	self->_bms_control_msg = 0x00U;

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

struct kangoo_fake_bms_can_frame *kangoo_fake_bms_can_frame_read(
						struct kangoo_fake_bms *self)
{
	struct kangoo_fake_bms_can_frame *result = NULL;

	if (self->_frames_available > 0U) {
		result = &self->_frames[self->_frames_available - 1U];
		
		self->_frames_available--;
	}

	return result;
}

void kangoo_fake_bms_can_frame_write(struct kangoo_fake_bms *self,
				     struct kangoo_fake_bms_can_frame frame)
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
