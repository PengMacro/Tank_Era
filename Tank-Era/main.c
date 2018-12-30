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

#include "main.h"

char group[] = "Group??";
char individual_1[] = "Pengfei Zhu";
char individual_2[] = "Peng Cheng";

///////////////////////////
// Global declared here //
/////////////////////////
#define MASTER  false

uint8_t myID[]      = { 3,5,3,8,8};
uint8_t remoteID[]  = { 3,5,3,2,4};

wireless_com_status_t   status_master;
wireless_com_status_t   status_slave;

// states
#define WELCOME 0
#define INSTRUCTION 1
#define PLAYING 2
#define END 3


uint8_t state = WELCOME; // default state

uint8_t pin_logic_level; // for expanded buttons


bool printed_background = false;
bool print_left = false;
bool print_right = true;

bool alert_up = false;
bool alert_down = false;
bool alert_left = false;
bool alert_right = false;

bool first_drawn = true;

bool head_up = false;
bool head_down = false;
bool head_left = false;
bool head_right = false;

bool fire_up = false;
bool fire_down = false;
bool fire_left = false;
bool fire_right = false;

bool touch_right_border = false;
bool touch_left_border = false;
bool touch_up_border = false;
bool touch_down_border = false;

bool touch_up = false;
bool touch_down = false;
bool touch_left = false;
bool touch_right = false;
bool able_to_fire = true;
int steps = 30;



int bomb_count = 2;
int first_bomb_update_count = 10;
int second_bomb_update_count = 10;
bool first_explode = false;
bool second_explode = false;
int life_count = 3;
int parity = 0;
int temp_frequency = 700;
int temp_frequency_result = 500;
bool UP_SCROLL = false;

bool win = true;
bool lose = true;

uint32_t my_kills = 0;
uint32_t other_kills = 0;


int monster_count = 0;

uint32_t xvalue = 0;
uint32_t yvalue = 0;

bool already_launched = false;
volatile bool joystick_int = false;
volatile int color_change = false;
volatile bool ALERT_MISSLE_UPDATE = false;
volatile bool ALERT_BOMB_UPDATE1 = false;
volatile bool GENERATE_MONSTER = false;
volatile bool ALERT_MONSTER_UPDATE = false;
volatile bool CHANGE_MONSTER_EXPLOSION_UPDATE = false;
volatile bool ALERT_SCORE_UPDATE = false;
volatile bool COLLISION_HARM =true;
volatile bool TONE_UPDATE = false;
volatile bool collision_happened =false;
bool received_other_result = false;

int frequency = 500;

left_right_t joystick_left_right;
up_down_t joystick_up_down;

tank_t tank;
bomb_t first_bomb;
bomb_t second_bomb;

struct missle * m_head = NULL;
struct missle * m_curr = NULL;
struct missle * m_tail = NULL;

struct monster * monster_head = NULL;
struct monster * monster_curr = NULL;
struct monster * monster_tail = NULL;

void ADC0SS2_Handler(void){
	static int count = 1;
	int i;
	char msg[80];
	int threshold_left;
	int threshold_right;
	int threshold_up;
	int threshold_down;
	threshold_left = 0xfff * 0.85;
	threshold_right = 0xfff * 0.15;
	threshold_up = 0xfff * 0.85;
	threshold_down = 0xfff * 0.15;
	xvalue = ADC0->SSFIFO2 & 0xFFF;    // Read 12-bit data
	yvalue = ADC0->SSFIFO2 & 0xFFF;    // Read 12-bit data
	ADC0->ISC  = ADC_ISC_IN2;          // Ack the conversion
	//y direction
	if(xvalue > threshold_left){
		joystick_left_right = LFT;
		//sprintf(msg,"x value : 0x%03x\n\r",xvalue);
		//put_string(msg);
	}else if (xvalue < threshold_right) {
		joystick_left_right = RGHT;
		//sprintf(msg,"x value : 0x%03x\n\r",xvalue);
		//put_string(msg);
	}else{
		joystick_left_right = IDLE_lr;
	}
	//x direction
	if(yvalue > threshold_up){
		joystick_up_down = UP;
		//sprintf(msg,"y value : 0x%03x\n\r",yvalue);
		//put_string(msg);
	}else if (yvalue < threshold_down) {
		joystick_up_down = DOWN;
		//sprintf(msg,"y value : 0x%03x\n\r",yvalue);
		//put_string(msg);
	}else{
		joystick_up_down = IDLE_ud;
	}
}

// 10ms
void TIMER0A_Handler(void){
	uint8_t last_4_bits;
	bool pressed = false;
	static int count1 = 0;
	static int count2 = 0;
	static int count3 = 0;
	static int count4= 0;
	static int count5 = 0;
	static int count6 = 0;
	
	if(count1 <= 20) lp_io_set_pin(RED_BIT);
	else 	lp_io_clear_pin(RED_BIT);
	
	count1 = count1++;
	
	if(count1 >= 40) count1 = 0;
	
	if(count2 <= 100) 						 color_change = 0;
	else if (count2 >= 100 && count2 <= 200) color_change = 1;
	else 									 color_change = 2;
	
	if(count2 >=300) count2 = 0;
	count2++;
	
    TIMER0->ICR |= TIMER_ICR_TATOCINT;
	
	get_button_data(&pin_logic_level);
	last_4_bits = pin_logic_level & 0xF;
	
	if(last_4_bits < 0xF) pressed = true;
	
	if(pressed&&!already_launched){//if down and never launched before
		check_debounce(last_4_bits,&already_launched,&alert_up,&alert_down,&alert_left,&alert_right);
	}
	
	if(pin_logic_level == 0xF){// if up
		already_launched = false;//previous launch has been handled
		alert_up = false;
		alert_down = false;
		alert_left = false;
		alert_right = false;
	}
	
	ALERT_MISSLE_UPDATE = true;
	count3++;
	if(count3 == 20){
		count3 = 0;
		ALERT_BOMB_UPDATE1 = true;
	}
	
	count4++;
	if(count4 == frequency){
		count4 = 0;
		GENERATE_MONSTER = true;
	}
	if(!collision_happened) count5 = 0;
	count5++;
	
	if(count5 == 200){
		collision_happened = false;
		count5 = 0;
		COLLISION_HARM = true;
		able_to_fire = true;
	}
	
	ALERT_MONSTER_UPDATE = true;
	count6++;
	if(count6 == 20){
		count6 = 0;
		ALERT_SCORE_UPDATE = true;
		TONE_UPDATE = true;
	}
}


// 20ms
void TIMER0B_Handler(void){
	static int count2 = 0;
	if(count2 >= 20) lp_io_set_pin(GREEN_BIT);
	else 			 lp_io_clear_pin(GREEN_BIT);
	
	count2 = count2++;
	if(count2 >= 40) count2 = 0;
	
	
	TIMER0->ICR |= TIMER_ICR_TBTOCINT;
	
	//set the SSMUX2 to 0001|0000 -> start from AIN 0 and move to AIN1
	generate_adc_read(PS2_ADC_BASE,0x10);
	if(joystick_left_right != IDLE_lr || joystick_up_down != IDLE_ud){
		joystick_int = true;
	}
}

// interrupts
void DisableInterrupts(void)
{
  __asm {
         CPSID  I
  }
}
void EnableInterrupts(void)
{
  __asm {
    CPSIE  I
  }
}


void initialize_hardware(void){
	DisableInterrupts();
	
	init_serial_debug(true, true);
	
	spi_select_init();
	spi_select(NORDIC);
	wireless_initialize();
	
	// setup lcd GPIO, config the screen, and clear it
	lcd_config_screen();
	lcd_clear_screen(LCD_COLOR_BLACK);
	
	// for touch screen  
	ft6x06_init();
	
	// for port expander
	mcp23017_init();
	configure_buttons();

	// for two timers 
	gp_timer_config_16(TIMER0_BASE,TIMER_TAMR_TAMR_PERIOD,false,true);
	gp_timer_config_32(TIMER2_BASE, TIMER_TAMR_TAMR_1_SHOT, false, false);
	
	// setup GPIO for LED drive
	lp_io_init();

	// Setup ADC to convert on PS2 joystick using SS2 and interrupts
	ps2_initialize(); 
	
	// for buzzer
	pwm_init();

	EnableInterrupts();
}




