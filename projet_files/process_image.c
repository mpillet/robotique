#include "ch.h"
#include "hal.h"
#include <chprintf.h>
#include <usbcfg.h>

#include <main.h>
#include <camera/po8030.h>

#include <process_image.h>





//semaphore
static BSEMAPHORE_DECL(image_ready_sem, TRUE);
static uint8_t angle = 0;

/*
 *  Returns the line's width extracted from the image buffer given
 *  Returns 0 if line not found
 */

static THD_WORKING_AREA(waCaptureImage, 256);
static THD_FUNCTION(CaptureImage, arg) {

    chRegSetThreadName(__FUNCTION__);
    (void)arg;

	//Takes pixels 0 to IMAGE_BUFFER_SIZE of the line 10 + 11 (minimum 2 lines because reasons)
	po8030_advanced_config(FORMAT_RGB565, 0, 10, IMAGE_BUFFER_SIZE, 2, SUBSAMPLING_X1, SUBSAMPLING_X1);
	dcmi_enable_double_buffering();
	dcmi_set_capture_mode(CAPTURE_ONE_SHOT);
	dcmi_prepare();

    while(1){
        //starts a capture
		dcmi_capture_start();
		//waits for the capture to be done
		wait_image_ready();
		//signals an image has been captured
		chBSemSignal(&image_ready_sem);
    }
}


static THD_WORKING_AREA(waProcessImage, 4096);
static THD_FUNCTION(ProcessImage, arg)
{
    chRegSetThreadName(__FUNCTION__);
    (void)arg;
	uint8_t *img_buff_ptr;
	uint8_t image_r[IMAGE_BUFFER_SIZE] = {0};
	uint8_t image_g[IMAGE_BUFFER_SIZE] = {0};
	uint8_t image_b[IMAGE_BUFFER_SIZE] = {0};
	//bool send_to_computer = true;

    while(1)
    {
    	//waits until an image has been captured
        chBSemWait(&image_ready_sem);
		//gets the pointer to the array filled with the last image in RGB565    
		img_buff_ptr = dcmi_get_last_image_ptr();

		for(uint16_t i = 0 ; i < (2 * IMAGE_BUFFER_SIZE) ; i+=2)
		{
			image_r[i/2] = (uint8_t)img_buff_ptr[i]&0xF8 >> 2;		//Extracts only the red pixels
			image_g[i/2] = (uint8_t)img_buff_ptr[i]&0x07 << 3;		//Extracts only the 3 first green pixels
			image_g[i/2] |= (uint8_t)img_buff_ptr[i+1]&0xE0 >> 5;		//Extracts only the 3 last green pixels
			image_b[i/2] = (uint8_t)img_buff_ptr[i+1]&0x1F << 1;		//Extracts only the blue pixels
		}



		angle = find_angle(image_r, image_g, image_b);
		chprintf((BaseSequentialStream *)&SD3, "angle = %d\n", angle);


//		if(send_to_computer)
//		{
//			//sends to the computer the image
//			SendUint8ToComputer(image, IMAGE_BUFFER_SIZE);
//
//		}
		//invert the bool
		//send_to_computer = !send_to_computer;
    }
}


uint8_t find_angle(uint8_t *buffer_r, uint8_t *buffer_g, uint8_t *buffer_b)
{
	uint32_t mean_r = 0, mean_g = 0, mean_b = 0;

	//calculate the mean for one line and for each color (red, green, blue)
	for(uint16_t i = 0 ; i < IMAGE_BUFFER_SIZE ; i++)
	{
		mean_r += buffer_r[i];
		mean_g += buffer_g[i];
		mean_b += buffer_b[i];
	}
	mean_r /= IMAGE_BUFFER_SIZE;
	mean_g /= IMAGE_BUFFER_SIZE;
	mean_b /= IMAGE_BUFFER_SIZE;

//	chprintf((BaseSequentialStream *)&SD3,"r = %d, g = %d, b = %d\n", mean_r, mean_g, mean_b);
//	chprintf((BaseSequentialStream *)&SD3, "g = %d\n", mean_g);
//	chprintf((BaseSequentialStream *)&SD3, "b = %d\n", mean_b);

	if((mean_r > mean_b) && (mean_r > mean_g))
	{
		return ANGLE_90_LEFT;
	}
	else if((mean_g > mean_r) && (mean_g > mean_b))
	{
		return ANGLE_45_LEFT;
	}
	else if((mean_b > mean_r) && (mean_b > mean_g))
	{
		return ANGLE_45_RIGHT;
	}
	else
	{
		return ANGLE_90_RIGHT;
	}

}



void process_image_start(void)
{
	chThdCreateStatic(waProcessImage, sizeof(waProcessImage), NORMALPRIO, ProcessImage, NULL);
	chThdCreateStatic(waCaptureImage, sizeof(waCaptureImage), NORMALPRIO, CaptureImage, NULL);
}
