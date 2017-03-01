/*
 *      Name:    snake_game.C
 * 			Description: This c file includes the tasks needed to implement a basic snake game
 *----------------------------------------------------------------------------
 *      Authors:
 *			Abhishek Ravi (205 651 80)
 *			Raj Lad	(205 242 37)
 *---------------------------------------------------------------------------
 * Please note that the fuctions used for INT0 button and the joystick in KDC.c has been retrived from 
 * C:\Software\Keil\ARM\Boards\Keil\MCB1700\RTX_Traffic and the following URL
 * http://www.ee.ryerson.ca/~courses/ee8205/labs/ARM-uVision_Tutorial.pdf
 */
#include <RTL.h>
#include "LPC17xx.H"                    /* LPC17xx definitions               */
#include "GLCD.h"
#include "LED.h"
#include <stdio.h>              
#include "Serial.h"
#include "KBD.h"
#include "ADC.h"

#define __FI        1                   /* Font index 16x24                  */
#define s1   Red
#define s2 	 Blue
#define s3 	 Magenta
#define OFFSET 10
#define RIGHTDIR 1
#define LEFTDIR -1
#define UPDIR 2
#define DOWNDIR -2
#define GAMEOVERPRI 12 
#define MOVEPRI	11
#define CONSUMEFOODPRI 10

typedef int bool;
#define true 1
#define false 0


OS_MUT mut_GLCD;                        /* Mutex to controll GLCD access  */

char text[10];

//GLOBAL VARIABLES/contstants
uint32_t MAXWIDTH = 320;
uint32_t MAXLENGTH = 240;
uint32_t initialX =  120;
uint32_t initialY = 160;
uint32_t body_size = 10;
bool GAME_OVER = 1; // flag for the int0 button interrupt 
int SPEED = 2;//DELAY -  lower the delay the faster the speed

/* Import external variables from IRQ.c file                                  */
extern uint8_t  clock_1s;

typedef struct{ //snake's body struct
	uint32_t headX;
	uint32_t headY;
	uint32_t tailX;
	uint32_t tailY;
	uint32_t x[100];
	uint32_t y[100];
	uint32_t size;
	int direction;// 1 is x -1 is negative x direction and 2 is y and -2 is negative y direction
} snakeObj;

typedef struct{
	uint32_t positionX;
	uint32_t positionY;
} foodObj;
	
//global objects for snake, food and poistion food
	snakeObj snake;
	foodObj food;
	foodObj food_poison;

