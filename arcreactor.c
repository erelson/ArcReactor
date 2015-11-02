#include "hardware.h"
#include "light_ws2812.h"

struct CRGB { uint8_t g; uint8_t r; uint8_t b; };
struct CRGB led[14]; //This is an array of 14 triples, so 14*3=42 bytes

int8_t i;
int8_t j;

int8_t mode;
int8_t buttonWait;

TICK_COUNT buttonWaitStartTime;
TICK_COUNT buttonWaitTime;
TICK_COUNT unusedButtonRemainingTime;

#define NUMMODES 4

TICK_COUNT halfperiods[2];
TICK_COUNT remainingtime;
TICK_COUNT switchtime;
int8_t dir; //direction
int8_t set; //which set of LED's is changing colors
int8_t minbright; // minimum brightness
int8_t maxbright; // maximum brightness
int8_t newbrightness; //
int8_t delta; // amount to change the brightness; calculated live


// Initialise the hardware
void appInitHardware(void) {
	initHardware();
}
// Initialise the software
TICK_COUNT appInitSoftware(TICK_COUNT loopStart){
	
	led[0].r=0; led[0].g=0 ;led[0].b=maxbright;
	led[1].r=0; led[1].g=0 ;led[1].b=maxbright;
	led[2].r=0; led[2].g=0 ;led[2].b=maxbright;
	led[3].r=0; led[3].g=0 ;led[3].b=maxbright;
	led[4].r=0; led[4].g=0 ;led[4].b=maxbright;
	led[5].r=0; led[5].g=0 ;led[5].b=maxbright;
	led[6].r=0; led[6].g=0 ;led[6].b=maxbright;
	led[7].r=0; led[7].g=0 ;led[7].b=maxbright;
	led[8].r=0; led[8].g=0 ;led[8].b=maxbright;
	led[9].r=0; led[9].g=0 ;led[9].b=maxbright;
	led[10].r=0;led[10].g=0;led[10].b=maxbright;
	led[11].r=0;led[11].g=0;led[11].b=maxbright;
	led[12].r=0;led[12].g=0;led[12].b=maxbright;
	led[13].r=0;led[13].g=0;led[13].b=maxbright;
	// led[14].r=0;led[14].g=0;led[14].b=minbright;
	// led[15].r=0;led[15].g=0;led[15].b=minbright;
	
	
	cli();				// Disable interrupts. Can be removed if no interrupts are used.
	ws2812_sendarray((uint8_t *)&led[0],42);
	sei();				// Enable interrupts.
	
	halfperiods[0] = 500000; // us
	halfperiods[1] = 625000;
	switchtime = clockGetus();
	// halfperiods[0] = 3000; // ms
	// halfperiods[1] = 5000;
	// switchtime = clockGetus() / 1000;
	
	minbright = 0; //0
	maxbright = 30; //40
	newbrightness = minbright;
	set = 0;
	dir = -1;
	mode = 0;
	buttonWait = 0;
	buttonWaitTime = 1000000;
	
	return 0;
}
// This is the main loop
TICK_COUNT appControl(LOOP_COUNT loopCount, TICK_COUNT loopStart) {
	
	//Toggle mode via button
	if (!buttonWait){
		
		if(SWITCH_pressed(&button)){
			// pressed
			mode = (mode + 1) % NUMMODES;
			buttonWait = 1;
			buttonWaitStartTime = loopStart;
		}
		for (int i=set; i < 14; i++){
			led[i].r = 0;
			led[i].g = 0;
		}
	}
	else {
		if (clockHasElapsedGetOverflow(buttonWaitStartTime, buttonWaitTime, &unusedButtonRemainingTime)){
			buttonWait = 0;
		}
	}

	/// alternating half bright half fading
	if (mode == 0) {
		if (clockHasElapsedGetOverflow(switchtime, halfperiods[set], &remainingtime)) {
			rprintf("boom!\n");
			delay_ms(400);
			// Switch modes if we've reached the end of increasing brightness
			if (dir == 1){
				set = (set + 1) % 2;
			}
			switchtime = clockGetus();
			dir *= -1;
			remainingtime = halfperiods[set] - remainingtime;
			
			// Set half the LEDs to maximum brightness
			for (int i=(1-set); i < 14; i+=2){
				led[i].b = maxbright * 2;
			}
		}
		
		// Calculate brightness of dimmer LEDs
		if(dir == 1){ // increasing brightness
			newbrightness = maxbright - (maxbright - minbright) * remainingtime / halfperiods[set];
		}
		else{ // decreasing brightness
			newbrightness = minbright + (maxbright - minbright) * remainingtime / halfperiods[set];
		}
		newbrightness *= 2;
		newbrightness += 1;
		
		for (int i=set; i < 14; i+=2){
			led[i].b = newbrightness;
		}
		
		// Send update to all LEDs
		cli();				// Disable interrupts. Can be removed if no interrupts are used.
		ws2812_sendarray((uint8_t *)&led[0],42);
		sei();				// Enable interrupts.

	}
	
	else if (mode == 1) {
		if (clockHasElapsedGetOverflow(switchtime, halfperiods[set], &remainingtime)) {
			rprintf("boom!\n");
			set = (set + 1) % 2;
			switchtime = clockGetus();
			remainingtime = halfperiods[set] - remainingtime;
			
		}
		
		
		for (int i=0; i < 14; i+=2){
			led[i].b = 0;
		}
		
		j = (14 * (remainingtime /1000))  / (halfperiods[set] / 1000);
		
		led[j].b = maxbright;
		
	
		cli();				// Disable interrupts. Can be removed if no interrupts are used.
		ws2812_sendarray((uint8_t *)&led[0],42);
		sei();				// Enable interrupts.
	}
	
	/// moving white
	else if (mode == 2) {
		if (clockHasElapsedGetOverflow(switchtime, halfperiods[set], &remainingtime)) {
			rprintf("boom!\n");
			//delay_ms(250);
			// Switch modes if we've reached the end of increasing brightness
			//if (dir == 1){
				set = (set + 1) % 2;
			//}
			switchtime = clockGetus();
			//dir *= -1;
			remainingtime = halfperiods[set] - remainingtime;
			
		}
		
		
		for (int i=0; i < 14; i++){
			led[i].r = 0;
			led[i].g = 0;
			led[i].b = 0;
		}
		
		j = (14 * (remainingtime /1000))  / (halfperiods[set] / 1000);
		
		led[j].r = maxbright;
		led[j].g = maxbright;
		led[j].b = maxbright;
		
	
		cli();				// Disable interrupts. Can be removed if no interrupts are used.
		ws2812_sendarray((uint8_t *)&led[0],42);
		sei();				// Enable interrupts.
	}




	else if (mode == 3) {
		if (clockHasElapsedGetOverflow(switchtime, halfperiods[set], &remainingtime)) {
			rprintf("boom!\n");
			delay_ms(400);
			// Switch modes if we've reached the end of increasing brightness
			if (dir == 1){
				set = (set + 1) % 2;
			}
			switchtime = clockGetus();
			dir *= -1;
			remainingtime = halfperiods[set] - remainingtime;
			
		}
		
		// Calculate brightness
		if(dir == 1){ // increasing brightness
			newbrightness = maxbright - (maxbright - minbright) * remainingtime / halfperiods[set];
		}
		else{ // decreasing brightness
			newbrightness = minbright + (maxbright - minbright) * remainingtime / halfperiods[set];
		}
		
		for (int i=0; i < 14; i++){
			led[i].r = newbrightness * 2;
			led[i].g = newbrightness * 2;
			led[i].b = newbrightness * 2;
		}
		
		// Send update to all LEDs
		cli();				// Disable interrupts. Can be removed if no interrupts are used.
		ws2812_sendarray((uint8_t *)&led[0],42);
		sei();				// Enable interrupts.

	}
	
	
	// ///////
	// // BlinkM control (doesn't work; probably need pull up resistors)
	// uint8_t blinkm_packet_setcolor[4];
	
	// blinkm_packet_setcolor[0] = (uint8_t)'n';
	// blinkm_packet_setcolor[1] = 50;
	// blinkm_packet_setcolor[2] = 50;
	// blinkm_packet_setcolor[3] = 50;
		
	// i2cMasterSend(&blinkm.i2cInfo, sizeof(blinkm_packet_setcolor), blinkm_packet_setcolor);
	// // i2cMasterSend(&blinkm.i2cInfo, sizeof(blinkm_packet_setcolor), *blinkm_packet_setcolor);
	// delay_ms(50);

	return 20000;
}
