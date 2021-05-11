#include "ch.h"
#include "hal.h"
#include <arm_math.h>
#include <arm_const_structs.h>

#include <main.h>
#include <process_audio.h>
#include <process_speed.h>

#include <motors.h>
#include <audio/microphone.h>

static float micLeft_cmplx_input[DOUBLE * FFT_SIZE];
static float micRight_cmplx_input[DOUBLE * FFT_SIZE];
static float micLeft_output[FFT_SIZE];
static float micRight_output[FFT_SIZE];

static bool ready_to_go = true;


void compute_turn(float* data_left, float* data_right)
{
	float max_norm_left = MIN_VALUE_THRESHOLD;
	int8_t max_norm_index_left = MIN_INDEX;
	float max_norm_right = MIN_VALUE_THRESHOLD;
	int8_t max_norm_index_right = MIN_INDEX;
	static bool already_turned = false;

	//search for the highest peak left microphone
	for(uint8_t i = MIN_FREQ ; i <= MAX_FREQ ; i++)
	{
		if(data_left[i] > max_norm_left)
		{
			max_norm_left = data_left[i];
			max_norm_index_left = i;
		}
	}
	//search for the highest peak right microphone
	for(uint8_t i = MIN_FREQ ; i <= MAX_FREQ ; i++)
	{
		if(data_right[i] > max_norm_right)
		{
			max_norm_right = data_right[i];
			max_norm_index_right = i;
		}
	}

	//go right
	if((max_norm_index_left >= RIGHT_FREQ-MARGE && max_norm_index_left <= RIGHT_FREQ+MARGE) &&
	   (max_norm_index_right >= RIGHT_FREQ-MARGE && max_norm_index_right <= RIGHT_FREQ+MARGE))
	{
		left_motor_set_speed(TURN_SPEED);
		right_motor_set_speed(NO_SPEED);
		already_turned = true;
	}
	//go left
	else if((max_norm_index_left >= LEFT_FREQ-MARGE && max_norm_index_left <= LEFT_FREQ+MARGE) &&
			(max_norm_index_right >= LEFT_FREQ-MARGE && max_norm_index_right <= LEFT_FREQ+MARGE))
	{
		left_motor_set_speed(NO_SPEED);
		right_motor_set_speed(TURN_SPEED);
		already_turned = true;
	}
	else
	{
		left_motor_set_speed(NO_SPEED);
		right_motor_set_speed(NO_SPEED);
		if(already_turned)
		{
			ready_to_go = true;
			clear_ready_to_turn();
			already_turned = false;
		}
	}
}

bool get_ready_to_go(void)
{
	return ready_to_go;
}

void clear_ready_to_go(void)
{
	ready_to_go = 0;
}

void processAudioData(int16_t *data, uint16_t num_samples)
{
	if(get_ready_to_turn())
	{
		static uint16_t nb_samples = 0;

		//fills the buffers until they are full
		for(uint16_t i = 0 ; i < num_samples ; i+=4)
		{
			micRight_cmplx_input[nb_samples] = (float)data[i + MIC_RIGHT];
			micLeft_cmplx_input[nb_samples] = (float)data[i + MIC_LEFT];
			nb_samples++;

			micRight_cmplx_input[nb_samples] = 0;
			micLeft_cmplx_input[nb_samples] = 0;
			nb_samples++;

			if(nb_samples >= (DOUBLE*FFT_SIZE))
			{
				break;
			}
		}

		//the buffers are full, and data is ready to be processed
		if(nb_samples >= (DOUBLE*FFT_SIZE))
		{
			//FFT proccessing
			doFFT_optimized(FFT_SIZE, micRight_cmplx_input);
			doFFT_optimized(FFT_SIZE, micLeft_cmplx_input);
			//Magnitude processing
			arm_cmplx_mag_f32(micRight_cmplx_input, micRight_output, FFT_SIZE);
			arm_cmplx_mag_f32(micLeft_cmplx_input, micLeft_output, FFT_SIZE);

			nb_samples = 0;
			compute_turn(micLeft_output, micRight_output);
		}
	}
}

void doFFT_optimized(uint16_t size, float* complex_buffer)
{
	if(size == FFT_SIZE)
	{
		arm_cfft_f32(&arm_cfft_sR_f32_len1024, complex_buffer, 0, 1);
	}
}