// bit maps for the 3 appearing objects on the lcd
  unsigned short snake_bitmap[] = {   s1, s1, s1, s1, s1, s1, s1, s1, s1, s1, s1, s1, s1, s1, s1, s1, s1, s1, s1, s1,
																			s1, s1, s1, s1, s1, s1, s1, s1, s1, s1, s1, s1, s1, s1, s1, s1, s1, s1, s1, s1,
																			s1, s1, s1, s1, s1, s1, s1, s1, s1, s1, s1, s1, s1, s1, s1, s1, s1, s1, s1, s1,
																			s1, s1, s1, s1, s1, s1, s1, s1, s1, s1, s1, s1, s1, s1, s1, s1, s1, s1, s1, s1,
																			s1, s1, s1, s1, s1, s1, s1, s1, s1, s1, s1, s1, s1, s1, s1, s1, s1, s1, s1, s1,
																			s1, s1, s1, s1, s1, s1, s1, s1, s1, s1, s1, s1, s1, s1, s1, s1, s1, s1, s1, s1,
																			s1, s1, s1, s1, s1, s1, s1, s1, s1, s1, s1, s1, s1, s1, s1, s1, s1, s1, s1, s1,
																			s1, s1, s1, s1, s1, s1, s1, s1, s1, s1, s1, s1, s1, s1, s1, s1, s1, s1, s1, s1,
																			s1, s1, s1, s1, s1, s1, s1, s1, s1, s1, s1, s1, s1, s1, s1, s1, s1, s1, s1, s1,
																			s1, s1, s1, s1, s1, s1, s1, s1, s1, s1, s1, s1, s1, s1, s1, s1, s1, s1, s1, s1,
																			s1, s1, s1, s1, s1, s1, s1, s1, s1, s1, s1, s1, s1, s1, s1, s1, s1, s1, s1, s1,
																			s1, s1, s1, s1, s1, s1, s1, s1, s1, s1, s1, s1, s1, s1, s1, s1, s1, s1, s1, s1,
																			s1, s1, s1, s1, s1, s1, s1, s1, s1, s1, s1, s1, s1, s1, s1, s1, s1, s1, s1, s1,
																			s1, s1, s1, s1, s1, s1, s1, s1, s1, s1, s1, s1, s1, s1, s1, s1, s1, s1, s1, s1,
																			s1, s1, s1, s1, s1, s1, s1, s1, s1, s1, s1, s1, s1, s1, s1, s1, s1, s1, s1, s1,
																			s1, s1, s1, s1, s1, s1, s1, s1, s1, s1, s1, s1, s1, s1, s1, s1, s1, s1, s1, s1,
																			s1, s1, s1, s1, s1, s1, s1, s1, s1, s1, s1, s1, s1, s1, s1, s1, s1, s1, s1, s1,
																			s1, s1, s1, s1, s1, s1, s1, s1, s1, s1, s1, s1, s1, s1, s1, s1, s1, s1, s1, s1,
																			s1, s1, s1, s1, s1, s1, s1, s1, s1, s1, s1, s1, s1, s1, s1, s1, s1, s1, s1, s1,
																			s1, s1, s1, s1, s1, s1, s1, s1, s1, s1, s1, s1, s1, s1, s1, s1, s1, s1, s1, s1};
																			
unsigned short poison_bitmap[] = {   	s2, s2, s2, s2, s2, s2, s2, s2, s2, s2, s2, s2, s2, s2, s2, s2, s2, s2, s2, s2,
																			s2, s2, s2, s2, s2, s2, s2, s2, s2, s2, s2, s2, s2, s2, s2, s2, s2, s2, s2, s2,
																			s2, s2, s2, s2, s2, s2, s2, s2, s2, s2, s2, s2, s2, s2, s2, s2, s2, s2, s2, s2,
																			s2, s2, s2, s2, s2, s2, s2, s2, s2, s2, s2, s2, s2, s2, s2, s2, s2, s2, s2, s2,
																			s2, s2, s2, s2, s2, s2, s2, s2, s2, s2, s2, s2, s2, s2, s2, s2, s2, s2, s2, s2,
																			s2, s2, s2, s2, s2, s2, s2, s2, s2, s2, s2, s2, s2, s2, s2, s2, s2, s2, s2, s2,
																			s2, s2, s2, s2, s2, s2, s2, s2, s2, s2, s2, s2, s2, s2, s2, s2, s2, s2, s2, s2,
																			s2, s2, s2, s2, s2, s2, s2, s2, s2, s2, s2, s2, s2, s2, s2, s2, s2, s2, s2, s2,
																			s2, s2, s2, s2, s2, s2, s2, s2, s2, s2, s2, s2, s2, s2, s2, s2, s2, s2, s2, s2,
																			s2, s2, s2, s2, s2, s2, s2, s2, s2, s2, s2, s2, s2, s2, s2, s2, s2, s2, s2, s2,
																			s2, s2, s2, s2, s2, s2, s2, s2, s2, s2, s2, s2, s2, s2, s2, s2, s2, s2, s2, s2,
																			s2, s2, s2, s2, s2, s2, s2, s2, s2, s2, s2, s2, s2, s2, s2, s2, s2, s2, s2, s2,
																			s2, s2, s2, s2, s2, s2, s2, s2, s2, s2, s2, s2, s2, s2, s2, s2, s2, s2, s2, s2,
																			s2, s2, s2, s2, s2, s2, s2, s2, s2, s2, s2, s2, s2, s2, s2, s2, s2, s2, s2, s2,
																			s2, s2, s2, s2, s2, s2, s2, s2, s2, s2, s2, s2, s2, s2, s2, s2, s2, s2, s2, s2,
																			s2, s2, s2, s2, s2, s2, s2, s2, s2, s2, s2, s2, s2, s2, s2, s2, s2, s2, s2, s2,
																			s2, s2, s2, s2, s2, s2, s2, s2, s2, s2, s2, s2, s2, s2, s2, s2, s2, s2, s2, s2,
																			s2, s2, s2, s2, s2, s2, s2, s2, s2, s2, s2, s2, s2, s2, s2, s2, s2, s2, s2, s2,
																			s2, s2, s2, s2, s2, s2, s2, s2, s2, s2, s2, s2, s2, s2, s2, s2, s2, s2, s2, s2,
																			s2, s2, s2, s2, s2, s2, s2, s2, s2, s2, s2, s2, s2, s2, s2, s2, s2, s2, s2, s2};
																	
	
