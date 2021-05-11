#ifndef PROCESS_AUDIO_H
#define PROCESS_AUDIO_H

#define FFT_SIZE 				1024
#define MIN_VALUE_THRESHOLD 	10000
#define MIN_INDEX				-1
#define MIN_FREQ 				10 	//we don’t analyze before this index to not use resources for nothing
#define LEFT_FREQ 				15	//230 Hz
#define RIGHT_FREQ 				20	//300 Hz
#define MAX_FREQ 				30 	//we don’t analyze after this index to not use resources for nothing
#define MARGE					1
#define TURN_SPEED				300

bool get_ready_to_go(void);
void clear_ready_to_go(void);
void processAudioData(int16_t *, uint16_t);
void doFFT_optimized(uint16_t size, float* complex_buffer);

#endif /* PROCESS_AUDIO */
