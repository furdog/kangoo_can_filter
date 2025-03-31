#include "kangoo_can_filter.h"
#include <stdio.h>

uint8_t buf[512];
struct mon_var_storage st;

int main()
{
	struct mon_var *v;
	mon_var_storage_init(&st, buf, 512);
	
	v = mon_var_storage_alloc(&st, sizeof(int));

	v->name  = "some_var";
	v->type  = MON_VAR_TYPE_INT;
	v->flags = 0;
	*(int *)(v->data) = 42;
	v->_changed = true;
	
	v = mon_var_storage_update(&st);
	assert(v == NULL);
	
	v = mon_var_storage_update(&st);
	assert(v != NULL);

	return 0;
}
