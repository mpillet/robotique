#ifndef PROCESS_IMAGE_H
#define PROCESS_IMAGE_H

#define WORKING_AREA_CAPT_IMAGE		256
#define WORKING_AREA_PROC_IMAGE		1024
#define IMAGE_BUFFER_SIZE			640
#define WIDTH_SLOPE					15
#define MIN_LINE_WIDTH				70
#define MAX_PIX						400
#define TWO_LINES					2
#define FIRST_PIXEL					0
#define BOTTOM_LINE					479
#define RED_PIX_MASK				0xF8

bool get_state(void);
void process_image_start(void);

#endif /* PROCESS_IMAGE_H */