void add_missle(){
	
	// setup tank borders
	int left_lane = tank.x_loc-TANK_WIDTH/2;
	int right_lane = tank.x_loc+TANK_WIDTH/2;
	int top_lane = tank.y_loc-TANK_HEIGHT/2;
	int btm_lane = tank.y_loc+TANK_HEIGHT/2;
	
	struct missle* new_missle;
	new_missle = (struct missle *)malloc(sizeof(struct missle));
	
	// missle hit walls, not add
	if(top_lane == 41 && head_up){
		return;
	}
	if(top_lane == 42 && head_up){
		return;
	}
	if(top_lane == 43 && head_up){
		return;
	}
	if(btm_lane == ROWS-1&& head_down){
		return;
	}
	if(left_lane == 0&& head_left){
		return;
	}
	if(right_lane == COLS-1 && head_right){
		return;
	}	
		if(top_lane <= 159 && btm_lane >= 60){
			if(right_lane == 17 && head_right){
				return;
			}
			if(left_lane == 38&& head_left){//left boarder
			return;
			}
		}
		if(left_lane <= 38 && right_lane >= 17){
			if(btm_lane == 60&& head_down){//down boarder
			return;
		}
			if(top_lane == 160&& head_up){//up boarder
			return;
		}
		}
		if(top_lane <= 158 && btm_lane >= 60){
			if(right_lane == 54&& head_right){
				return;
			}
			if(left_lane == 75&& head_left){//left boarder
			return;
			}
		}
		if(left_lane <= 75 && right_lane >= 54){
			if(btm_lane == 60&& head_down){//down boarder
			return;
		}
			if(top_lane == 158&& head_up){//up boarder
			return;
		}
		}
		if(top_lane <= 82 && btm_lane >= 60){
			if(right_lane == 92&& head_right){
				return;
			}
			if(left_lane == 113&& head_left){//left boarder
			return;
			}
		}
		if(left_lane <= 113 && right_lane >= 92){
			if(btm_lane == 60&& head_down){//down boarder
			return;
		}
			if(top_lane == 82&& head_up){//up boarder
			return;
		}
		}
		if(top_lane <= 137 && btm_lane >= 104){
			if(right_lane == 92&& head_right){
				return;
			}
			if(left_lane == 113&& head_left){//left boarder
			return;
			}
		}
		if(left_lane <= 113 && right_lane >= 92){
			
			if(btm_lane == 104&& head_down){//down boarder
			return;
		}
			if(top_lane == 137&& head_up){//up boarder
			return;
		}
		}
		if(top_lane <= 82 && btm_lane >= 60){
			if(right_lane == 130&& head_right){
				return;
			}
			if(left_lane == 149&& head_left){//left boarder
			return;
			}
		}
		if(top_lane <= 102 && btm_lane >= 83){
			if(right_lane == 138&& head_right){
				return;
			}
			if(left_lane == 149&& head_left){//left boarder
			return;
			}
		}
		if(top_lane <= 137 && btm_lane >= 103){
			if(right_lane == 130&& head_right){
				return;
			}
			if(left_lane == 149&& head_left){//left boarder
			return;
			}
		}
		if(left_lane <= 148 && right_lane >= 129){
			if(btm_lane == 59&& head_down){//down boarder
			return;
		}
			if(top_lane == 138&& head_up){//up boarder
			return;
		}
		}
		if(right_lane >= 131 && right_lane <= 138){			

			if(btm_lane == 104&& head_down){//down boarder
			return;
		}
			if(top_lane == 83&& head_up){//up boarder
			return;
		}
		}
		if(top_lane <= 158 && btm_lane >= 61){
			if(right_lane == 167&& head_right){
				return;
			}
			if(left_lane == 187&& head_left){//left boarder
			return;
			}
		}
		if(left_lane <= 186 && right_lane >= 168){
			if(btm_lane == 60&& head_down){//down boarder
			return;
		}
			if(top_lane == 159&& head_up){//up boarder
			return;
		}
		}	
		if(top_lane <= 159 && btm_lane >= 61){
			if(right_lane == 205&& head_right){
				return;
			}
			if(left_lane == 225&& head_left){//left boarder
			return;
			}
		}
		if(left_lane <= 224 && right_lane >= 206){
			
			if(btm_lane == 60&& head_down){//down boarder
			return;
		}
			if(top_lane == 160&& head_up){//up boarder
			return;
		}
		}
		if(top_lane <= 181 && btm_lane >= 159){
			if(right_lane == 92&& head_right){
			return;
			}
			if(left_lane == 113&& head_left){//left boarder
			return;
			}
		}
		if(left_lane <= 112 && right_lane >= 93){
			
			if(btm_lane == 158&& head_down){//down boarder
			return;
		}
			if(top_lane == 182&& head_up){//up boarder
		return;
		}
		}
		if(top_lane <= 181 && btm_lane >= 159){
			if(right_lane == 130&& head_right){
			return;
			}
			if(left_lane == 146&& head_left){//left boarder
			return;
			}
		}
		if(left_lane <= 145 && right_lane >= 131){
			
			if(btm_lane == 158&& head_down){//down boarder
			return;
		}
			if(top_lane == 182&& head_up){//up boarder
			return;
		}
		}
		
		if(top_lane <= 203 && btm_lane >= 181){
			if(right_lane == 167&& head_right){
				return;
			}
			if(left_lane == 206&& head_left){//left boarder
			return;
			}
		}
		if(left_lane <= 205 && right_lane >= 168){
			
			if(btm_lane == 180&& head_down){//down boarder
			return;
		}
			if(top_lane == 204&& head_up){//up boarder
			return;
		}
		}
		
		if(top_lane <= 203 && btm_lane >= 186){
			if(right_lane == 35&& head_right){
			return;
			}
			if(left_lane == 75&& head_left){//left boarder
			return;
			}
		}
		if(left_lane <= 74 && right_lane >= 36){
			if(btm_lane == 185&& head_down){//down boarder
			return;
		}
			if(top_lane == 204&& head_up){//up boarder
			return;
		}
		}
		if(top_lane <= 185 && btm_lane >= 181){
			if(right_lane == 54&& head_right){
				return;
			}
			if(left_lane == 75&& head_left){//left boarder
			return;
			}
		}
		if(left_lane <= 74 && right_lane >= 55){
			if(btm_lane == 180&& head_down){//down boarder
			return;
		}
		}
		if(top_lane <= 302 && btm_lane >= 225){
			if(right_lane == 17&& head_right){
			return;
			}
			if(left_lane == 37&& head_left){//left boarder
			return;
			}
		}
		if(left_lane <= 36 && right_lane >= 18){
			if(btm_lane == 224&& head_down){//down boarder
			return;
		}
			if(top_lane == 303&& head_up){//up boarder
			return;
		}
		}
		if(top_lane <= 302 && btm_lane >= 225){
			if(right_lane == 54&& head_right){
				return;
			}
			if(left_lane == 75&& head_left){//left boarder
			return;
			}
		}
		if(left_lane <= 74 && right_lane >= 55){
			if(btm_lane == 224&& head_down){//down boarder
			return;
		}
			if(top_lane == 303&& head_up){//up boarder
			return;
		}
		}
		if(top_lane <= 268 && btm_lane >= 225){
			if(right_lane == 91&& head_right){
			return;
			}
			if(left_lane == 112&& head_left){//left boarder
			return;
			}
		}
		if(left_lane <= 111 && right_lane >= 92){
			if(btm_lane == 224&& head_down){//down boarder
			return;
		}
			if(top_lane == 269&& head_up){//up boarder
			return;
		}
		}
		if(top_lane <= 268 && btm_lane >= 225){
			if(right_lane == 129&& head_right){
				return;
			}
			if(left_lane == 151&& head_left){//left boarder
			return;
			}
		}
		if(left_lane <= 150 && right_lane >= 130){
			if(btm_lane == 224&& head_down){//down boarder
			return;
		}
			if(top_lane == 269&& head_up){//up boarder
			return;
		}
		}
		if(left_lane <= 149 && right_lane >= 93){
			if(btm_lane == 224&& head_down){//down boarder
			return;
		}
			if(top_lane == 236&& head_up){//up boarder
			return;
		}
		}
		if(top_lane <= 301 && btm_lane >= 225){
			if(right_lane == 167&& head_right){
				return;
			}
			if(left_lane == 188&& head_left){//left boarder
			return;
			}
		}
		if(left_lane <= 187 && right_lane >= 168){
			if(btm_lane == 224&& head_down){//down boarder
			return;
		}
			if(top_lane == 302&& head_up){//up boarder
			return;
		}
	}
		if(top_lane <= 301 && btm_lane >= 225){
			if(right_lane == 205&& head_right){
			return;
			}
			if(left_lane == 225&& head_left){//left boarder
			return;
			}
		}
		if(left_lane <= 224 && right_lane >= 206){
			if(btm_lane == 224&& head_down){//down boarder
			return;
		}
			if(top_lane == 302&& head_up){//up boarder
			return;
		}
	}
	
	if(head_up){
		new_missle->dir = 0;
		new_missle->x_loc = tank.x_loc;
		new_missle->y_loc = tank.y_loc -2;
	}
	if(head_down){
		new_missle->dir = 1;
		new_missle->x_loc = tank.x_loc;
		new_missle->y_loc = tank.y_loc +2;
	}
	if(head_left){
		new_missle->dir = 2;
		new_missle->x_loc = tank.x_loc - 2;
		new_missle->y_loc = tank.y_loc;
	}
	if(head_right){
		new_missle->dir = 3;
		new_missle->x_loc = tank.x_loc +2;
		new_missle->y_loc = tank.y_loc;
	}
	
	//set up head and tail
	if(m_head == NULL){
		m_head = new_missle;
		m_tail = new_missle;
		m_head->nxt = NULL;
		m_head->prv = NULL;
		return;
	}
	//link with the previous tail
	new_missle->prv = m_tail;//if new missle is the tail, the prv of tail is itself
	new_missle->nxt = NULL;
	m_tail->nxt = new_missle;
	//new tail
	m_tail = new_missle;	
}
bool remove_missle(struct missle* del_missle){
	if(del_missle == NULL){
		return false;
	}
	//printf("next missle location: %d and %d\n",del_missle->nxt->x_loc,del_missle->nxt->y_loc);
	if(del_missle == m_head){
		m_head = del_missle->nxt;
		free(del_missle);
		del_missle = NULL;
		m_curr = NULL;
		return true;
	}
	(del_missle->nxt)->prv = del_missle->prv;
	(del_missle->prv)->nxt = del_missle->nxt;
	m_curr = del_missle->nxt;
	free(del_missle);
	del_missle = NULL;
	return true;
}
//this function renders the welcome page
void initialize_wecome_page(){
	char msg1[] = "W E L C O M E";
	char msg2[] = "*^.^* TANK  ERA *^.^*";
	
	if(color_change == 0){
		  lcd_print_stringXY(msg2,0,1,LCD_COLOR_BLACK,LCD_COLOR_WHITE);
	}else if (color_change == 1){
		  lcd_print_stringXY(msg2,0,1,LCD_COLOR_RED,LCD_COLOR_WHITE);
	}else{
		  lcd_print_stringXY(msg1,4,1,LCD_COLOR_RED,LCD_COLOR_WHITE);
	}
	if(alert_up){
		lcd_clear_screen(LCD_COLOR_BLACK);
		state = INSTRUCTION;
		return;
	}
	if(TONE_UPDATE){
		TONE_UPDATE = false;
		if(!UP_SCROLL){
			temp_frequency = temp_frequency - 100;
		}else{
			temp_frequency = temp_frequency + 100;
		}
		
		enable_pwm(temp_frequency);
		if(temp_frequency <= 100){
			UP_SCROLL = ! UP_SCROLL;
		}
		if(temp_frequency >= 800){
			UP_SCROLL = ! UP_SCROLL;
		}
	}
	if(print_right && alert_down){ // the default position is at right, which is indicating matching state
		lcd_clear_screen(LCD_COLOR_BLACK);	
		state = INSTRUCTION;//1 is INSTRUCTION state
		return;
		}
	if(print_left && alert_down){ // the default position is at right, which is indicating matching state
		disable_pwm();
		state = PLAYING; //3 is playing state
		}
	if(joystick_left_right == RGHT){
		put_string("I chose right\n\r");
		print_right = true;
		print_left = false;
	}
	if(joystick_left_right == LFT){
		put_string("I chose left\n\r");
		print_left = true;
		print_right = false;

}
	
	if(print_right){
				lcd_draw_image(
									BUTTON_WIDTH + BUTTON_WIDTH/2,
									SHELL_WIDTH,
									ROWS - SHELL_HEIGHT/2,	
									SHELL_HEIGHT,
									shellBitmaps,
									LCD_COLOR_BLACK,
									LCD_COLOR_WHITE);
			//PLAY BUTTON
			lcd_draw_image(
									BUTTON_WIDTH/2,
									BUTTON_WIDTH,
									COLS + (ROWS - COLS)/2,	//SHOULD BE 280
									BUTTON_HEIGHT,
									play_button,
									LCD_COLOR_BLACK,
									LCD_COLOR_WHITE);
	}
	if(print_left){
				lcd_draw_image(
									BUTTON_WIDTH/2,
									SHELL_WIDTH,
									ROWS - SHELL_HEIGHT/2,	
									SHELL_HEIGHT,
									shellBitmaps,
									LCD_COLOR_BLACK,
									LCD_COLOR_WHITE);
				//SHARING BUTTON
	lcd_draw_image(
									BUTTON_WIDTH + BUTTON_WIDTH/2,
									BUTTON_WIDTH,
									COLS + (ROWS - COLS)/2,	//SHOULD BE 280
									BUTTON_HEIGHT,
									multiple_players,
									LCD_COLOR_BLACK,
									LCD_COLOR_WHITE);
	}
	if(!printed_background){
		printed_background = true;
		lcd_draw_image(
									COLS/2,
									WELCOME_PAGE_WIDTH,
									ROWS/2 - (ROWS - COLS)/2 + 20,
									WELCOME_PAGE_HEIGHT,
									ch26_59_Patton_LegionBitmaps,
									LCD_COLOR_BLACK,
									LCD_COLOR_WHITE);
	//PLAY BUTTON
	lcd_draw_image(
									BUTTON_WIDTH/2,
									BUTTON_WIDTH,
									COLS + (ROWS - COLS)/2,	//SHOULD BE 280
									BUTTON_HEIGHT,
									play_button,
									LCD_COLOR_BLACK,
									LCD_COLOR_WHITE);
	//SHARING BUTTON
	lcd_draw_image(
									BUTTON_WIDTH + BUTTON_WIDTH/2,
									BUTTON_WIDTH,
									COLS + (ROWS - COLS)/2,	//SHOULD BE 280
									BUTTON_HEIGHT,
									multiple_players,
									LCD_COLOR_BLACK,
									LCD_COLOR_WHITE);
	lcd_draw_image(
									2*BUTTON_WIDTH + ERASE_BLOCK_WIDTH/2,
									ERASE_BLOCK_WIDTH,
									COLS + (ERASE_BLOCK_HEIGHT)/2,	
									ERASE_BLOCK_HEIGHT,
									erase_block,
									LCD_COLOR_BLACK,
									LCD_COLOR_WHITE);
	lcd_draw_image(
									2*BUTTON_WIDTH + ERASE_BLOCK_WIDTH/2,
									ERASE_BLOCK_WIDTH,
									COLS + ERASE_BLOCK_HEIGHT + (ERASE_BLOCK_HEIGHT)/2,	
									ERASE_BLOCK_HEIGHT,
									erase_block,
									LCD_COLOR_BLACK,
									LCD_COLOR_WHITE);
	}
}