unsigned short food_bitmap[] =  { 
																			s3, s3, s3, s3, s3, s3, s3, s3, s3, s3, s3, s3, s3, s3, s3, s3, s3, s3, s3, s3,
																			s3, s3, s3, s3, s3, s3, s3, s3, s3, s3, s3, s3, s3, s3, s3, s3, s3, s3, s3, s3,
																			s3, s3, s3, s3, s3, s3, s3, s3, s3, s3, s3, s3, s3, s3, s3, s3, s3, s3, s3, s3,
																			s3, s3, s3, s3, s3, s3, s3, s3, s3, s3, s3, s3, s3, s3, s3, s3, s3, s3, s3, s3,
																			s3, s3, s3, s3, s3, s3, s3, s3, s3, s3, s3, s3, s3, s3, s3, s3, s3, s3, s3, s3,
																			s3, s3, s3, s3, s3, s3, s3, s3, s3, s3, s3, s3, s3, s3, s3, s3, s3, s3, s3, s3,
																			s3, s3, s3, s3, s3, s3, s3, s3, s3, s3, s3, s3, s3, s3, s3, s3, s3, s3, s3, s3,
																			s3, s3, s3, s3, s3, s3, s3, s3, s3, s3, s3, s3, s3, s3, s3, s3, s3, s3, s3, s3,
																			s3, s3, s3, s3, s3, s3, s3, s3, s3, s3, s3, s3, s3, s3, s3, s3, s3, s3, s3, s3,
																			s3, s3, s3, s3, s3, s3, s3, s3, s3, s3, s3, s3, s3, s3, s3, s3, s3, s3, s3, s3,
																			s3, s3, s3, s3, s3, s3, s3, s3, s3, s3, s3, s3, s3, s3, s3, s3, s3, s3, s3, s3,
																			s3, s3, s3, s3, s3, s3, s3, s3, s3, s3, s3, s3, s3, s3, s3, s3, s3, s3, s3, s3,
																			s3, s3, s3, s3, s3, s3, s3, s3, s3, s3, s3, s3, s3, s3, s3, s3, s3, s3, s3, s3,
																			s3, s3, s3, s3, s3, s3, s3, s3, s3, s3, s3, s3, s3, s3, s3, s3, s3, s3, s3, s3,
																			s3, s3, s3, s3, s3, s3, s3, s3, s3, s3, s3, s3, s3, s3, s3, s3, s3, s3, s3, s3,
																			s3, s3, s3, s3, s3, s3, s3, s3, s3, s3, s3, s3, s3, s3, s3, s3, s3, s3, s3, s3,
																			s3, s3, s3, s3, s3, s3, s3, s3, s3, s3, s3, s3, s3, s3, s3, s3, s3, s3, s3, s3,
																			s3, s3, s3, s3, s3, s3, s3, s3, s3, s3, s3, s3, s3, s3, s3, s3, s3, s3, s3, s3,
																			s3, s3, s3, s3, s3, s3, s3, s3, s3, s3, s3, s3, s3, s3, s3, s3, s3, s3, s3, s3};																	
							

