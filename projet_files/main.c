#include "ch.h"
#include "hal.h"
#include "memory_protection.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include <main.h>
#include <process_image.h>
#include <process_audio.h>
#include <process_speed.h>

#include <motors.h>
#include <camera/po8030.h>
#include <msgbus/messagebus.h>
#include <sensors/proximity.h>
#include <audio/play_melody.h>
#include <audio/microphone.h>
#include <audio/audio_thread.h>

#define SEND_FROM_MIC

messagebus_t bus;
MUTEX_DECL(bus_lock);
CONDVAR_DECL(bus_condvar);

int main(void)
{
	//initialisations
    halInit();
    chSysInit();
    mpu_init();
    dcmi_start();		//camera
	po8030_start();
	motors_init();		//motors
	messagebus_init(&bus, &bus_lock, &bus_condvar);
	proximity_start();	//sensors
	calibrate_ir();
	playMelodyStart();	//melody
	dac_start();

    //starts the threads
    process_speed_start();
    process_image_start();
	#ifdef SEND_FROM_MIC
		mic_start(&processAudioData);
	#endif  /* SEND_FROM_MIC */

    /* Infinite loop. */
    while (1)
    {
        chThdSleepMilliseconds(ONE_SECOND);
    }
}

#define STACK_CHK_GUARD 0xe2dee396
uintptr_t __stack_chk_guard = STACK_CHK_GUARD;

void __stack_chk_fail(void)
{
    chSysHalt("Stack smashing detected");
}