void instruction_state(){
	char msd1[] = "Instruction:";
	char msd2[] = "Move:";
	char msd2_1[] = "Joystick";
	char msd3[] = "Volume:";
	char msd3_1[] = "Slide Up/Down Screen";
	char msd4[] = "Pad:";
	char msd4_1[] = "Up:Place Bomb";
	char msd4_2[] = "Down: Fire!!!";
	char msd4_3[] = "Left: Use Omni-Shield";
	char msd4_4[] = "Right: Yield to Your Enemy :(";
	printf("INSTRUCTION!!!!!\n");
	printed_background = false;
	lcd_draw_image(
									INSTRUCTION_WIDTH/2,
									INSTRUCTION_WIDTH,
									INSTRUCTION_HEIGHT/2,	
									INSTRUCTION_HEIGHT,
									instructionBitmaps,
									LCD_COLOR_WHITE,
									LCD_COLOR_BLACK);
	lcd_print_stringXY(msd1,1,3,LCD_COLOR_WHITE,LCD_COLOR_BLACK);
	lcd_print_stringXY(msd2,1,5,LCD_COLOR_WHITE,LCD_COLOR_BLACK);
	lcd_print_stringXY(msd2_1,1,6,LCD_COLOR_RED,LCD_COLOR_BLACK);
	lcd_print_stringXY(msd3,1,8,LCD_COLOR_WHITE,LCD_COLOR_BLACK);
	lcd_print_stringXY(msd3_1,1,9,LCD_COLOR_RED,LCD_COLOR_BLACK);
	lcd_print_stringXY(msd4,1,11,LCD_COLOR_WHITE,LCD_COLOR_BLACK);
	lcd_print_stringXY(msd4_1,1,12,LCD_COLOR_RED,LCD_COLOR_BLACK);
	lcd_print_stringXY(msd4_2,1,14,LCD_COLOR_RED,LCD_COLOR_BLACK);
	lcd_print_stringXY(msd4_3,1,16,LCD_COLOR_RED,LCD_COLOR_BLACK);
	lcd_print_stringXY(msd4_4,1,18,LCD_COLOR_RED,LCD_COLOR_BLACK);
	if(TONE_UPDATE){
		TONE_UPDATE = false;
		if(UP_SCROLL){
			disable_pwm();
			UP_SCROLL = !UP_SCROLL;
		}else{
		  enable_pwm(temp_frequency);
			UP_SCROLL = !UP_SCROLL;
		}
	}
	if(alert_right){
		lcd_clear_screen(LCD_COLOR_BLACK);
		state = WELCOME;
		return;
	}
}
void matching_state(){
	printf("MATCHING!!!!!\n");
}
void setup_boundries(){
	int left_lane = tank.x_loc-TANK_WIDTH/2;
	int right_lane = tank.x_loc+TANK_WIDTH/2;
	int top_lane = tank.y_loc-TANK_HEIGHT/2;
	int btm_lane = tank.y_loc+TANK_HEIGHT/2;
		if((right_lane == COLS-1)){//right boarder
			touch_right_border = true;
		}else{
			touch_right_border = false;
		}
		if((left_lane == 0)){//left boarder
			touch_left_border = true;
		}else{
			touch_left_border = false;
		}
		if((btm_lane == ROWS-1)){//down boarder
			touch_down_border = true;
		}else{
			touch_down_border = false;
		}
		if((top_lane == 41)){//up boarder
			touch_up_border = true;
		}else{
			touch_up_border = false;
		}
		
		if(top_lane <= 159 && btm_lane >= 60){
			if(right_lane == 17){
				touch_right_border = true;
			}
			if(left_lane == 38){//left boarder
			touch_left_border = true;
			}
		}
		if(left_lane <= 38 && right_lane >= 17){
			if(btm_lane == 60){//down boarder
			touch_down_border = true;
		}
			if(top_lane == 160){//up boarder
			touch_up_border = true;
		}
		}
		if(top_lane <= 158 && btm_lane >= 60){
			if(right_lane == 54){
				touch_right_border = true;
			}
			if(left_lane == 75){//left boarder
			touch_left_border = true;
			}
		}
		if(left_lane <= 75 && right_lane >= 54){
			if(btm_lane == 60){//down boarder
			touch_down_border = true;
		}
			if(top_lane == 158){//up boarder
			touch_up_border = true;
		}
		}
		if(top_lane <= 82 && btm_lane >= 60){
			if(right_lane == 92){
				touch_right_border = true;
			}
			if(left_lane == 113){//left boarder
			touch_left_border = true;
			}
		}
		if(left_lane <= 113 && right_lane >= 92){
			if(btm_lane == 60){//down boarder
			touch_down_border = true;
		}
			if(top_lane == 82){//up boarder
			touch_up_border = true;
		}
		}
		if(top_lane <= 137 && btm_lane >= 104){
			if(right_lane == 92){
				touch_right_border = true;
			}
			if(left_lane == 113){//left boarder
			touch_left_border = true;
			}
		}
		if(left_lane <= 113 && right_lane >= 92){
			
			if(btm_lane == 104){//down boarder
			touch_down_border = true;
		}
			if(top_lane == 137){//up boarder
			touch_up_border = true;
		}
		}
		if(top_lane <= 82 && btm_lane >= 60){
			if(right_lane == 130){
				touch_right_border = true;
			}
			if(left_lane == 149){//left boarder
			touch_left_border = true;
			}
		}
		if(top_lane <= 102 && btm_lane >= 83){
			if(right_lane == 138){
				touch_right_border = true;
			}
			if(left_lane == 149){//left boarder
			touch_left_border = true;
			}
		}
		if(top_lane <= 137 && btm_lane >= 103){
			if(right_lane == 130){
				touch_right_border = true;
			}
			if(left_lane == 149){//left boarder
			touch_left_border = true;
			}
		}
		if(left_lane <= 148 && right_lane >= 129){
			if(btm_lane == 59){//down boarder
			touch_down_border = true;
		}
			if(top_lane == 138){//up boarder
			touch_up_border = true;
		}
		}
		if(right_lane >= 131 && right_lane <= 138){			

			if(btm_lane == 104){//down boarder
			touch_down_border = true;
		}
			if(top_lane == 83){//up boarder
			touch_up_border = true;
		}
		}
		if(top_lane <= 158 && btm_lane >= 61){
			if(right_lane == 167){
				touch_right_border = true;
			}
			if(left_lane == 187){//left boarder
			touch_left_border = true;
			}
		}
		if(left_lane <= 186 && right_lane >= 168){
			if(btm_lane == 60){//down boarder
			touch_down_border = true;
		}
			if(top_lane == 159){//up boarder
			touch_up_border = true;
		}
		}	
		if(top_lane <= 159 && btm_lane >= 61){
			if(right_lane == 205){
				touch_right_border = true;
			}
			if(left_lane == 225){//left boarder
			touch_left_border = true;
			}
		}
		if(left_lane <= 224 && right_lane >= 206){
			
			if(btm_lane == 60){//down boarder
			touch_down_border = true;
		}
			if(top_lane == 160){//up boarder
			touch_up_border = true;
		}
		}
		if(top_lane <= 181 && btm_lane >= 159){
			if(right_lane == 92){
				touch_right_border = true;
			}
			if(left_lane == 113){//left boarder
			touch_left_border = true;
			}
		}
		if(left_lane <= 112 && right_lane >= 93){
			
			if(btm_lane == 158){//down boarder
			touch_down_border = true;
		}
			if(top_lane == 182){//up boarder
			touch_up_border = true;
		}
		}
		if(top_lane <= 181 && btm_lane >= 159){
			if(right_lane == 130){
				touch_right_border = true;
			}
			if(left_lane == 146){//left boarder
			touch_left_border = true;
			}
		}
		if(left_lane <= 145 && right_lane >= 131){
			
			if(btm_lane == 158){//down boarder
			touch_down_border = true;
		}
			if(top_lane == 182){//up boarder
			touch_up_border = true;
		}
		}
		
		if(top_lane <= 203 && btm_lane >= 181){
			if(right_lane == 167){
				touch_right_border = true;
			}
			if(left_lane == 206){//left boarder
			touch_left_border = true;
			}
		}
		if(left_lane <= 205 && right_lane >= 168){
			
			if(btm_lane == 180){//down boarder
			touch_down_border = true;
		}
			if(top_lane == 204){//up boarder
			touch_up_border = true;
		}
		}
		
		if(top_lane <= 203 && btm_lane >= 186){
			if(right_lane == 35){
				touch_right_border = true;
			}
			if(left_lane == 75){//left boarder
			touch_left_border = true;
			}
		}
		if(left_lane <= 74 && right_lane >= 36){
			if(btm_lane == 185){//down boarder
			touch_down_border = true;
		}
			if(top_lane == 204){//up boarder
			touch_up_border = true;
		}
		}
		if(top_lane <= 185 && btm_lane >= 181){
			if(right_lane == 54){
				touch_right_border = true;
			}
			if(left_lane == 75){//left boarder
			touch_left_border = true;
			}
		}
		if(left_lane <= 74 && right_lane >= 55){
			if(btm_lane == 180){//down boarder
			touch_down_border = true;
		}
		}
		if(top_lane <= 302 && btm_lane >= 225){
			if(right_lane == 17){
				touch_right_border = true;
			}
			if(left_lane == 37){//left boarder
			touch_left_border = true;
			}
		}
		if(left_lane <= 36 && right_lane >= 18){
			if(btm_lane == 224){//down boarder
			touch_down_border = true;
		}
			if(top_lane == 303){//up boarder
			touch_up_border = true;
		}
		}
		if(top_lane <= 302 && btm_lane >= 225){
			if(right_lane == 54){
				touch_right_border = true;
			}
			if(left_lane == 75){//left boarder
			touch_left_border = true;
			}
		}
		if(left_lane <= 74 && right_lane >= 55){
			if(btm_lane == 224){//down boarder
			touch_down_border = true;
		}
			if(top_lane == 303){//up boarder
			touch_up_border = true;
		}
		}
		if(top_lane <= 268 && btm_lane >= 225){
			if(right_lane == 91){
				touch_right_border = true;
			}
			if(left_lane == 112){//left boarder
			touch_left_border = true;
			}
		}
		if(left_lane <= 111 && right_lane >= 92){
			if(btm_lane == 224){//down boarder
			touch_down_border = true;
		}
			if(top_lane == 269){//up boarder
			touch_up_border = true;
		}
		}
		if(top_lane <= 268 && btm_lane >= 225){
			if(right_lane == 129){
				touch_right_border = true;
			}
			if(left_lane == 151){//left boarder
			touch_left_border = true;
			}
		}
		if(left_lane <= 150 && right_lane >= 130){
			if(btm_lane == 224){//down boarder
			touch_down_border = true;
		}
			if(top_lane == 269){//up boarder
			touch_up_border = true;
		}
		}
		if(left_lane <= 149 && right_lane >= 93){
			if(btm_lane == 224){//down boarder
			touch_down_border = true;
		}
			if(top_lane == 236){//up boarder
			touch_up_border = true;
		}
		}
		if(top_lane <= 301 && btm_lane >= 225){
			if(right_lane == 167){
				touch_right_border = true;
			}
			if(left_lane == 188){//left boarder
			touch_left_border = true;
			}
		}
		if(left_lane <= 187 && right_lane >= 168){
			if(btm_lane == 224){//down boarder
			touch_down_border = true;
		}
			if(top_lane == 302){//up boarder
			touch_up_border = true;
		}
	}
		if(top_lane <= 301 && btm_lane >= 225){
			if(right_lane == 205){
				touch_right_border = true;
			}
			if(left_lane == 225){//left boarder
			touch_left_border = true;
			}
		}
		if(left_lane <= 224 && right_lane >= 206){
			if(btm_lane == 224){//down boarder
			touch_down_border = true;
		}
			if(top_lane == 302){//up boarder
			touch_up_border = true;
		}
	}
		
}
void setup_monster_boundires(struct monster* curr_mon){
		int left_lane = curr_mon->x_loc-MONSTER_WIDTH/2;
	int right_lane = curr_mon->x_loc+MONSTER_WIDTH/2;
	int top_lane = curr_mon->y_loc-MONSTER_HEIGHT/2;
	int btm_lane = curr_mon->y_loc+MONSTER_HEIGHT/2;
		if((right_lane == COLS-1)){//right boarder
			touch_right = true;
		}else{
			touch_right = false;
		}
		if((left_lane == 0)){//left boarder
			touch_left= true;
		}else{
			touch_left = false;
		}
		if((btm_lane == ROWS-1)){//down boarder
			touch_down = true;
		}else{
			touch_down = false;
		}
		if((top_lane == 41)){//up boarder
			touch_up = true;
		}else{
			touch_up = false;
		}
		
		if(top_lane <= 159 && btm_lane >= 60){
			if(right_lane == 17){
				touch_right = true;
			}
			if(left_lane == 38){//left boarder
			touch_left = true;
			}
		}
		if(left_lane <= 38 && right_lane >= 17){
			if(btm_lane == 60){//down boarder
			touch_down = true;
		}
			if(top_lane == 160){//up boarder
			touch_up = true;
		}
		}
		if(top_lane <= 158 && btm_lane >= 60){
			if(right_lane == 54){
				touch_right = true;
			}
			if(left_lane == 75){//left boarder
			touch_left = true;
			}
		}
		if(left_lane <= 75 && right_lane >= 54){
			if(btm_lane == 60){//down boarder
			touch_down = true;
		}
			if(top_lane == 158){//up boarder
			touch_up = true;
		}
		}
		if(top_lane <= 82 && btm_lane >= 60){
			if(right_lane == 92){
				touch_right = true;
			}
			if(left_lane == 113){//left boarder
			touch_left = true;
			}
		}
		if(left_lane <= 113 && right_lane >= 92){
			if(btm_lane == 60){//down boarder
			touch_down = true;
		}
			if(top_lane == 82){//up boarder
			touch_up = true;
		}
		}
		if(top_lane <= 137 && btm_lane >= 104){
			if(right_lane == 92){
				touch_right = true;
			}
			if(left_lane == 113){//left boarder
			touch_left = true;
			}
		}
		if(left_lane <= 113 && right_lane >= 92){
			
			if(btm_lane == 104){//down boarder
			touch_down = true;
		}
			if(top_lane == 137){//up boarder
			touch_up = true;
		}
		}
		if(top_lane <= 82 && btm_lane >= 60){
			if(right_lane == 130){
				touch_right = true;
			}
			if(left_lane == 149){//left boarder
			touch_left = true;
			}
		}
		if(top_lane <= 102 && btm_lane >= 83){
			if(right_lane == 138){
				touch_right = true;
			}
			if(left_lane == 149){//left boarder
			touch_left = true;
			}
		}
		if(top_lane <= 137 && btm_lane >= 103){
			if(right_lane == 130){
				touch_right = true;
			}
			if(left_lane == 149){//left boarder
			touch_left = true;
			}
		}
		if(left_lane <= 148 && right_lane >= 129){
			if(btm_lane == 59){//down boarder
			touch_down = true;
		}
			if(top_lane == 138){//up boarder
			touch_up = true;
		}
		}
		if(right_lane >= 131 && right_lane <= 138){			

			if(btm_lane == 104){//down boarder
			touch_down = true;
		}
			if(top_lane == 83){//up boarder
			touch_up = true;
		}
		}
		if(top_lane <= 158 && btm_lane >= 61){
			if(right_lane == 167){
				touch_right = true;
			}
			if(left_lane == 187){//left boarder
			touch_left = true;
			}
		}
		if(left_lane <= 186 && right_lane >= 168){
			if(btm_lane == 60){//down boarder
			touch_down = true;
		}
			if(top_lane == 159){//up boarder
			touch_up = true;
		}
		}	
		if(top_lane <= 159 && btm_lane >= 61){
			if(right_lane == 205){
				touch_right = true;
			}
			if(left_lane == 225){//left boarder
			touch_left = true;
			}
		}
		if(left_lane <= 224 && right_lane >= 206){
			
			if(btm_lane == 60){//down boarder
			touch_down = true;
		}
			if(top_lane == 160){//up boarder
			touch_up = true;
		}
		}
		if(top_lane <= 181 && btm_lane >= 159){
			if(right_lane == 92){
				touch_right = true;
			}
			if(left_lane == 113){//left boarder
			touch_left = true;
			}
		}
		if(left_lane <= 112 && right_lane >= 93){
			
			if(btm_lane == 158){//down boarder
			touch_down = true;
		}
			if(top_lane == 182){//up boarder
			touch_up = true;
		}
		}
		if(top_lane <= 181 && btm_lane >= 159){
			if(right_lane == 130){
				touch_right = true;
			}
			if(left_lane == 146){//left boarder
			touch_left = true;
			}
		}
		if(left_lane <= 145 && right_lane >= 131){
			
			if(btm_lane == 158){//down boarder
			touch_down = true;
		}
			if(top_lane == 182){//up boarder
			touch_up = true;
		}
		}
		
		if(top_lane <= 203 && btm_lane >= 181){
			if(right_lane == 167){
				touch_right = true;
			}
			if(left_lane == 206){//left boarder
			touch_left = true;
			}
		}
		if(left_lane <= 205 && right_lane >= 168){
			
			if(btm_lane == 180){//down boarder
			touch_down = true;
		}
			if(top_lane == 204){//up boarder
			touch_up = true;
		}
		}
		
		if(top_lane <= 203 && btm_lane >= 186){
			if(right_lane == 35){
				touch_right = true;
			}
			if(left_lane == 75){//left boarder
			touch_left = true;
			}
		}
		if(left_lane <= 74 && right_lane >= 36){
			if(btm_lane == 185){//down boarder
			touch_down = true;
		}
			if(top_lane == 204){//up boarder
			touch_up = true;
		}
		}
		if(top_lane <= 185 && btm_lane >= 181){
			if(right_lane == 54){
				touch_right = true;
			}
			if(left_lane == 75){//left boarder
			touch_left = true;
			}
		}
		if(left_lane <= 74 && right_lane >= 55){
			if(btm_lane == 180){//down boarder
			touch_down = true;
		}
		}
		if(top_lane <= 302 && btm_lane >= 225){
			if(right_lane == 17){
				touch_right = true;
			}
			if(left_lane == 37){//left boarder
			touch_left = true;
			}
		}
		if(left_lane <= 36 && right_lane >= 18){
			if(btm_lane == 224){//down boarder
			touch_down = true;
		}
			if(top_lane == 303){//up boarder
			touch_up = true;
		}
		}
		if(top_lane <= 302 && btm_lane >= 225){
			if(right_lane == 54){
				touch_right = true;
			}
			if(left_lane == 75){//left boarder
			touch_left = true;
			}
		}
		if(left_lane <= 74 && right_lane >= 55){
			if(btm_lane == 224){//down boarder
			touch_down = true;
		}
			if(top_lane == 303){//up boarder
			touch_up = true;
		}
		}
		if(top_lane <= 268 && btm_lane >= 225){
			if(right_lane == 91){
				touch_right = true;
			}
			if(left_lane == 112){//left boarder
			touch_left = true;
			}
		}
		if(left_lane <= 111 && right_lane >= 92){
			if(btm_lane == 224){//down boarder
			touch_down = true;
		}
			if(top_lane == 269){//up boarder
			touch_up = true;
		}
		}
		if(top_lane <= 268 && btm_lane >= 225){
			if(right_lane == 129){
				touch_right = true;
			}
			if(left_lane == 151){//left boarder
			touch_left = true;
			}
		}
		if(left_lane <= 150 && right_lane >= 130){
			if(btm_lane == 224){//down boarder
			touch_down = true;
		}
			if(top_lane == 269){//up boarder
			touch_up = true;
		}
		}
		if(left_lane <= 149 && right_lane >= 93){
			if(btm_lane == 224){//down boarder
			touch_down = true;
		}
			if(top_lane == 236){//up boarder
			touch_up = true;
		}
		}
		if(top_lane <= 301 && btm_lane >= 225){
			if(right_lane == 167){
				touch_right = true;
			}
			if(left_lane == 188){//left boarder
			touch_left = true;
			}
		}
		if(left_lane <= 187 && right_lane >= 168){
			if(btm_lane == 224){//down boarder
			touch_down = true;
		}
			if(top_lane == 302){//up boarder
			touch_up = true;
		}
	}
		if(top_lane <= 301 && btm_lane >= 225){
			if(right_lane == 205){
				touch_right = true;
			}
			if(left_lane == 225){//left boarder
			touch_left = true;
			}
		}
		if(left_lane <= 224 && right_lane >= 206){
			if(btm_lane == 224){//down boarder
			touch_down = true;
		}
			if(top_lane == 302){//up boarder
			touch_up = true;
		}
	}
}
bool setup_bullet_boundries(){
	int left_lane = m_curr->x_loc-BULLET_WIDTH/2;
	int right_lane = m_curr->x_loc+BULLET_WIDTH/2;
	int top_lane = m_curr->y_loc - BULLET_HEIGHT/2;
	int btm_lane = m_curr->y_loc + BULLET_HEIGHT/2;
		if((right_lane == COLS-1)){//right boarder
			remove_missle(m_curr);
			return false;
		}
		if((left_lane == 0)){//left boarder
			remove_missle(m_curr);
			return false;
		}
		if((btm_lane == ROWS-1)){//down boarder
			remove_missle(m_curr);
			return false;
		}
		if((top_lane == 40)){//up boarder
			remove_missle(m_curr);
			return false;
		}
		
		if(top_lane <= 159 && btm_lane >= 60){
			if(right_lane == 17){
				remove_missle(m_curr);
				return false;
			}
			if(left_lane == 38){//left boarder
			remove_missle(m_curr);
				return false;
			}
		}
		if(left_lane <= 38 && right_lane >= 17){
			if(btm_lane == 60){//down boarder
			remove_missle(m_curr);
				return false;
		}
			if(top_lane == 160){//up boarder
			remove_missle(m_curr);
				return false;
		}
		}
		if(top_lane <= 158 && btm_lane >= 60){
			if(right_lane == 54){
				remove_missle(m_curr);
				return false;
			}
			if(left_lane == 75){//left boarder
			remove_missle(m_curr);
				return false;
			}
		}
		if(left_lane <= 75 && right_lane >= 54){
			if(btm_lane == 60){//down boarder
			remove_missle(m_curr);
				return false;
		}
			if(top_lane == 158){//up boarder
			remove_missle(m_curr);
				return false;
		}
		}
		if(top_lane <= 82 && btm_lane >= 60){
			if(right_lane == 92){
				remove_missle(m_curr);
				return false;
			}
			if(left_lane == 113){//left boarder
			remove_missle(m_curr);
				return false;
			}
		}
		if(left_lane <= 113 && right_lane >= 92){
			if(btm_lane == 60){//down boarder
			remove_missle(m_curr);
				return false;
		}
			if(top_lane == 82){//up boarder
			remove_missle(m_curr);
				return false;
		}
		}
		if(top_lane <= 137 && btm_lane >= 104){
			if(right_lane == 92){
				remove_missle(m_curr);
				return false;
			}
			if(left_lane == 113){//left boarder
			remove_missle(m_curr);
				return false;
			}
		}
		if(left_lane <= 113 && right_lane >= 92){
			
			if(btm_lane == 104){//down boarder
			remove_missle(m_curr);
				return false;
		}
			if(top_lane == 137){//up boarder
			remove_missle(m_curr);
				return false;
		}
		}
		if(top_lane <= 82 && btm_lane >= 60){
			if(right_lane == 130){
				remove_missle(m_curr);
				return false;
			}
			if(left_lane == 149){//left boarder
			remove_missle(m_curr);
				return false;
			}
		}
		if(top_lane <= 102 && btm_lane >= 83){
			if(right_lane == 138){
				remove_missle(m_curr);
				return false;
			}
			if(left_lane == 149){//left boarder
			remove_missle(m_curr);
				return false;
			}
		}
		if(top_lane <= 137 && btm_lane >= 103){
			if(right_lane == 130){
				remove_missle(m_curr);
				return false;
			}
			if(left_lane == 149){//left boarder
			remove_missle(m_curr);
				return false;
			}
		}
		if(left_lane <= 148 && right_lane >= 129){
			if(btm_lane == 59){//down boarder
			remove_missle(m_curr);
				return false;
		}
			if(top_lane == 138){//up boarder
			remove_missle(m_curr);
				return false;
		}
		}
		if(right_lane >= 131 && right_lane <= 138){			

			if(btm_lane == 104){//down boarder
			remove_missle(m_curr);
				return false;
		}
			if(top_lane == 83){//up boarder
			remove_missle(m_curr);
				return false;
		}
		}
		if(top_lane <= 158 && btm_lane >= 61){
			if(right_lane == 167){
				remove_missle(m_curr);
				return false;
			}
			if(left_lane == 187){//left boarder
			remove_missle(m_curr);
				return false;
			}
		}
		if(left_lane <= 186 && right_lane >= 168){
			if(btm_lane == 60){//down boarder
			remove_missle(m_curr);
				return false;
		}
			if(top_lane == 159){//up boarder
			remove_missle(m_curr);
				return false;
		}
		}	
		if(top_lane <= 159 && btm_lane >= 61){
			if(right_lane == 205){
				remove_missle(m_curr);
				return false;
			}
			if(left_lane == 225){//left boarder
			remove_missle(m_curr);
				return false;
			}
		}
		if(left_lane <= 224 && right_lane >= 206){
			
			if(btm_lane == 60){//down boarder
			remove_missle(m_curr);
				return false;
		}
			if(top_lane == 160){//up boarder
			remove_missle(m_curr);
				return false;
		}
		}
		if(top_lane <= 181 && btm_lane >= 159){
			if(right_lane == 92){
				remove_missle(m_curr);
				return false;
			}
			if(left_lane == 113){//left boarder
			remove_missle(m_curr);
				return false;
			}
		}
		if(left_lane <= 112 && right_lane >= 93){
			
			if(btm_lane == 158){//down boarder
			remove_missle(m_curr);
				return false;
		}
			if(top_lane == 182){//up boarder
			remove_missle(m_curr);
				return false;
		}
		}
		if(top_lane <= 181 && btm_lane >= 159){
			if(right_lane == 130){
				remove_missle(m_curr);
				return false;
			}
			if(left_lane == 146){//left boarder
			remove_missle(m_curr);
				return false;
			}
		}
		if(left_lane <= 145 && right_lane >= 131){
			
			if(btm_lane == 158){//down boarder
			remove_missle(m_curr);
				return false;
		}
			if(top_lane == 182){//up boarder
			remove_missle(m_curr);
				return false;
		}
		}
		
		if(top_lane <= 203 && btm_lane >= 181){
			if(right_lane == 167){
				remove_missle(m_curr);
				return false;
			}
			if(left_lane == 206){//left boarder
			remove_missle(m_curr);
				return false;
			}
		}
		if(left_lane <= 205 && right_lane >= 168){
			
			if(btm_lane == 180){//down boarder
			remove_missle(m_curr);
				return false;
		}
			if(top_lane == 204){//up boarder
			remove_missle(m_curr);
				return false;
		}
		}
		
		if(top_lane <= 203 && btm_lane >= 186){
			if(right_lane == 35){
				remove_missle(m_curr);
				return false;
			}
			if(left_lane == 75){//left boarder
			remove_missle(m_curr);
				return false;
			}
		}
		if(left_lane <= 74 && right_lane >= 36){
			if(btm_lane == 185){//down boarder
			remove_missle(m_curr);
				return false;
		}
			if(top_lane == 204){//up boarder
			remove_missle(m_curr);
				return false;
		}
		}
		if(top_lane <= 185 && btm_lane >= 181){
			if(right_lane == 54){
				remove_missle(m_curr);
				return false;
			}
			if(left_lane == 75){//left boarder
			remove_missle(m_curr);
				return false;
			}
		}
		if(left_lane <= 74 && right_lane >= 55){
			if(btm_lane == 180){//down boarder
			remove_missle(m_curr);
				return false;
		}
		}
		if(top_lane <= 302 && btm_lane >= 225){
			if(right_lane == 17){
				remove_missle(m_curr);
				return false;
			}
			if(left_lane == 37){//left boarder
			remove_missle(m_curr);
				return false;
			}
		}
		if(left_lane <= 36 && right_lane >= 18){
			if(btm_lane == 224){//down boarder
			remove_missle(m_curr);
				return false;
		}
			if(top_lane == 303){//up boarder
			remove_missle(m_curr);
				return false;
		}
		}
		if(top_lane <= 302 && btm_lane >= 225){
			if(right_lane == 54){
				remove_missle(m_curr);
				return false;
			}
			if(left_lane == 75){//left boarder
			remove_missle(m_curr);
				return false;
			}
		}
		if(left_lane <= 74 && right_lane >= 55){
			if(btm_lane == 224){//down boarder
			remove_missle(m_curr);
				return false;
		}
			if(top_lane == 303){//up boarder
			remove_missle(m_curr);
				return false;
		}
		}
		if(top_lane <= 268 && btm_lane >= 225){
			if(right_lane == 91){
				remove_missle(m_curr);
				return false;
			}
			if(left_lane == 112){//left boarder
			remove_missle(m_curr);
				return false;
			}
		}
		if(left_lane <= 111 && right_lane >= 92){
			if(btm_lane == 224){//down boarder
			remove_missle(m_curr);
				return false;
		}
			if(top_lane == 269){//up boarder
			remove_missle(m_curr);
				return false;
		}
		}
		if(top_lane <= 268 && btm_lane >= 225){
			if(right_lane == 129){
				remove_missle(m_curr);
				return false;
			}
			if(left_lane == 151){//left boarder
			remove_missle(m_curr);
				return false;
			}
		}
		if(left_lane <= 150 && right_lane >= 130){
			if(btm_lane == 224){//down boarder
			remove_missle(m_curr);
				return false;
		}
			if(top_lane == 269){//up boarder
			remove_missle(m_curr);
				return false;
		}
		}
		if(left_lane <= 149 && right_lane >= 93){
			if(btm_lane == 224){//down boarder
			remove_missle(m_curr);
				return false;
		}
			if(top_lane == 236){//up boarder
			remove_missle(m_curr);
				return false;
		}
		}
		if(top_lane <= 301 && btm_lane >= 225){
			if(right_lane == 167){
				remove_missle(m_curr);
				return false;
			}
			if(left_lane == 188){//left boarder
			remove_missle(m_curr);
				return false;
			}
		}
		if(left_lane <= 187 && right_lane >= 168){
			if(btm_lane == 224){//down boarder
			remove_missle(m_curr);
				return false;
		}
			if(top_lane == 302){//up boarder
			remove_missle(m_curr);
				return false;
		}
	}
		if(top_lane <= 301 && btm_lane >= 225){
			if(right_lane == 205){
				remove_missle(m_curr);
				return false;
			}
			if(left_lane == 225){//left boarder
			remove_missle(m_curr);
				return false;
			}
		}
		if(left_lane <= 224 && right_lane >= 206){
			if(btm_lane == 224){//down boarder
			remove_missle(m_curr);
				return false;
		}
			if(top_lane == 302){//up boarder
			remove_missle(m_curr);
				return false;
		}
	}
		return true;
}
void add_bomb(){
	if(bomb_count == 2){
	first_bomb.x_loc = tank.x_loc;
	first_bomb.y_loc = tank.y_loc;
	lcd_draw_image(
							COLS/2 + 50 + 25,
							BOMB_WIDTH,
							30,
							BOMB_HEIGHT,
							bomb,
							LCD_COLOR_BLACK,
							LCD_COLOR_BLACK);
		return;
	}
	if(bomb_count == 1){
	second_bomb.x_loc = tank.x_loc;
	second_bomb.y_loc = tank.y_loc;
			lcd_draw_image(
							COLS/2 + 50,
							BOMB_WIDTH,
							30,
							BOMB_HEIGHT,
							bomb,
							LCD_COLOR_BLACK,
							LCD_COLOR_BLACK);
		return;
	}
	return;
}
void add_monster(int x_location){
	struct monster* new_monster;
	new_monster = (struct monster *)malloc(sizeof(struct monster));
	new_monster->x_loc = x_location;
	new_monster->y_loc = 41 + MONSTER_HEIGHT/2;
	new_monster->dir = 0;
	new_monster->prv_success = false;
	new_monster->num_steps = 0;
		//set up head and tail
	if(monster_head == NULL){
		monster_head = new_monster;
		monster_tail = new_monster;
		monster_head->nxt = NULL;
		monster_head->prv = NULL;
		return;
	}
	//link with the previous tail
	new_monster->prv = monster_tail;//if new missle is the tail, the prv of tail is itself
	new_monster->nxt = NULL;
	monster_tail->nxt = new_monster;
	//new tail
	monster_tail = new_monster;	
}
bool remove_monster(struct monster* del_monster){
		if(del_monster == NULL){
		return false;
	}
	//printf("next missle location: %d and %d\n",del_missle->nxt->x_loc,del_missle->nxt->y_loc);
	if(del_monster == monster_head){
		monster_head = del_monster->nxt;
		free(del_monster);
	}
	(del_monster->nxt)->prv = del_monster->prv;
	(del_monster->prv)->nxt = del_monster->nxt;
	monster_curr = del_monster->nxt;
	free(del_monster);
	del_monster = NULL;
	return true;
}
bool check_collision(struct monster* curr_mon,struct missle* curr_missle){
	int curr_bullet_x;
	int curr_bullet_y;
	int left_lane_mon = curr_mon->x_loc-MONSTER_WIDTH/2;
	int right_lane_mon = curr_mon->x_loc+MONSTER_WIDTH/2;
	int top_lane_mon = curr_mon->y_loc-MONSTER_HEIGHT/2;
	int btm_lane_mon = curr_mon->y_loc+MONSTER_HEIGHT/2;
	int left_lane_bullet = curr_missle->x_loc-BULLET_WIDTH/2;
	int right_lane_bullet = curr_missle->x_loc+BULLET_WIDTH/2;
	int top_lane_bullet = curr_missle->y_loc - BULLET_HEIGHT/2;
	int btm_lane_bullet = curr_missle->y_loc + BULLET_HEIGHT/2;
	uint16_t curr_tank_x;
	uint16_t curr_tank_y;
	curr_tank_x = tank.x_loc;
	curr_tank_y = tank.y_loc;
	//monster collides with tank
	if(top_lane_mon <= tank.y_loc &&btm_lane_mon >=tank.y_loc &&left_lane_mon <= tank.x_loc && right_lane_mon >= tank.x_loc){
		life_count--;
			collision_happened = true;
			COLLISION_HARM = false;
			able_to_fire = false;
	}
	//bullet collide with monster
	curr_bullet_x = curr_missle->x_loc;
	curr_bullet_y = curr_missle->y_loc;

		if(top_lane_bullet < btm_lane_mon && btm_lane_bullet > top_lane_mon && left_lane_bullet < right_lane_mon && right_lane_bullet > left_lane_mon){
			remove_monster(curr_mon);
			remove_missle(curr_missle);
					lcd_draw_image(
					curr_bullet_x,
					BULLET_WIDTH,
					curr_bullet_y,
					BULLET_HEIGHT,
					bulletErase,
					LCD_COLOR_WHITE,
					LCD_COLOR_WHITE);
			monster_count--;

			return true;
		}
	return false;
	
}
bool check_bomb_kills(struct monster* curr_mon,bomb_t curr_bomb){
	int curr_bomb_x = curr_bomb.x_loc;
	int curr_bomb_y = curr_bomb.y_loc;
	int left_lane = curr_bomb_x - 40;
	int right_lane = curr_bomb_x + 40;
	int top_lane = curr_bomb_y - 40;
	int btm_lane = curr_bomb_y + 40;
	if(top_lane <= curr_mon->y_loc &&btm_lane >=curr_mon->y_loc &&left_lane <= curr_mon->x_loc && right_lane >= curr_mon->x_loc){
				lcd_draw_image(
							curr_mon->x_loc,
							MONSTER_WIDTH,
							curr_mon->y_loc,
							MONSTER_HEIGHT,
							monster,
							LCD_COLOR_WHITE,
							LCD_COLOR_WHITE);
		remove_monster(curr_mon);
		return true;
	}
	return false;
}
void result_state(){
	if(my_kills > other_kills ){
		if(TONE_UPDATE && win){
		TONE_UPDATE = false;
		if(win){
			temp_frequency_result = temp_frequency_result + 50;
		}
		enable_pwm(temp_frequency_result);
		if(temp_frequency_result >= 800){
			win = false;
		}
	}
		if(TONE_UPDATE && !win){
		TONE_UPDATE = false;
		if(UP_SCROLL){
			disable_pwm();
			UP_SCROLL = !UP_SCROLL;
		}else{
		  enable_pwm(temp_frequency_result);
			UP_SCROLL = !UP_SCROLL;
		}
	}
						lcd_draw_image(
									COLS/2,
									LOST_WIDTH,
									ROWS/2,	
									LOST_HEIGHT,
									winGraph,
									LCD_COLOR_BLACK,
									LCD_COLOR_WHITE);
	}else{
		if(TONE_UPDATE && lose){
		TONE_UPDATE = false;
		if(lose){
			temp_frequency_result = temp_frequency_result - 50;
		}
		//printf("the freq is: %d\n", temp_frequency_result);
		enable_pwm(temp_frequency_result);
		if(temp_frequency_result <= 200){
			lose = false;
		}
	}
		if(TONE_UPDATE && !lose){
		TONE_UPDATE = false;
		if(UP_SCROLL){
			disable_pwm();
			UP_SCROLL = !UP_SCROLL;
		}else{
		  enable_pwm(temp_frequency_result);
			UP_SCROLL = !UP_SCROLL;
		}
	}
								lcd_draw_image(
									COLS/2,
									LOST_WIDTH,
									ROWS/2,	
									LOST_HEIGHT,
									lostGraph,
									LCD_COLOR_BLACK,
									LCD_COLOR_WHITE);
	}
}
void playing_state(){
	int location = 0;
	int min = MONSTER_WIDTH/2;
	int max = 240-min;
	int temp;
	uint32_t my_send;
	uint32_t other_send;
	uint8_t other_dir;
	bool unmodified = true;
	bool harmed = false;
	bool killed_by_bomb = false;

	char you_killed[10];
	char opponent_killed[10];
	sprintf(you_killed, "You: %d", my_kills);
	sprintf(opponent_killed, "Opp: %d", other_kills);
	printf("i killed: %d\n",my_kills);
	printf("other killed: %d\n",other_kills);
	lcd_print_stringXY(you_killed,4,0,LCD_COLOR_WHITE,LCD_COLOR_BLACK);
	lcd_print_stringXY(opponent_killed,12,0,LCD_COLOR_WHITE,LCD_COLOR_BLACK);
	setup_boundries();
	
	if(first_drawn){
		lcd_draw_image(
							COLS/2,
							TITLE_WIDTH,
							20,
							TITLE_HEIGHT,
							title,
							LCD_COLOR_BLACK,
							LCD_COLOR_WHITE);
		lcd_draw_image(
							COLS/2 - 50,
							HEART_WIDTH,
							30,
							HEART_HEIGHT,
							hearts,
							LCD_COLOR_WHITE,
							LCD_COLOR_BLACK);
				lcd_draw_image(
							COLS/2 - 27,
							HEART_WIDTH,
							30,
							HEART_HEIGHT,
							hearts,
							LCD_COLOR_WHITE,
							LCD_COLOR_BLACK);
		lcd_draw_image(
							COLS/2 -4,
							HEART_WIDTH,
							30,
							HEART_HEIGHT,
							hearts,
							LCD_COLOR_WHITE,
							LCD_COLOR_BLACK);
		lcd_draw_image(
							COLS/2 + 50,
							BOMB_WIDTH,
							30,
							BOMB_HEIGHT,
							bomb,
							LCD_COLOR_WHITE,
							LCD_COLOR_BLACK);
		lcd_draw_image(
							COLS/2 + 50 + 25,
							BOMB_WIDTH,
							30,
							BOMB_HEIGHT,
							bomb,
							LCD_COLOR_WHITE,
							LCD_COLOR_BLACK);
		lcd_draw_image(
							COLS/2,
							MAP_WIDTH,
							ROWS/2 + 20,
							MAP_HEIGHT,
							mapBitmaps,
							LCD_COLOR_WHITE,
							LCD_COLOR_BLACK);
		lcd_draw_image(
							tank.x_loc,
							TANK_WIDTH,
							tank.y_loc,
							TANK_HEIGHT,
							tank_up,
							LCD_COLOR_BLACK,
							LCD_COLOR_WHITE);
		first_drawn = false;
		head_up = true;
		head_down = false;
		head_left = false;
		head_right = false;
	}
	if(joystick_left_right == IDLE_lr || joystick_up_down == IDLE_ud){
		if(ALERT_MISSLE_UPDATE){
			if(!COLLISION_HARM){
				if(head_up){
				lcd_draw_image(
							tank.x_loc,
							TANK_WIDTH,
							tank.y_loc,
							TANK_HEIGHT,
							tank_up,
							LCD_COLOR_GRAY,
							LCD_COLOR_WHITE);
			}
			if(head_down){
				lcd_draw_image(
							tank.x_loc,
							TANK_WIDTH,
							tank.y_loc,
							TANK_HEIGHT,
							tank_down,
							LCD_COLOR_GRAY,
							LCD_COLOR_WHITE);
			}
			if(head_left){
				lcd_draw_image(
							tank.x_loc,
							TANK_WIDTH,
							tank.y_loc,
							TANK_HEIGHT,
							tank_left,
							LCD_COLOR_GRAY,
							LCD_COLOR_WHITE);
			}
			if(head_right){
				lcd_draw_image(
							tank.x_loc,
							TANK_WIDTH,
							tank.y_loc,
							TANK_HEIGHT,
							tank_right,
							LCD_COLOR_GRAY,
							LCD_COLOR_WHITE);
			}
			}else{
				if(head_up){
				lcd_draw_image(
							tank.x_loc,
							TANK_WIDTH,
							tank.y_loc,
							TANK_HEIGHT,
							tank_up,
							LCD_COLOR_BLACK,
							LCD_COLOR_WHITE);
			}
			if(head_down){
				lcd_draw_image(
							tank.x_loc,
							TANK_WIDTH,
							tank.y_loc,
							TANK_HEIGHT,
							tank_down,
							LCD_COLOR_BLACK,
							LCD_COLOR_WHITE);
			}
			if(head_left){
				lcd_draw_image(
							tank.x_loc,
							TANK_WIDTH,
							tank.y_loc,
							TANK_HEIGHT,
							tank_left,
							LCD_COLOR_BLACK,
							LCD_COLOR_WHITE);
			}
			if(head_right){
				lcd_draw_image(
							tank.x_loc,
							TANK_WIDTH,
							tank.y_loc,
							TANK_HEIGHT,
							tank_right,
							LCD_COLOR_BLACK,
							LCD_COLOR_WHITE);
			}
			}
			
		}
	}
	
			if(joystick_left_right != IDLE_lr || joystick_up_down != IDLE_ud){//we need to  redraw
			if(joystick_left_right == RGHT && !touch_right_border && joystick_int){
				tank.x_loc = tank.x_loc +1;
				if(!COLLISION_HARM){
					lcd_draw_image(
							tank.x_loc,
							TANK_WIDTH,
							tank.y_loc,
							TANK_HEIGHT,
							tank_right,
							LCD_COLOR_GRAY,
							LCD_COLOR_WHITE);
				}else{
					lcd_draw_image(
							tank.x_loc,
							TANK_WIDTH,
							tank.y_loc,
							TANK_HEIGHT,
							tank_right,
							LCD_COLOR_BLACK,
							LCD_COLOR_WHITE);
				}
					
				head_up = false;
				head_down = false;
				head_left = false;
				head_right = true;
			}
			if(joystick_left_right == LFT && !touch_left_border && joystick_int){
				tank.x_loc = tank.x_loc -1;
				if(!COLLISION_HARM){
					lcd_draw_image(
							tank.x_loc,
							TANK_WIDTH,
							tank.y_loc,
							TANK_HEIGHT,
							tank_left,
							LCD_COLOR_GRAY,
							LCD_COLOR_WHITE);
				}else{
					lcd_draw_image(
							tank.x_loc,
							TANK_WIDTH,
							tank.y_loc,
							TANK_HEIGHT,
							tank_left,
							LCD_COLOR_BLACK,
							LCD_COLOR_WHITE);
				}
				head_up = false;
				head_down = false;
				head_left = true;
				head_right = false;
				
			}
			if(joystick_up_down == UP && !touch_up_border && joystick_int){
				tank.y_loc = tank.y_loc -1;
				if(!COLLISION_HARM){
					lcd_draw_image(
							tank.x_loc,
							TANK_WIDTH,
							tank.y_loc,
							TANK_HEIGHT,
							tank_up,
							LCD_COLOR_GRAY,
							LCD_COLOR_WHITE);
				}else{
					lcd_draw_image(
							tank.x_loc,
							TANK_WIDTH,
							tank.y_loc,
							TANK_HEIGHT,
							tank_up,
							LCD_COLOR_BLACK,
							LCD_COLOR_WHITE);
				}
				head_up = true;
				head_down = false;
				head_left = false;
				head_right = false;
			}
			if(joystick_up_down == DOWN && !touch_down_border && joystick_int){
				tank.y_loc = tank.y_loc +1;
				if(!COLLISION_HARM){
					lcd_draw_image(
							tank.x_loc,
							TANK_WIDTH,
							tank.y_loc,
							TANK_HEIGHT,
							tank_down,
							LCD_COLOR_GRAY,
							LCD_COLOR_WHITE);
				}else{
					lcd_draw_image(
							tank.x_loc,
							TANK_WIDTH,
							tank.y_loc,
							TANK_HEIGHT,
							tank_down,
							LCD_COLOR_BLACK,
							LCD_COLOR_WHITE);
				}
				head_up = false;
				head_down = true;
				head_left = false;
				head_right = false;
			}
			joystick_int = false;
		}
		//printf("alert_down is: %d and already_launched is: %d\n", alert_down, already_launched);
		if(alert_down && able_to_fire){
			alert_down = false;
			add_missle();
			enable_pwm(100);
			if(TONE_UPDATE){
				TONE_UPDATE = false;
			}
		}
			if(TONE_UPDATE){
				TONE_UPDATE = false;
				disable_pwm();
			}
		//printf("generate monster boolean is: %d\n",GENERATE_MONSTER );
		if(GENERATE_MONSTER && monster_count <= 14){//the max number of monsters is 15
			GENERATE_MONSTER = false;
			monster_count++;
			location = min + rand() % (max - min);
			//printf("My random location is: %d\n", location);
			add_monster(location);
		}
		if(alert_up){
			if(bomb_count != 0){
			alert_up = false;
			add_bomb();
			bomb_count--;
			}
		}
		
		
		if (ALERT_SCORE_UPDATE) {
			ALERT_SCORE_UPDATE = false;

			if (parity == 0) {
				if (MASTER) { // master send, slave rcv
					if (life_count <= 0) {
						my_send = 0xFFFF;
					} else {
						my_send = my_kills;
					}
					printf("Im sending : %d\n", my_send);
					wireless_send_32(false, false, my_send);
				} else {  // // master rcv, slave send
					wireless_get_32(false, &other_send);
					if (other_send < 0xFFFF) {
						other_kills = other_send;
					}
					printf("Im reading : %d\n", other_send);
				}
				parity = 1;
			}

			if (parity == 1) {
				parity = 0;
				if (MASTER) {
					wireless_get_32(false, &other_send);
					printf("Im reading : %d\n", other_send);
					if (other_send < 0xFFFF) {
						other_kills = other_send;
					}
				} else {  // slave
					if (life_count <= 0) {
						my_send = 0xFFFF;
					} else {
						my_send = my_kills;
					}
					printf("Im sending : %d\n", my_send);
					wireless_send_32(false, false, my_send);
				}
			}
			if(my_send == 0xFFFF && other_send == 0xFFFF){
				state = END;
				return;
			}
		}

		if(ALERT_MISSLE_UPDATE){
			int min = 0;
			int max = 4;
			int direction = 0;
		ALERT_MISSLE_UPDATE = false;
		m_curr = m_head;
		while(m_curr != NULL){
			//erase the curr
			lcd_draw_image(
					m_curr->x_loc,
					BULLET_WIDTH,
					m_curr->y_loc,
					BULLET_HEIGHT,
					bulletErase,
					LCD_COLOR_WHITE,
					LCD_COLOR_WHITE);
			if(m_curr->dir == 0){
				m_curr->y_loc--;
			}
			if(m_curr->dir == 1){
				m_curr->y_loc++;
			}
			if(m_curr->dir == 2){
				m_curr->x_loc--;
			}
			if(m_curr->dir == 3){
				m_curr->x_loc++;
			}
			unmodified = setup_bullet_boundries();
			if(!unmodified){
				continue;
			}
			lcd_draw_image(
					m_curr->x_loc,
					BULLET_WIDTH,
					m_curr->y_loc,
					BULLET_HEIGHT,
					bullet,
					LCD_COLOR_BROWN,
					LCD_COLOR_WHITE);
			
			m_curr = m_curr->nxt;
		}
	}
	
		if(ALERT_MONSTER_UPDATE){
			int curr_monster_x;
			int curr_monster_y;
			int min = 0;
			int max = 4;
			int direction = 0;
		ALERT_MONSTER_UPDATE = false;
		monster_curr = monster_head;
		while(monster_curr != NULL){
			//printf("monster_curr x: %d y: %d\n",monster_curr->x_loc,monster_curr->y_loc);
			//erase the curr
			lcd_draw_image(
					monster_curr->x_loc,
					MONSTER_WIDTH,
					monster_curr->y_loc,
					MONSTER_HEIGHT,
					monster,
					LCD_COLOR_WHITE,
					LCD_COLOR_WHITE);
			if(monster_curr->prv_success && monster_curr->num_steps <= steps){
				//printf("my steps is: %d\n", steps);
				direction = monster_curr->dir;
			}else{
				monster_curr->num_steps = 0;
				steps++;
				if(steps == 70){
					steps = 20;
				}
				direction = min + rand() % (max - min);
			}
			touch_up = false;
			touch_down = false;
			touch_left = false;
			touch_right = false;
			setup_monster_boundires(monster_curr);
			if(direction == 0 && !touch_up){//up
				monster_curr->y_loc--;
				monster_curr->dir = 0;
				monster_curr->prv_success = true;
				monster_curr->num_steps++;
			}else if(direction == 1 && !touch_down){//down
				monster_curr->y_loc++;
				monster_curr->dir = 1;
				monster_curr->prv_success = true;
				monster_curr->num_steps++;
			}else if(direction == 2 && !touch_left){//left
				monster_curr->x_loc--;
				monster_curr->dir = 2;
				monster_curr->prv_success = true;
				monster_curr->num_steps++;
			}else if (direction == 3 && !touch_right){//right
				monster_curr->x_loc++;
				monster_curr->dir = 3;
				monster_curr->prv_success = true;
				monster_curr->num_steps++;
			}else{
				monster_curr->prv_success = false;
			}
			curr_monster_x = monster_curr->x_loc;
			curr_monster_y = monster_curr->y_loc;
				m_curr = m_head;
			while(m_curr != NULL){
				if(COLLISION_HARM){
			  harmed = check_collision(monster_curr, m_curr);
				}
				if(harmed){
					harmed = false;
					my_kills++;
					continue;
				}
				m_curr = m_curr->nxt;
			}
			if(COLLISION_HARM){
			  harmed = check_collision(monster_curr, m_curr);
				}
			if(harmed){
				harmed = false;
					continue;
				}
			//printf("my life count is: %d\n", life_count);
			lcd_draw_image(
					monster_curr->x_loc,
					MONSTER_WIDTH,
					monster_curr->y_loc,
					MONSTER_HEIGHT,
					monster,
					LCD_COLOR_BLACK,
					LCD_COLOR_WHITE);
			monster_curr = monster_curr->nxt;
		}
		if(life_count <= 2){
		lcd_draw_image(
							COLS/2 -4,
							HEART_WIDTH,
							30,
							HEART_HEIGHT,
							hearts,
							LCD_COLOR_BLACK,
							LCD_COLOR_BLACK);
		}
		if(life_count <= 1){
				lcd_draw_image(
							COLS/2 - 27,
							HEART_WIDTH,
							30,
							HEART_HEIGHT,
							hearts,
							LCD_COLOR_BLACK,
							LCD_COLOR_BLACK);
		}
		if(life_count <= 0){
					lcd_draw_image(
							COLS/2 - 50,
							HEART_WIDTH,
							30,
							HEART_HEIGHT,
							hearts,
							LCD_COLOR_BLACK,
							LCD_COLOR_BLACK);
		}
	}

		if(ALERT_BOMB_UPDATE1){
			ALERT_BOMB_UPDATE1 = false;
			if(bomb_count == 2) return;
			if(bomb_count <= 1){//the first one has been planted
				if(first_bomb_update_count%2 == 0 && !first_explode){
						lcd_draw_image(
							first_bomb.x_loc,
							IN_MAP_BOMB_WIDTH,
							first_bomb.y_loc,
							IN_MAP_BOMB_HEIGHT,
							in_map_bomb,
							LCD_COLOR_BLACK,
							LCD_COLOR_WHITE);
				}else{
						lcd_draw_image(
							first_bomb.x_loc,
							IN_MAP_BOMB_WIDTH,
							first_bomb.y_loc,
							IN_MAP_BOMB_HEIGHT,
							in_map_bomb,
							LCD_COLOR_WHITE,
							LCD_COLOR_WHITE);
				}
				first_bomb_update_count--;
			}
			if(bomb_count == 0){//the second bomb has also been planted
				if(second_bomb_update_count%2 == 0 && !second_explode){
						lcd_draw_image(
							second_bomb.x_loc,
							IN_MAP_BOMB_WIDTH,
							second_bomb.y_loc,
							IN_MAP_BOMB_HEIGHT,
							in_map_bomb,
							LCD_COLOR_BLACK,
							LCD_COLOR_WHITE);
				}else{
						lcd_draw_image(
							second_bomb.x_loc,
							IN_MAP_BOMB_WIDTH,
							second_bomb.y_loc,
							IN_MAP_BOMB_HEIGHT,
							in_map_bomb,
							LCD_COLOR_WHITE,
							LCD_COLOR_WHITE);
				}
				second_bomb_update_count--;
			}
			if(first_bomb_update_count == 0){
				first_explode = true;
							lcd_draw_image(
							first_bomb.x_loc,
							EXPLOSION_WIDTH,
							first_bomb.y_loc,
							EXPLOSION_HEIGHT,
							explosion,
							LCD_COLOR_BLACK,
							LCD_COLOR_WHITE);
				monster_curr = monster_head;
				while(monster_curr != NULL){
					killed_by_bomb = check_bomb_kills(monster_curr, first_bomb);
					if(killed_by_bomb){
						killed_by_bomb = false;
						my_kills++;
						continue;
					}
					monster_curr = monster_curr->nxt;
				}
				
			}
			if(second_bomb_update_count == 0){
				second_explode = true;
						  lcd_draw_image(
							second_bomb.x_loc,
							EXPLOSION_WIDTH,
							second_bomb.y_loc,
							EXPLOSION_HEIGHT,
							explosion,
							LCD_COLOR_BLACK,
							LCD_COLOR_WHITE);
				monster_curr = monster_head;
				while(monster_curr != NULL){
					killed_by_bomb = check_bomb_kills(monster_curr, second_bomb);
					if(killed_by_bomb){
						killed_by_bomb = false;
						my_kills++;
						continue;
					}
					monster_curr = monster_curr->nxt;
				}
				
			}
		}

}
		

