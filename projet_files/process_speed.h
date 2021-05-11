#ifndef PROCESS_SPEED_H
#define PROCESS_SPEED_H

#define WORKING_AREA_SPEED		256
#define OFF						8
#define FINISHED				6
#define NOT_READY				0
#define READY					1
#define LETHAL_SPACE			200
#define VITAL_SPACE				5
#define WALK_SPEED				200
#define KP						10
#define CORRECTION				150
#define SENSOR4					3
#define SENSOR5					4
#define SENSOR1					0
#define SENSOR8					7
#define TURN_OFF				0
#define TURN_ON					1
#define LOW_SPEED				400
#define MIDDLE_SPEED			600
#define HIGH_SPEED				800
#define LOW_CNT 				8000000
#define MIDDLE_CNT				4000000
#define HIGH_CNT				2000000
#define MAX_CNT					1000000

void animation(uint16_t);
void process_speed_start(void);
bool get_ready_to_turn(void);
void clear_ready_to_turn(void);

#endif /* PROCESS_SPEED */
