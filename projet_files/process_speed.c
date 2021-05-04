#include "ch.h"
#include "hal.h"
#include <math.h>
#include <usbcfg.h>
#include <chprintf.h>
#include <main.h>
#include <motors.h>
#include <process_image.h>
#include <sensors/proximity.h>
#include <leds.h>
#include <audio/play_melody.h>
#include <process_speed.h>
#include <process_audio.h>
#include <selector.h>


static bool ready_to_turn = 0;

//simple PI regulator implementation
uint16_t accurate_speed(bool state, uint16_t sensor_1, uint16_t sensor_8,uint16_t sensor_4,  uint16_t sensor_5)
{

	if((state == STOP) || !(get_ready_to_go()) || (sensor_1 > 200 && sensor_8 > 200))
	{
		ready_to_turn = 1;
		clear_ready_to_go();
		return 0;
	}
	else if((sensor_5 > 5) && (sensor_4 > 5))
	{
		uint16_t acceleration_speed = KP*(sensor_4+sensor_5)/2.+CORRECTION;

		if(acceleration_speed > MOTOR_SPEED_LIMIT)
		{
			return MOTOR_SPEED_LIMIT;
		}
		return acceleration_speed;
	}
	else if(get_selector()==OFF)
	{
		clear_finished();
		return 0;
	}
	else
	{
		return DEFAULT_SPEED;
	}
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
	uint16_t sensor_1 = 0;
	uint16_t sensor_8 = 0;


	while(1)
	{
		if(get_finished())
		{
			melody_t* song = NULL;

			right_motor_set_speed(0);
			left_motor_set_speed(0);
			playMelody(PIRATES_OF_THE_CARIBBEAN, ML_SIMPLE_PLAY, song);
		}
		else if(get_ready_to_go())
		{
			time = chVTGetSystemTime();
			sensor_4 = get_calibrated_prox(3);
			sensor_5 = get_calibrated_prox(4);
			sensor_1 = get_calibrated_prox(0);
			sensor_8 = get_calibrated_prox(7);

			speed = accurate_speed(get_state(), sensor_1, sensor_8, sensor_4, sensor_5);
			animation(speed);


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

