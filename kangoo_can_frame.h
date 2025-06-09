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

void kangoo_can_frame_print(struct kangoo_can_frame *frame)
{
	int i;

	printf("ID: %08X, LEN: %i, [", frame->id, frame->len);
	for (i = 0; i < frame->len; i++) {
		printf("0x%02X", frame->data[i]);

		if (i < (frame->len - 1)) {
			printf(" ");
		}
	}
	
	printf("]\n");

	fflush(0);
}

#endif /* KANGOO_CAN_FRAME_GUARD */
