#include "debounce.h"
uint8_t debounce_cnt = 0;

void check_debounce(uint8_t last_4_bits, bool *already_launched, bool * alert_up, bool * alert_down, bool * alert_left, bool * alert_right){
		//reached 10ms indicated by flag set by handler
		debounce_cnt++;
		// pressed for 40ms
		if(debounce_cnt == 10){
			debounce_cnt=0;
			*already_launched = true;
			if(!(last_4_bits &(1<<UP_BUTTON_PIN))){//up is pressed
			*alert_up = true;
			*alert_down = false;
			*alert_left = false;
			*alert_right = false;
			}
			if(!(last_4_bits &(1<<LEFT_BUTTON_PIN))){//left is pressed
			*alert_up = false;
			*alert_down = false;
			*alert_left = true;
			*alert_right = false;
			}
			if(!(last_4_bits &(1<<DOWN_BUTTON_PIN))){//down is pressed
			*alert_up = false;
			*alert_down = true;
			*alert_left = false;
			*alert_right = false;
			}
			if(!(last_4_bits &(1<<RIGHT_BUTTON_PIN))){//right is pressed
			*alert_up = false;
			*alert_down = false;
			*alert_left = false;
			*alert_right = true;
			}
			//printf("up: %d, down: %d, left: %d, right: %d\n",*alert_up,*alert_down,*alert_left,*alert_right);
		}
}