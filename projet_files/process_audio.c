#include "ch.h"
#include "hal.h"
#include <main.h>
#include <usbcfg.h>
#include <chprintf.h>

#include <motors.h>
#include <audio/microphone.h>
#include <process_image.h>
#include <arm_math.h>
#include <arm_const_structs.h>
#include <process_audio.h>
#include <process_speed.h>



#define MIN_VALUE_THRESHOLD 10000
#define MIN_FREQ 10 	//we don’t analyze before this index to not use resources for nothing
#define LEFT_FREQ 15	//230 Hz
#define RIGHT_FREQ 20	//300 Hz
#define MAX_FREQ 30 	//we don’t analyze after this index to not use resources for nothing
#define MARGE 1

//semaphore
static BSEMAPHORE_DECL(sendToComputer_sem, TRUE);

//2 times FFT_SIZE because these arrays contain complex numbers (real + imaginary)
static float micLeft_cmplx_input[2 * FFT_SIZE];
static float micRight_cmplx_input[2 * FFT_SIZE];

//Arrays containing the computed magnitude of the complex numbers
static float micLeft_output[FFT_SIZE];
static float micRight_output[FFT_SIZE];


static bool ready_to_go = 1;


void sound_remote(float* data_left, float* data_right)
{
	float max_norm_left = MIN_VALUE_THRESHOLD;
	int16_t max_norm_index_left = -1;
	float max_norm_right = MIN_VALUE_THRESHOLD;
	int16_t max_norm_index_right = -1;
	static bool already_turned = 0;
	//search for the highest peak left microphone
	for(uint16_t i = MIN_FREQ ; i <= MAX_FREQ ; i++)
	{
		if(data_left[i] > max_norm_left)
		{
			max_norm_left = data_left[i];
			max_norm_index_left = i;
		}
	}
	//search for the highest peak right microphone
	for(uint16_t i = MIN_FREQ ; i <= MAX_FREQ ; i++)
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
		left_motor_set_speed(300);
		right_motor_set_speed(0);
		already_turned = 1;
	}
	//go left
	else if((max_norm_index_left >= LEFT_FREQ-MARGE && max_norm_index_left <= LEFT_FREQ+MARGE) &&
			(max_norm_index_right >= LEFT_FREQ-MARGE && max_norm_index_right <= LEFT_FREQ+MARGE))
	{
		left_motor_set_speed(0);
		right_motor_set_speed(300);
		already_turned = 1;

	}
	else
	{
		left_motor_set_speed(0);
		right_motor_set_speed(0);
		if(already_turned)
		{
			ready_to_go = 1;
			clear_ready_to_turn();
			already_turned = 0;
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



void processAudioData(int16_t *data, uint16_t num_samples){

	if(get_ready_to_turn())
	{
		static uint16_t nb_samples = 0;
		static uint8_t mustSend = 0;
		//loop to fill the buffers
		for(uint16_t i = 0 ; i < num_samples ; i+=4){
		//construct an array of complex numbers. Put 0 to the imaginary part
			micRight_cmplx_input[nb_samples] = (float)data[i + MIC_RIGHT];
			micLeft_cmplx_input[nb_samples] = (float)data[i + MIC_LEFT];
			nb_samples++;

			micRight_cmplx_input[nb_samples] = 0;
			micLeft_cmplx_input[nb_samples] = 0;
			nb_samples++;


		//stop when buffer is full
			if(nb_samples >= (2 * FFT_SIZE)){
				break;
			}
		}

		if(nb_samples >= (2 * FFT_SIZE)){
		/* FFT proccessing
		*
		* This FFT function stores the results in the input buffer given.
		* This is an "In Place" function.
		*/
			doFFT_optimized(FFT_SIZE, micRight_cmplx_input);
			doFFT_optimized(FFT_SIZE, micLeft_cmplx_input);
		/* Magnitude processing
		*
		* Computes the magnitude of the complex numbers and
		* stores them in a buffer of FFT_SIZE because it only contains
		* real numbers.
		*
		*/
			arm_cmplx_mag_f32(micRight_cmplx_input, micRight_output, FFT_SIZE);
			arm_cmplx_mag_f32(micLeft_cmplx_input, micLeft_output, FFT_SIZE);

		//sends only one FFT result over 10 for 1 mic to not flood the computer
		//sends to UART3
		if(mustSend > 8){
		//signals to send the result to the computer
			chBSemSignal(&sendToComputer_sem);
			mustSend = 0;
		}
		nb_samples = 0;
		mustSend++;
		sound_remote(micLeft_output, micRight_output);
		}
	}
}

void doFFT_optimized(uint16_t size, float* complex_buffer){

	if(size == 1024)
		arm_cfft_f32(&arm_cfft_sR_f32_len1024, complex_buffer, 0, 1);

}

