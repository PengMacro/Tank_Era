#ifndef __DEBOUNCE_H__
#define __DEBOUNCE_H__

#include <stdbool.h>
#include <stdint.h>
#include "launchpad_io.h"
#include "mcp23017.h"
#include "serial_debug.h"

// Four directional buttons correspond to these GPIOB pins on the IO expander
#define RIGHT_BUTTON_PIN			3
#define LEFT_BUTTON_PIN				2
#define DOWN_BUTTON_PIN				1
#define UP_BUTTON_PIN					0


extern void check_debounce(uint8_t, bool *,bool *,bool *,bool *,bool *);

#endif