// Copyright (c) 2015-16, Joe Krachey
// All rights reserved.
//
// Redistribution and use in source or binary form, with or without modification, 
// are permitted provided that the following conditions are met:
//
// 1. Redistributions in source form must reproduce the above copyright 
//    notice, this list of conditions and the following disclaimer in 
//    the documentation and/or other materials provided with the distribution.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" 
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, 
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR 
// PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR 
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, 
// EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, 
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; 
// OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, 
// WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING 
// NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, 
// EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#ifndef __MAIN_H__
#define __MAIN_H__

#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "lcd.h"
#include "timers.h"
#include "ps2.h"
#include "launchpad_io.h"
#include "HW3_images.h"
#include "TM4C123.h"
#include "driver_defines.h"
#include "gpio_port.h"
#include "serial_debug.h"
#include "validate.h"
#include "timers.h"
#include "mcp23017.h"
#include "debounce.h"
#include "printString.h"
#include "ft6x06.h"
#include "pwm.h"
#include "spi_select.h"
#include "wireless.h"

#define BULLET_FREQUENCY 100
#define HIT_FREQUENCY 100
#define EXPLODE_FREQUENCY 100
#define PRESS_BUTTON_FREQUENCY 100
#define ONESECOND 50000000

// dir in moster, missle struct 
//#define UP 0
//#define DOWN 1
#define LEFT 2
#define RIGHT 3




extern void initialize_serial_debug(void);

///////////////////////////////
// Declare any custom types //
/////////////////////////////
typedef enum {IDLE_lr, RGHT, LFT} left_right_t;
typedef enum {IDLE_ud, UP, DOWN} up_down_t;

typedef struct {
	uint16_t x_loc;
	uint16_t y_loc;
} tank_t;

struct monster{
	uint16_t x_loc;
	uint16_t y_loc;
	bool prv_success;
	uint8_t dir;
	uint16_t num_steps;
	struct monster *prv;
	struct monster *nxt;
} ;

struct missle {
	uint16_t x_loc;
	uint16_t y_loc;
	uint8_t dir; //0: up 1: down 2: left 3: right
	struct missle *prv;
	struct missle *nxt;
};

typedef struct {
	uint16_t x_loc;
	uint16_t y_loc;
} bomb_t;

//////////////////////////////
// Function Prototype Next //
////////////////////////////
void add_missle(void);
bool remove_missle(struct missle* del_missle);


#endif