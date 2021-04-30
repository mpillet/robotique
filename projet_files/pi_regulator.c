#include "ch.h"
#include "hal.h"
#include <math.h>
#include <usbcfg.h>
#include <chprintf.h>


#include <main.h>
#include <motors.h>
#include <pi_regulator.h>
#include <process_image.h>
#include <audio_processing.h>
#include <sensors/proximity.h>


static bool ready_to_turn = 0;

//simple PI regulator implementation
int16_t pi_regulator(bool state){

	//disables the PI regulator if the error is to small
	//this avoids to always move as we cannot exactly be where we want and 
	//the camera is a bit noisy
	if((state == STOP) || !(get_ready_to_go()))
	{

		ready_to_turn = 1;
		clear_ready_to_go();
		return 0;
	}

    return 1;
}

static THD_WORKING_AREA(waPiRegulator, 256);
static THD_FUNCTION(PiRegulator, arg)
{
	chRegSetThreadName(__FUNCTION__);
	(void)arg;

	systime_t time;

	uint16_t speed = 0;
	uint16_t sensor_4 = 0;
	uint16_t sensor_5 = 0;

	while(1)
	{
		if(get_ready_to_go())
		{
			time = chVTGetSystemTime();
			sensor_4 = get_calibrated_prox(3);
			sensor_5 = get_calibrated_prox(4);
			//chprintf((BaseSequentialStream *)&SDU1, "IR 4 = %d\n", get_calibrated_prox(3));
			//chprintf((BaseSequentialStream *)&SDU1, "IR 5 = %d\n", get_calibrated_prox(4));

			if((sensor_5 > 20) && (sensor_4 > 20))
			{
				speed = (5.)*(sensor_4+sensor_5)/2.+100.;

				if(speed > 1100)
					{
						speed = 1100;
					}
			}

			else
			{
				speed = 200;
			}

			if(!pi_regulator(get_state()))
			{
				speed = 0;
			}



			//applies the speed from the PI regulator and the correction for the rotation
			right_motor_set_speed(speed);
			left_motor_set_speed(speed);

			//100Hz
			chThdSleepUntilWindowed(time, time + MS2ST(10));
		}
	}
}

void pi_regulator_start(void){
	chThdCreateStatic(waPiRegulator, sizeof(waPiRegulator), NORMALPRIO, PiRegulator, NULL);
}

bool get_ready_to_turn(void){
	return ready_to_turn;
}

void clear_ready_to_turn(void)
{
	ready_to_turn = 0;
}

