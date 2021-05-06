#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "ch.h"
#include "hal.h"
#include "memory_protection.h"
#include <usbcfg.h>
#include <main.h>
#include <motors.h>
#include <camera/po8030.h>
#include <msgbus/messagebus.h>
#include <sensors/proximity.h>
#include <chprintf.h>
#include <process_image.h>
#include <audio/play_melody.h>
#include <process_audio.h>
#include <process_speed.h>
#include <audio/microphone.h>
#include <audio/audio_thread.h>


messagebus_t bus;
MUTEX_DECL(bus_lock);
CONDVAR_DECL(bus_condvar);


#define SEND_FROM_MIC



static void serial_start(void)
{
	static SerialConfig ser_cfg = {
	    115200,
	    0,
	    0,
	    0,
	};

	sdStart(&SD3, &ser_cfg); // UART3.
}

int main(void)
{
    halInit();
    chSysInit();
    mpu_init();
    //starts the serial communication
    serial_start();
    //start the USB communication
    usb_start();
    //starts the camera
    dcmi_start();
	po8030_start();
	//inits the motors
	motors_init();
	messagebus_init(&bus, &bus_lock, &bus_condvar);

	//init sensors
	proximity_start();
	calibrate_ir();
	//init melody
	playMelodyStart();
	dac_start();

    //lancement des threads
    //stars the threads for the pi regulator and the processing of the image
    pi_regulator_start();
    process_image_start();

#ifdef SEND_FROM_MIC
    //starts the microphones processing thread.
    //it calls the callback given in parameter when samples are ready
    mic_start(&processAudioData);
#endif  /* SEND_FROM_MIC */


    /* Infinite loop. */
    while (1) {
    	//waits 1 second
        chThdSleepMilliseconds(1000);
    }
}

#define STACK_CHK_GUARD 0xe2dee396
uintptr_t __stack_chk_guard = STACK_CHK_GUARD;

void __stack_chk_fail(void)
{
    chSysHalt("Stack smashing detected");
}