//*****************************************************************************
//*****************************************************************************
int 
main(void)
{
	uint16_t x,y; // touch screen 
  uint8_t touch_event;
	char msg[80];
	static uint32_t debounce_cnt=0;
	
  initialize_hardware();

  put_string("\n\r");
  put_string("************************************\n\r");
  put_string("ECE353 - Spring 2018 HW3\n\r  ");
  put_string(group);
  put_string("\n\r     Name:");
  put_string(individual_1);
  put_string("\n\r     Name:");
  put_string(individual_2);
  put_string("\n\r");  
  put_string("************************************\n\r");

	//// Initialize Plane location and image ////
	
	tank.x_loc = COLS/2;
	tank.y_loc = ROWS/2;

	if(MASTER)
  {
    wireless_configure_device(myID, remoteID ) ;
  }
  else
  {
    wireless_configure_device(remoteID, myID) ;
  }
	
	enable_pwm(temp_frequency);
  // Reach infinite loop
  while(1){
		my_kills = 0;
		//state = 4;
		touch_event = ft6x06_read_td_status();
		if(touch_event > 0){
			y = ft6x06_read_y();
			x = ft6x06_read_x();
			printf("The Y cordinate is: %d\n\r",y);
			printf("The X cordinate is: %d\n\r",x);
			
			int playbutton_left_lane = BUTTON_WIDTH/2 -  BUTTON_WIDTH/2;
			int playbutton_right_lane = BUTTON_WIDTH/2 +  BUTTON_WIDTH/2;
			int playbutton_up_lane = COLS + (ROWS - COLS)/2 -  BUTTON_HEIGHT/2;
			int playbutton_down_lane = COLS + (ROWS - COLS)/2 + BUTTON_HEIGHT/2;
			
			int insbutton_left_lane = BUTTON_WIDTH + BUTTON_WIDTH/2 -  BUTTON_WIDTH/2;
			int insbutton_right_lane = BUTTON_WIDTH + BUTTON_WIDTH/2 +  BUTTON_WIDTH/2;
			int insbutton_up_lane = COLS + (ROWS - COLS)/2 -  BUTTON_HEIGHT/2;
			int insbutton_down_lane = COLS + (ROWS - COLS)/2 + BUTTON_HEIGHT/2;
			
			
			if(x <= playbutton_right_lane && x>= playbutton_left_lane && y <= playbutton_down_lane && x>= playbutton_up_lane){
				state = PLAYING; // play
			}
			
			if(x <= insbutton_right_lane && x>= insbutton_left_lane && y <= insbutton_down_lane && x>= insbutton_up_lane){
				state = INSTRUCTION; // ins
			}		
		}
		
		if(state == WELCOME){
		initialize_wecome_page();
		}
		if(state == INSTRUCTION){//entered instruction state
			instruction_state();
		}//deleted state 2 (matching state)
		if(state == PLAYING){//entered playing state 
			playing_state();
		}

		if(state == END){
			result_state();
		}
  }		// end of while(1) loop

}
