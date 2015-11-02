#include "hardware.h"
#include "light_ws2812.h"

struct CRGB { uint8_t g; uint8_t r; uint8_t b; };
struct CRGB led[14]; //This is an array of 14 triples, so 14*3=42 bytes

int8_t j;

int8_t mode = 1;
int8_t buttonWait = 0;

TICK_COUNT buttonWaitStartTime;
TICK_COUNT buttonWaitTime = 1000000;
TICK_COUNT unusedButtonRemainingTime;

#define NUMMODES 6

TICK_COUNT halfperiods[2];
TICK_COUNT remainingtime;
TICK_COUNT switchtime;
int8_t dir = -1; //direction
int8_t set = 0; //which set of LED's is changing colors
int8_t minbright = 0; // minimum brightness
int8_t maxbright = 30; // maximum brightness
int8_t newbrightness; //
int8_t delta; // amount to change the brightness; calculated live

int k;
int y = 2;
int z = 7;
int w = 12;
		

void setLeds(){
	// Send update to all LEDs
	cli();				// Disable interrupts. Can be removed if no interrupts are used.
	ws2812_sendarray((uint8_t *)&led[0],42);
	sei();				// Enable interrupts.
}

// Initialise the hardware
void appInitHardware(void) {
	initHardware();
}
// Initialise the software
TICK_COUNT appInitSoftware(TICK_COUNT loopStart){
	for (int i=0; i < 14; i+=1){
			led[i].r = 0;
			led[i].g = 0;
			led[i].b = 0;
		}
		
	setLeds();
	
	halfperiods[0] = 500000; // us
	halfperiods[1] = 625000;
	switchtime = clockGetus();
	newbrightness = minbright;
	
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
		
			// Clear vals for new mode
			for (int i=set; i < 14; i++){
				led[i].r = 0;
				led[i].g = 0;
				led[i].b = 0;
			}
			setLeds();
		}
	}
	// Wait 1 second after noticing a button press before allowing another button press
	else {
		if (clockHasElapsedGetOverflow(buttonWaitStartTime, buttonWaitTime, &unusedButtonRemainingTime)){
			buttonWait = 0;
		}
	}

	/// alternating half bright half fading
	if (mode == 1) {
		if (clockHasElapsedGetOverflow(switchtime, halfperiods[set], &remainingtime)) {
			// rprintf("boom!\n");
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
				// led[i].b = maxbright * 2;
				led[i].r = maxbright * 2;
				led[i].g = maxbright;
			}
		}
		
		// Calculate brightness of dimmer LEDs
		if(dir == 1){ // increasing brightness
			newbrightness = maxbright - (maxbright - minbright) * remainingtime / halfperiods[set];
		}
		else{ // decreasing brightness
			newbrightness = minbright + (maxbright - minbright) * remainingtime / halfperiods[set];
		}
		// Monocolor
		// newbrightness *= 2;
		// newbrightness += 1;
		
		for (int i=set; i < 14; i+=2){
			// blue
			// led[i].b = newbrightness;
			// orange
			led[i].r = newbrightness * 2 + 1;
			led[i].g = newbrightness + 1;
		}
		
		setLeds();
	}
	
	///
	else if (mode == 2) {
		if (clockHasElapsedGetOverflow(switchtime, halfperiods[set], &remainingtime)) {
			// rprintf("boom!\n");
			set = (set + 1) % 2;
			switchtime = clockGetus();
			remainingtime = halfperiods[set] - remainingtime;
		}
		
		for (int i=0; i < 14; i+=2){
			led[i].b = 0;
		}
		
		j = (14 * (remainingtime /1000))  / (halfperiods[set] / 1000);
		
		led[j].b = maxbright;
		
		setLeds();
	}
	
	/// moving orange
	else if (mode == 3) {
		if (clockHasElapsedGetOverflow(switchtime, halfperiods[set], &remainingtime)) {
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
		led[j].g = maxbright/2;
		//led[j].b = maxbright;
		
		setLeds();
	}
	
	/// FLASHING WHITE
	else if (mode == 4) {
		if (clockHasElapsedGetOverflow(switchtime, halfperiods[set], &remainingtime)) {
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
		
		setLeds();
	}
	
	else if (mode == 5)
	{	
		k = (k+1)%w;
		y = 4;
		z = 7;
		w = 7;
		int val = (k*y)%z;
		// >>>  for k in xrange(w): print (k*y)%z;
		// for (int k=0; k < w; k+=x)
		// {
			// val = (k*y)%z;
		// }
		for (int i=0; i < 14; i++){
			led[i].r = maxbright * 2;
			led[i].g = maxbright;
			// led[i].b = newbrightness * 2;
		}
		led[val].r = maxbright * 2;
		led[val].g = maxbright * 2;
		
		led[val+7].r = maxbright * 3;
		led[val+7].g = maxbright * 2;
		
		setLeds();
		return 500000;
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

	else if (mode == 0){
		
		for (int i=0; i < 14; i++){
			led[i].r = 0;
			led[i].g = 0;
			led[i].b = 0;
		}
		
		setLeds();	
	}
	
	return 20000;
}
