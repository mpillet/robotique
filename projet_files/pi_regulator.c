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
#include <leds.h>


static bool ready_to_turn = 0;

//simple PI regulator implementation
bool pi_regulator(bool state){

	//disables the PI regulator if the error is to small
	//this avoids to always move as we cannot exactly be where we want and 
	//the camera is a bit noisy
	if((state == STOP) || !(get_ready_to_go()))
	{
		ready_to_turn = 1;
		clear_ready_to_go();
		return STOP;
	}

    return CONTINUE;
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
	uint16_t mean = 0;


	while(1)
	{
		if(get_ready_to_go())
		{
			time = chVTGetSystemTime();
			sensor_4 = get_calibrated_prox(3);
			sensor_5 = get_calibrated_prox(4);
			mean = (sensor_4+sensor_5)/2.;


			if((sensor_5 > 5) && (sensor_4 > 5))
			{
				speed = KP*mean+CORRECTION;
				if(speed > MOTOR_SPEED_LIMIT)
					{
						speed = MOTOR_SPEED_LIMIT;
					}
			}
			else
			{
				speed = DEFAULT_SPEED;
			}

			if(pi_regulator(get_state()) == STOP)
			{
				speed = 0;
			}

			animation(speed);
//			chprintf((BaseSequentialStream *)&SDU1, "vitesse = %d\n", speed);


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

void animation(uint16_t speed)
{
	int counter = 0;
	static bool last_led = 0;

	if(speed <= DEFAULT_SPEED)
	{
		set_body_led(0);
		last_led = 0;
	}
	else if(speed <= LOW_SPEED)
	{

		while(counter < LOW_CNT)
		{
			counter++;
		}
		counter = 0;
		if(last_led)
		{
			set_body_led(0);
			last_led = 0;
		}
		else
		{
			set_body_led(1);
			last_led = 1;
		}
	}
	else if(speed <= MIDDLE_SPEED)
	{

		while(counter < MIDDLE_CNT)
		{
			counter++;
		}
		counter = 0;
		if(last_led)
		{
			set_body_led(0);
			last_led = 0;
		}
		else
		{
			set_body_led(1);
			last_led = 1;
		}
	}
	else if(speed <= HIGH_SPEED)
	{

		while(counter < HIGH_CNT)
		{
			counter++;
		}
		counter = 0;
		if(last_led)
		{
			set_body_led(0);
			last_led = 0;
		}
		else
		{
			set_body_led(1);
			last_led = 1;
		}
	}
	else
	{

		while(counter < MAX_CNT)
		{
			counter++;
		}
		counter = 0;
		if(last_led)
		{
			set_body_led(0);
			last_led = 0;
		}
		else
		{
			set_body_led(1);
			last_led = 1;
		}
	}
}