//delay function in multiples of 100ms
void delayTime(unsigned  long time  ){
	int j; 
	int i = 1333333;   // This number is equivalent to 100 ms
	for (j = 0 ; j < i * time ; j++){}
}

// gets increment in x direction depending on the movement direction (if moving perpendicular to x increment will be zero
int getXdirection(int override, int direction){
	int dir;
	if (override == 1){ // override flag determines if you want to use snakes direction or hardcoded direction
		dir = direction;
	}
	else{
		dir = snake.direction;
	}
	if (dir == RIGHTDIR){// increments in positive x axis
		return body_size;
	}
	else if (dir == LEFTDIR){// increments in negative x axis
		return (-1*(body_size));
	}
	else {
		return 0;
	}
}

// finds y increment depending on direction of movement, same as x but for Y direction
int getYdirection(int override, int direction){ 
	int dir;
	if (override == 1){
		dir = direction;
	}
	else{
		dir = snake.direction;
	}
	if (dir == UPDIR){
		return body_size;
	}
	else if (dir == DOWNDIR){
		return (-body_size);
	}
	else {
		return 0;
	}
}
__task void moveSnake(){//move whole snake forward one block
	int i, xIncr, yIncr;
	delayTime(SPEED);
	for(i = 0; i < snake.size; i++){
		GLCD_Bitmap(snake.x[i], snake.y[i], body_size, body_size, (unsigned char*)White);
		xIncr = getXdirection(0, 0);
		yIncr = getYdirection(0, 0);
		snake.x[i] += xIncr;
		snake.y[i] += yIncr;
		GLCD_Bitmap(snake.x[i], snake.y[i], body_size, body_size, (unsigned char*)snake_bitmap);
	}
	os_tsk_delete_self ();
	
}

//determines collision between head of snake and food, growing snakes size
__task void foodConsume(){
		snake.size++; // increase the sanke size
		LED_On(snake.size - 2);
		
		GLCD_Bitmap(food.positionX, food.positionY, body_size, body_size , (unsigned char*)White);
		snake.x[snake.size - 1] = snake.x[snake.size -2] - getXdirection(0,0);
		snake.y[snake.size - 1] = snake.y[snake.size -2] - getYdirection(0,0);
	
		//reposition the food
		food.positionY = getRand(0,32); 
		food.positionY *= 10;
		food.positionY  = food.positionY % MAXLENGTH;
		food.positionX = getRand(0,24);
		food.positionX *= 10;
		food.positionX  = food.positionX % MAXWIDTH;
		GLCD_Bitmap(food.positionX, food.positionY, body_size, body_size , (unsigned char*)food_bitmap);	
		//randomize the poison again
		GLCD_Bitmap(food_poison.positionX, food_poison.positionY, body_size, body_size , (unsigned char*)White);
		food_poison.positionX = rand() % 320;
		food_poison.positionY = rand() % 240;
		GLCD_Bitmap(food_poison.positionX, food_poison.positionY, body_size, body_size , (unsigned char*)poison_bitmap);
		os_tsk_delete_self ();
}

