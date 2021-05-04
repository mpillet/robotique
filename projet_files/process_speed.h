#ifndef PROCESS_SPEED_H
#define PROCESS_SPEED_H

//start the PI regulator thread
void pi_regulator_start(void);
bool get_ready_to_turn(void);
void clear_ready_to_turn(void);
void animation(uint16_t);


#endif /* PROCESS_SPEED */
