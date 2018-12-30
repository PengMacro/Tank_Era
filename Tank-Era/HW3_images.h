#ifndef __HW3_IMAGES_H__
#define __HW3_IMAGES_H__

#include <stdint.h>
#include <stdio.h>

#define       PLANE_HEIGHT 	58	// height of plane image in pixels
#define       PLANE_WIDTH  	47	// width of plane image in pixels
#define				MISSLE_HEIGHT	11	// height of missle image in pixels
#define				MISSLE_WIDTH	7		// width of missle image in pixels
#define				WELCOME_PAGE_WIDTH 240
#define				WELCOME_PAGE_HEIGHT 200
#define				BUTTON_WIDTH 100
#define				BUTTON_HEIGHT 80
#define				INSTRUCTION_WIDTH 40
#define				INSTRUCTION_HEIGHT 40
#define				ERASE_BLOCK_WIDTH 40
#define				ERASE_BLOCK_HEIGHT 40
#define				SHELL_WIDTH 20
#define				SHELL_HEIGHT 24
#define				MAP_WIDTH 240
#define				MAP_HEIGHT 280
#define				TANK_WIDTH 14
#define			  TANK_HEIGHT 14
#define				BULLET_WIDTH 5
#define			  BULLET_HEIGHT 5
#define				TITLE_WIDTH 240
#define			  TITLE_HEIGHT 40
#define				HEART_WIDTH 20
#define			  HEART_HEIGHT 20
#define				BOMB_WIDTH 18
#define			  BOMB_HEIGHT 18
#define				IN_MAP_BOMB_WIDTH 10
#define			  IN_MAP_BOMB_HEIGHT 10
#define				MONSTER_WIDTH 15
#define			  MONSTER_HEIGHT 15
#define				EXPLOSION_WIDTH 10
#define			  EXPLOSION_HEIGHT 10
#define				LOST_WIDTH 240
#define			  LOST_HEIGHT 320
#define				WIN_WIDTH 240
#define			  WIN_HEIGHT 320


extern const uint8_t planeBitmap[];
extern const uint8_t missleBitmap[];
extern const uint8_t missleErase[];
extern const uint8_t ch26_59_Patton_LegionBitmaps[];
extern const uint8_t play_button[];
extern const uint8_t multiple_players[];
extern const uint8_t erase_block[];
extern const uint8_t shellBitmaps[];
extern const uint8_t instructionBitmaps[];
extern const uint8_t mapBitmaps[];
extern const uint8_t tank_up[];
extern const uint8_t tank_down[];
extern const uint8_t tank_left[];
extern const uint8_t tank_right[];
extern const uint8_t bullet[];
extern const uint8_t bulletErase[];
extern const uint8_t title[];
extern const uint8_t hearts[];
extern const uint8_t bomb[];
extern const uint8_t in_map_bomb[];
extern const uint8_t in_map_bomb_erase[];
extern const uint8_t monster[];
extern const uint8_t explosion[];
extern const uint8_t lostGraph[];
extern const uint8_t winGraph[];
#endif
