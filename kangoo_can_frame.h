#ifndef KANGOO_CAN_FRAME_GUARD
#define KANGOO_CAN_FRAME_GUARD

/******************************************************************************
 * GENERIC CAN BUS ABSTRACTION FOR KANGOO
 *****************************************************************************/
/* TODO MAKE SAFE READ/WRITE */
struct kangoo_can_frame {
	uint32_t id;
	int8_t   len;
	uint8_t  data[8];
};

#endif /* KANGOO_CAN_FRAME_GUARD */
