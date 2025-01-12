#include <stdint.h>

#define THRESHOLD_ANGLE 23.6 // 23.6 degrees

// Flag to indicate whether the display is rotated (0: not rotated, 1: rotated)
extern uint8_t is_rotated;

extern void init_accelerometer(void);
void read_accelerometer(int* y);
float calculate_angle(int axis_value, int reference_value);
extern void check_orientation(void);
