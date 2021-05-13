#include "ch.h"
#include "hal.h"

#include <main.h>
#include <process_image.h>
#include <process_audio.h>

#include <camera/po8030.h>

static BSEMAPHORE_DECL(image_ready_sem, TRUE);

static bool state = CONTINUE;


void find_state(uint8_t *buffer)
{
	uint32_t mean = 0;
	uint16_t i = 0, begin = 0, end = 0;
	bool stop = 0, wrong_line = 0, line_not_found = 0;

	//performs an average
	for(uint16_t i = 0 ; i < IMAGE_BUFFER_SIZE ; i++)
	{
		mean += buffer[i];
	}
	mean /= IMAGE_BUFFER_SIZE;

	//says what sees the camera
	do
	{
		wrong_line = false;
		//search for a begin
		while(!stop && i < (IMAGE_BUFFER_SIZE - WIDTH_SLOPE))
		{
			if(buffer[i] > mean && buffer[i+WIDTH_SLOPE] < mean)
			{
				begin = i;
				stop = true;
			}
			i++;
		}
		//if a begin was found, search for an end
		if(begin && i < (IMAGE_BUFFER_SIZE - WIDTH_SLOPE))
		{
			stop = false;

			while(!stop && i < IMAGE_BUFFER_SIZE)
			{
				if(buffer[i] > mean && buffer[i-WIDTH_SLOPE] < mean)
				{
					end = i;
					stop = true;
				}
				i++;
			}
			//if no end was found
			if (i > IMAGE_BUFFER_SIZE || !end)
			{
				line_not_found = true;
			}
		}
		//if no begin was found
		else
		{
			line_not_found = true;
		}

		//if a line too small has been detected
		if(!line_not_found && ((end-begin) < MIN_LINE_WIDTH || (end-begin) > MAX_PIX))
		{
			i = end;
			begin = 0;
			end = 0;
			stop = false;
			wrong_line = true;
		}
	} while(wrong_line);

	//gives the state the robot is in
	if(line_not_found || wrong_line)
	{
		begin = 0;
		end = 0;
		state = CONTINUE;
	}
	else
	{
		state = STOP;
	}
}


//Le reste du code est pris du TP4 et modifié pour notre application

static THD_WORKING_AREA(waCaptureImage, WORKING_AREA_CAPT_IMAGE);
static THD_FUNCTION(CaptureImage, arg)
{
    chRegSetThreadName(__FUNCTION__);
    (void)arg;

	po8030_advanced_config(FORMAT_RGB565, FIRST_PIXEL, BOTTOM_LINE, IMAGE_BUFFER_SIZE,
						   TWO_LINES, SUBSAMPLING_X1, SUBSAMPLING_X1);
	dcmi_enable_double_buffering();
	dcmi_set_capture_mode(CAPTURE_ONE_SHOT);
	dcmi_prepare();

	while(1)
    {
		dcmi_capture_start();
		wait_image_ready();
		chBSemSignal(&image_ready_sem);
    }
}

static THD_WORKING_AREA(waProcessImage, WORKING_AREA_PROC_IMAGE);
static THD_FUNCTION(ProcessImage, arg)
{
    chRegSetThreadName(__FUNCTION__);
    (void)arg;

	uint8_t *img_buff_ptr;
	uint8_t image[IMAGE_BUFFER_SIZE] = {0};

    while(1)
    {
        chBSemWait(&image_ready_sem);
		img_buff_ptr = dcmi_get_last_image_ptr();

		//Extracts only the red pixels in RGB format (5 first bits of the first byte for each pixel)
		for(uint16_t i = 0 ; i < (DOUBLE*IMAGE_BUFFER_SIZE) ; i+=2)
		{
			image[i/2] = (uint8_t)img_buff_ptr[i]&RED_PIX_MASK;
		}

		find_state(image);
	}
}

bool get_state(void)
{
	return state;
}

void process_image_start(void)
{
	chThdCreateStatic(waProcessImage, sizeof(waProcessImage), NORMALPRIO+2, ProcessImage, NULL);
	chThdCreateStatic(waCaptureImage, sizeof(waCaptureImage), NORMALPRIO+2, CaptureImage, NULL);
}
