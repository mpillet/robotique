#ifndef MAIN_H
#define MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

#include "camera/dcmi_camera.h"
#include "msgbus/messagebus.h"
#include "parameter/parameter.h"


//constants for the differents parts of the project
#define OFF						8
#define IMAGE_BUFFER_SIZE		640
#define WIDTH_SLOPE				15
#define MIN_LINE_WIDTH			70
#define ROTATION_THRESHOLD		10
#define ROTATION_COEFF			2 
#define PXTOCM					1570.0f //experimental value
#define MAX_PIX					400
#define STOP_LINE_WIDTH			300
#define MAX_DISTANCE 			25.0f
#define ERROR_THRESHOLD			1 //[cm] because of the noise of the camera
#define KP						10.
#define KI 						3.5f	//must not be zero
#define CORRECTION				150.
#define MAX_SUM_ERROR 			(MOTOR_SPEED_LIMIT/KI)
#define STOP					1
#define CONTINUE				0
#define DEFAULT_SPEED			200
#define LOW_SPEED				400
#define MIDDLE_SPEED			600
#define HIGH_SPEED				800
#define LOW_CNT 				8000000
#define MIDDLE_CNT				4000000
#define HIGH_CNT				2000000
#define MAX_CNT					1000000





/** Robot wide IPC bus. */
extern messagebus_t bus;

extern parameter_namespace_t parameter_root;

void SendUint8ToComputer(uint8_t* data, uint16_t size);

#ifdef __cplusplus
}
#endif

#endif
