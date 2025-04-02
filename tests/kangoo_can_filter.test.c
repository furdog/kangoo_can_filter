#include <stdio.h>
#include "kangoo_can_filter.h"

uint8_t buf[512];
struct mon_var_storage st;

/* User specified mon_var metadata */
struct mon_var_meta
{
	const char *source;

	uint8_t type;
	uint8_t flags;
};

struct mon_var_data_int
{
	struct mon_var_header header;
	struct mon_var_meta   meta;
	int                   value;
};

struct mon_var_data_string
{
	struct mon_var_header header;
	struct mon_var_meta   meta;
	const char           *value;
};

struct mon_var_data_generic
{
	struct mon_var_header header;
	struct mon_var_meta   meta;
	void                 *value;
};

int main()
{
	int i;

	struct mon_var_data_int    *ivar;
	struct mon_var_data_string *svar;

	mon_var_storage_init(&st, buf, 512);

	/* IVAR */
	ivar = mon_var_storage_alloc(&st, sizeof(struct mon_var_data_int));

	ivar->header.name    = "some_var";
	ivar->meta.type      = 1;
	ivar->value = 42;
	mon_var_change(ivar);

	assert(mon_var_storage_update(&st) == NULL);
	assert(mon_var_storage_update(&st) != NULL);

	printf("data: %i\n", ivar->value);

	/* SVAR */
	svar = mon_var_storage_alloc(&st, sizeof(struct mon_var_data_int));

	svar->header.name    = "some_var";
	svar->meta.type      = 2;
	svar->value          = "Hello world!";
	mon_var_change(svar);

	printf("data: %s\n", svar->value);

	assert(mon_var_storage_update(&st) != NULL);

	/* change again*/
	ivar->value = 33;
	svar->value = "test";

	mon_var_change(ivar);
	mon_var_change(svar);

	for (i = 0; i < 10; i++) {
		struct mon_var_data_generic *data;

		data = mon_var_storage_update(&st);

		if (data && data->meta.type == 1) {
			printf("data: %i\n", ivar->value);
		}

		if (data && data->meta.type == 2) {
			printf("data: %s\n", svar->value);
		}
	}

	return 0;
}