// renders the turn of the snake 
void turnSnake(int oldDirec){
	int i, xIncr, yIncr, counter;
	bool isConsume = false;
	if (snake.size < 1){//case where snake is only size of one 
		GLCD_Bitmap(snake.x[0], snake.y[0], body_size, body_size, (unsigned char*)White);
		xIncr = getXdirection(0, 0);
		yIncr = getYdirection(0, 0);
		snake.x[0] += xIncr;
		snake.y[0] += yIncr;
		GLCD_Bitmap(snake.x[0], snake.y[0], body_size, body_size, (unsigned char*)snake_bitmap);
		delayTime(1.5);
		return;
	}
	for (i = 0; i < snake.size; i++){//takes care of case where snake is longer than one
		counter = 0;
		while(counter <= i){//renders blocks in new direction
			if (counter == 0){
				if(((food.positionX - snake.x[0] <= OFFSET) || (snake.x[0] - food.positionX <= OFFSET)) && ((food.positionY  - snake.y[0]  <= OFFSET)||(snake.y[0] - food.positionY <= OFFSET))){
					isConsume = true;
				}
			}
			GLCD_Bitmap(snake.x[counter], snake.y[counter], body_size, body_size, (unsigned char*)White);
			xIncr = getXdirection(0, 0);
			yIncr = getYdirection(0, 0);
			snake.x[counter] += xIncr;
			snake.y[counter] += yIncr;
			GLCD_Bitmap(snake.x[counter], snake.y[counter], body_size, body_size, (unsigned char*)snake_bitmap);
			counter++;
		}
		//move body elements on in old direction
		counter = i + 1;
		while(counter < snake.size){//renders blocks in old direction
			GLCD_Bitmap(snake.x[counter], snake.y[counter], body_size, body_size, (unsigned char*)White);
			xIncr = getXdirection(1, oldDirec);
			yIncr = getYdirection(1, oldDirec);
			snake.x[counter] += xIncr;
			snake.y[counter] += yIncr;
			GLCD_Bitmap(snake.x[counter], snake.y[counter], body_size, body_size, (unsigned char*)snake_bitmap);
			counter++;
		}
		delayTime(SPEED);
	}
	if (isConsume){//considers case where food is consumed during turn
		os_tsk_create(foodConsume, CONSUMEFOODPRI);
	}
	os_tsk_create(moveSnake, MOVEPRI);
}

int getRand(int min,int max){ //randomizer for food objects
     return(rand()%(max-min)+min);
}
__task void init (void);

__task void gameOver(void) {
	int i;
	GLCD_Clear(Black);                         /* Clear graphical LCD display   */
  GLCD_SetBackColor(Purple);
  GLCD_SetTextColor(LightGrey);
  GLCD_DisplayString(0, 0, __FI, "      GAME OVER       ");
  GLCD_DisplayString(1, 0, __FI, "  Please press int0   ");
  GLCD_DisplayString(2, 0, __FI, "  Restart the Game    ");
	while(GAME_OVER == 1){
		GAME_OVER = INT0_Get(); // exit loop when interrut from the int0 push button
	}
	GAME_OVER = 1; // reset the interrupt flag back to 1 (false)
	for (i = 0; i < snake.size - 1; i++){ // turn off all LEDs when the game ends
		LED_Off(i);
	}
	
	//start a new snake
	snake.size = 1;
	snake.x[0] = initialX;
	snake.y[0] = initialY;
	snake.direction = 1;
	
	//random food objects
	food.positionY = getRand(0,32); 
	food.positionY *= 10;
	food.positionY  = food.positionY % MAXLENGTH;
	food.positionX = getRand(0,24);
	food.positionX *= 10;
	food.positionX  = food.positionX % MAXWIDTH;
	
	//random food poison objects
	food_poison.positionY = (rand() % 320);
	food_poison.positionX = (rand() % 240);
	// reinitialize 
  LED_Init();                                /* LED Initialization            */
  SER_Init();                                /* UART Initialization           */
  ADC_Init();                                /* ADC Initialization            */
	KBD_Init();																	/* KDB Initialization 					*/
  GLCD_Init();                               /* Initialize graphical LCD      */
  GLCD_Clear(White);                         /* Clear graphical LCD display   */
	
	GLCD_Bitmap(food.positionX, food.positionY, body_size, body_size, (unsigned char*)food_bitmap);//generate food block
	GLCD_Bitmap(food_poison.positionX,food_poison.positionY , body_size, body_size, (unsigned char*)poison_bitmap);//generate food block
	
	os_tsk_delete_self ();
}

