#include <time.h>
#include <stdio.h>

#define MSECS_IN_HOUR (1000 * 60 * 60)

/** Timers */
clock_t timestamp = 0;
clock_t elapsed = 0;

/** Vehicle params. */
float voltage;
float current;
double cell_voltage = 3.2;
double cell_charge_ratio;
double charge_hours;
double soh = 100;

/** Filter params. */
double kwh = 0;
double cell_max = 4.18;
double kwh_max = 33;

void count_kwt_test()
{
	timestamp = 0;
	elapsed = 0;
	
	for (timestamp = 0; timestamp <= MSECS_IN_HOUR * charge_hours; timestamp += 10)
	{
		elapsed = timestamp - elapsed;
		double per_hour = ((double)elapsed / MSECS_IN_HOUR);
		/* printf("%lu\n", elapsed); */

		kwh += (double)(voltage * current) / 1000.0 * per_hour;
		if (kwh < 0)
			kwh = 0;
		
		cell_voltage += (current >= 0 ? cell_charge_ratio : -cell_charge_ratio) * per_hour;
		
		if (cell_voltage > cell_max)
			soh = kwh / kwh_max * 100.0;
		
		elapsed = timestamp;
	}
	printf("kwh: %f\n", kwh);
	printf("cell_v: %f\n", cell_voltage);
	printf("soh: %f\n\n", soh);
}

int main()
{		
	cell_voltage = 3.20;
	cell_charge_ratio = 0.2;

	/** Update kwt counter every 10ms
	 *  Test positive current. */
	voltage = 350;
	current = 10;
	charge_hours = 1;
	count_kwt_test();
	
	/** Update kwt counter every 10ms
	 *  Test negative current. */
	//cell_max = 4.28;
	voltage = 350;
	current = -10;
	charge_hours = 1;
	count_kwt_test();
	
	/** Update kwt counter every 10ms
	 *  Test negative current. */
	//cell_max = 4.28;
	voltage = 350;
	current = 10;
	charge_hours = 5;
	count_kwt_test();
}
