#include "ch.h"
#include "hal.h"
#include <math.h>

#include <main.h>
#include <process_speed.h>
#include <process_image.h>
#include <process_audio.h>

#include <sensors/proximity.h>
#include <leds.h>
#include <audio/play_melody.h>
#include <selector.h>
#include <motors.h>

static bool ready_to_turn = NOT_READY;

//gives the speed corresponding to the robot's actual state
uint16_t accurate_speed(uint16_t front_sensor_r, uint16_t front_sensor_l,
						uint16_t back_sensor_r,  uint16_t back_sensor_l, bool state)
{
	//the robot is on mode off
	if(get_selector() == OFF)
	{
		return NO_SPEED;
	}
	//the robot needs to turn
	else if((state == STOP) 						//because he found a line
			|| ((front_sensor_r > LETHAL_SPACE) 	//because we put him a stop
				&& (front_sensor_l > LETHAL_SPACE)))
	{
		ready_to_turn = READY;
		clear_ready_to_go();
		return NO_SPEED;
	}
	//the robot accelerates in function of the values of the back sensors
	else if((back_sensor_r > VITAL_SPACE) && (back_sensor_l > VITAL_SPACE))
	{
		uint16_t mean_sensors = (back_sensor_r + back_sensor_l) / 2.;
		uint16_t acceleration_speed = KP * mean_sensors + CORRECTION;
		if(acceleration_speed > MOTOR_SPEED_LIMIT)
		{
			return MOTOR_SPEED_LIMIT;
		}
		return acceleration_speed;
	}
	//the robot is in its default case
	else
	{
		return WALK_SPEED;
	}
}

static THD_WORKING_AREA(waProcessSpeed, WORKING_AREA_SPEED);
static THD_FUNCTION(ProcessSpeed, arg)
{
	chRegSetThreadName(__FUNCTION__);
	(void)arg;

	systime_t time;
	uint16_t speed = 0;

	while(1)
	{
		stopCurrentMelody();
		//the robot is in its final state
		while((get_selector() == FINISHED))
		{
			melody_t* song = NULL;
			right_motor_set_speed(NO_SPEED);
			left_motor_set_speed(NO_SPEED);
			playMelody(PIRATES_OF_THE_CARIBBEAN, ML_SIMPLE_PLAY, song);
		}
		//the robot is not turning, a speed needs to be determined
		if(get_ready_to_go())
		{
			time = chVTGetSystemTime();

			speed = accurate_speed(get_calibrated_prox(SENSOR1), get_calibrated_prox(SENSOR8),
								   get_calibrated_prox(SENSOR4), get_calibrated_prox(SENSOR5), get_state());
			animation(speed);

			right_motor_set_speed(speed);
			left_motor_set_speed(speed);

			chThdSleepUntilWindowed(time, time + MS2ST(10)); 	//calls the thread at 100Hz
		}
	}
}

//makes the body led blink at a given frequency with respect to the proximity to the back sensors
void animation(uint16_t speed)
{
	int counter = 0;
	static bool last_led = 0;

	if(speed <= WALK_SPEED)				//no blinking
	{
		set_body_led(TURN_OFF);
		last_led = TURN_OFF;
	}
	else if(speed <= LOW_SPEED)			//blinks slowly
	{

		while(counter < LOW_CNT)
		{
			counter++;
		}
		counter = 0;
		if(last_led)
		{
			set_body_led(TURN_OFF);
			last_led = TURN_OFF;
		}
		else
		{
			set_body_led(TURN_ON);
			last_led = TURN_ON;
		}
	}
	else if(speed <= MIDDLE_SPEED)		//blinks moderately
	{

		while(counter < MIDDLE_CNT)
		{
			counter++;
		}
		counter = 0;
		if(last_led)
		{
			set_body_led(TURN_OFF);
			last_led = TURN_OFF;
		}
		else
		{
			set_body_led(TURN_ON);
			last_led = TURN_ON;
		}
	}
	else if(speed <= HIGH_SPEED)		//blinks fast
	{

		while(counter < HIGH_CNT)
		{
			counter++;
		}
		counter = 0;
		if(last_led)
		{
			set_body_led(TURN_OFF);
			last_led = TURN_OFF;
		}
		else
		{
			set_body_led(TURN_ON);
			last_led = TURN_ON;
		}
	}
	else								//blinks super fast
	{
		while(counter < MAX_CNT)
		{
			counter++;
		}
		counter = 0;
		if(last_led)
		{
			set_body_led(TURN_OFF);
			last_led = TURN_OFF;
		}
		else
		{
			set_body_led(TURN_ON);
			last_led = TURN_ON;
		}
	}
}

void process_speed_start(void)
{
	chThdCreateStatic(waProcessSpeed, sizeof(waProcessSpeed), NORMALPRIO, ProcessSpeed, NULL);
}

bool get_ready_to_turn(void)
{
	return ready_to_turn;
}

void clear_ready_to_turn(void)
{
	ready_to_turn = NOT_READY;
}