__task void init (void) { // initialize the base task
	uint32_t joy_stick_reading;
	int oldDirec = 0;
  while (1) {// inifinate loop
		
		//base coditions for game over
		if(snake.x[0] < 0 || snake.y[0] < 0  || snake.x[0] > 320 || snake.y[0] > 240 ){
			os_tsk_create(gameOver, GAMEOVERPRI);
		}
		if(((food_poison.positionX - snake.x[0] <= OFFSET) || (snake.x[0] - food_poison.positionX <= OFFSET)) && ((food_poison.positionY  - snake.y[0]  <= OFFSET)||(snake.y[0] - food_poison.positionY <= OFFSET))){
			os_tsk_create(gameOver, GAMEOVERPRI);
		}
		if(((food.positionX - snake.x[0] <= OFFSET) || (snake.x[0] - food.positionX <= OFFSET)) && ((food.positionY  - snake.y[0]  <= OFFSET)||(snake.y[0] - food.positionY <= OFFSET))){
			os_tsk_create(foodConsume, CONSUMEFOODPRI);
		}
		joy_stick_reading = get_button();
		
		//create task for rendering movement of snake
		os_tsk_create(moveSnake, MOVEPRI);
		
		// joystick readins to render new direction of the snake
		if(joy_stick_reading == KBD_RIGHT){
			if (snake.direction != LEFTDIR){
				oldDirec = snake.direction;
				snake.direction = RIGHTDIR;
				turnSnake(oldDirec);
			}			
		}
		else if (joy_stick_reading == KBD_LEFT){
			if (snake.direction != RIGHTDIR){
				oldDirec = snake.direction;
				snake.direction = LEFTDIR;
				turnSnake(oldDirec);
			}
		}
		else if (joy_stick_reading == KBD_UP){
			if (snake.direction != DOWNDIR){
				oldDirec = snake.direction;
				snake.direction = UPDIR;
				turnSnake(oldDirec);
			}
		}
		else if (joy_stick_reading == KBD_DOWN){
			if (snake.direction != UPDIR){
				oldDirec = snake.direction;
				snake.direction = DOWNDIR;
				turnSnake(oldDirec);
			}
		}
  }
	os_tsk_delete_self ();
}

/*----------------------------------------------------------------------------
  Main: Initialize and start RTX Kernel
 *---------------------------------------------------------------------------*/
int main (void) {
	
	//first snake object
	snake.size = 1;
	snake.x[0] = initialX;
	snake.y[0] = initialY;
	snake.direction = 1;
	
	//random food objects
	food.positionY = getRand(0,32); 
	food.positionY *= 10;
	food.positionY  = food.positionY % MAXLENGTH;
	food.positionX = getRand(0,24);
	food.positionX *= 10;
	food.positionX  = food.positionX % MAXWIDTH;
	
	//random food poison objects
	food_poison.positionY = (rand() % 320);
	food_poison.positionX = (rand() % 240);
	
	//initialize everything
	SystemInit();
	SystemCoreClockUpdate();
  LED_Init();                                /* LED Initialization            */
  SER_Init();                                /* UART Initialization           */
  ADC_Init();                                /* ADC Initialization            */
	KBD_Init();																	/* KDB Initialization 					*/
  GLCD_Init();                               /* Initialize graphical LCD      */
  GLCD_Clear(White);                         /* Clear graphical LCD display   */
	
	GLCD_Bitmap(food.positionX, food.positionY, body_size, body_size, (unsigned char*)food_bitmap);//generate food block
	GLCD_Bitmap(food_poison.positionX,food_poison.positionY , body_size, body_size, (unsigned char*)poison_bitmap);//generate food block
  SysTick_Config(SystemCoreClock/100);       /* Generate interrupt each 10 ms */
  
	os_sys_init(init);                        /* initialize the snake */
}
