#ifndef PROCESS_AUDIO_H
#define PROCESS_AUDIO_H


#define FFT_SIZE 	1024


bool get_ready_to_go(void);
void clear_ready_to_go(void);
void processAudioData(int16_t *, uint16_t);
void doFFT_optimized(uint16_t size, float* complex_buffer);


#endif /* PROCESS_AUDIO */
