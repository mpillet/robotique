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

static bool ready_to_turn = 0;

//simple PI regulator implementation
int16_t pi_regulator(float distance, float goal){

	float error = 0;
	float speed = 0;

	static float sum_error = 0;

	error = distance - goal;

	//disables the PI regulator if the error is to small
	//this avoids to always move as we cannot exactly be where we want and 
	//the camera is a bit noisy
	if(fabs(error) < ERROR_THRESHOLD || !(get_ready_to_go()))
	{
		ready_to_turn = 1;
		clear_ready_to_go();
		return 0;
	}

    return 1100;
}

static THD_WORKING_AREA(waPiRegulator, 256);
static THD_FUNCTION(PiRegulator, arg)
{
	chRegSetThreadName(__FUNCTION__);
	(void)arg;

	systime_t time;

	int16_t speed = 0;
	int16_t speed_correction = 0;

	while(1)
	{
		if(get_ready_to_go())
		{
			time = chVTGetSystemTime();

			//computes the speed to give to the motors
			//distance_cm is modified by the image processing thread
			speed = pi_regulator(get_distance_cm(), GOAL_DISTANCE);

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

