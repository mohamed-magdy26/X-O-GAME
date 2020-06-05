// SpaceInvaders.c
// Runs on LM4F120/TM4C123
// Jonathan Valvano and Daniel Valvano
// This is a starter project for the edX Lab 15
// In order for other students to play your game
// 1) You must leave the hardware configuration as defined
// 2) You must not add/remove any files from the project
// 3) You must add your code only this this C file
// I.e., if you wish to use code from sprite.c or sound.c, move that code in this file
// 4) It must compile with the 32k limit of the free Keil

// April 10, 2014
// http://www.spaceinvaders.de/
// sounds at http://www.classicgaming.cc/classics/spaceinvaders/sounds.php
// http://www.classicgaming.cc/classics/spaceinvaders/playguide.php
/* This example accompanies the books
   "Embedded Systems: Real Time Interfacing to Arm Cortex M Microcontrollers",
   ISBN: 978-1463590154, Jonathan Valvano, copyright (c) 2013

   "Embedded Systems: Introduction to Arm Cortex M Microcontrollers",
   ISBN: 978-1469998749, Jonathan Valvano, copyright (c) 2013

 Copyright 2014 by Jonathan W. Valvano, valvano@mail.utexas.edu
    You may use, edit, run or distribute this file
    as long as the above copyright notice remains
 THIS SOFTWARE IS PROVIDED "AS IS".  NO WARRANTIES, WHETHER EXPRESS, IMPLIED
 OR STATUTORY, INCLUDING, BUT NOT LIMITED TO, IMPLIED WARRANTIES OF
 MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE APPLY TO THIS SOFTWARE.
 VALVANO SHALL NOT, IN ANY CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL,
 OR CONSEQUENTIAL DAMAGES, FOR ANY REASON WHATSOEVER.
 For more information about my classes, my research, and my books, see
 http://users.ece.utexas.edu/~valvano/
 */
// ******* Required Hardware I/O connections*******************
// Slide pot pin 1 connected to ground
// Slide pot pin 2 connected to PE2/AIN1
// Slide pot pin 3 connected to +3.3V 
// fire button connected to PE0
// special weapon fire button connected to PE1
// 8*R resistor DAC bit 0 on PB0 (least significant bit)
// 4*R resistor DAC bit 1 on PB1
// 2*R resistor DAC bit 2 on PB2
// 1*R resistor DAC bit 3 on PB3 (most significant bit)
// LED on PB4
// LED on PB5

// Blue Nokia 5110
// ---------------
// Signal        (Nokia 5110) LaunchPad pin
// Reset         (RST, pin 1) connected to PA7
// SSI0Fss       (CE,  pin 2) connected to PA3
// Data/Command  (DC,  pin 3) connected to PA6
// SSI0Tx        (Din, pin 4) connected to PA5
// SSI0Clk       (Clk, pin 5) connected to PA2
// 3.3V          (Vcc, pin 6) power
// back light    (BL,  pin 7) not connected, consists of 4 white LEDs which draw ~80mA total
// Ground        (Gnd, pin 8) ground

// Red SparkFun Nokia 5110 (LCD-10168)
// -----------------------------------
// Signal        (Nokia 5110) LaunchPad pin
// 3.3V          (VCC, pin 1) power
// Ground        (GND, pin 2) ground
// SSI0Fss       (SCE, pin 3) connected to PA3
// Reset         (RST, pin 4) connected to PA7
// Data/Command  (D/C, pin 5) connected to PA6
// SSI0Tx        (DN,  pin 6) connected to PA5
// SSI0Clk       (SCLK, pin 7) connected to PA2
// back light    (LED, pin 8) not connected, consists of 4 white LEDs which draw ~80mA total

#include "tm4c123gh6pm.h"
#include "Nokia5110.h"
#include "Random.h"
#include "TExaS.h"

void DisableInterrupts(void); // Disable interrupts
void EnableInterrupts(void);  // Enable interrupts
void Timer2_Init(unsigned long period);
void Delay100ms(unsigned long count);
char WinnerChecker(void);
unsigned long TimerCount;
unsigned long Semaphore;
char BoardArray[4][4];
char TheWinner=0;
char CurrentPostion,player,GameBoardArea,game_element,done;



	void PortF_Init(void){ volatile unsigned long delay;
  SYSCTL_RCGC2_R |= 0x00000020;     // 1) F clock
  delay = SYSCTL_RCGC2_R;           // delay   
  GPIO_PORTF_LOCK_R = 0x4C4F434B;   // 2) unlock PortF PF0  
  GPIO_PORTF_CR_R = 0x1F;           // allow changes to PF4-0       
  GPIO_PORTF_AMSEL_R = 0x00;        // 3) disable analog function
  GPIO_PORTF_PCTL_R = 0x00000000;   // 4) GPIO clear bit PCTL  
  GPIO_PORTF_DIR_R = 0x0E;          // 5) PF4,PF0 input, PF3,PF2,PF1 output   
  GPIO_PORTF_AFSEL_R = 0x00;        // 6) no alternate function
  GPIO_PORTF_PUR_R = 0x11;          // enable pullup resistors on PF4,PF0       
  GPIO_PORTF_DEN_R = 0x1F;          // 7) enable digital pins PF4-PF0        
}
void PORTE_INIT(void){
	volatile unsigned long delay;
	SYSCTL_RCGC2_R |= 0x00000010;     // 1) F clock
  delay = SYSCTL_RCGC2_R;           // delay   
  GPIO_PORTE_LOCK_R = 0x4C4F434B;   // 2) unlock PortE
  GPIO_PORTE_CR_R = 0x03;           // allow changes to PE2       
  GPIO_PORTE_AMSEL_R = 0x00;        // 3) disable analog function
  GPIO_PORTE_PCTL_R = 0x00000000;   // 4) GPIO clear bit PCTL  
  GPIO_PORTE_DIR_R = 0x02;          // 5) PE0 input, PE1 output   
  GPIO_PORTE_AFSEL_R = 0x00;        // 6) no alternate function
  GPIO_PORTE_PDR_R= 0x01;          // enable pulldown resistors on PE0       
  GPIO_PORTE_DEN_R = 0x01;          // 7) enable digital pins PE0-PE1
}

void CreateGame(){
	char i,j,x,y;
	x=84/GameBoardArea;
	y=48/GameBoardArea;
	
	for(i=0;i<GameBoardArea;i++){
		for(j=0;j<48;j++)Nokia5110_SetPixel(x*i,j);
	  for(j=0;j<84;j++)Nokia5110_SetPixel(j,y*i);
	}
	
}

char start(){
	char RightMove, LeftMove ,Set_X_O,choose=3;
	
	   
	Nokia5110_SetCursor(0,0);
	Nokia5110_OutString("XO GAME");
	Nokia5110_SetCursor(0,2);
	Nokia5110_OutString("PRESS ANY");
	
	while(1){
		 RightMove = GPIO_PORTE_DATA_R&0x01;     // read PF4 into RightMove
     LeftMove = GPIO_PORTE_DATA_R&0x02;     // read PF0 into LeftMove
	   Set_X_O = GPIO_PORTF_DATA_R&0x10;     // read PE0 into Set_X_O
	
		      

	if((LeftMove || RightMove || !(Set_X_O) )){
		      Delay100ms(1);
					Nokia5110_Clear();
					Nokia5110_SetCursor(0,3);
					Delay100ms(1);
					Nokia5110_OutString("starting");
					Delay100ms(1);
					Nokia5110_SetCursor(0,3);
					Nokia5110_OutString("starting.");
					Delay100ms(1);
					Nokia5110_SetCursor(0,3);
					Nokia5110_OutString("starting..");
					Delay100ms(1);
					Nokia5110_SetCursor(0,3);
					Nokia5110_OutString("starting...");
					Delay100ms(2);
return choose;
		}
	}
}

char WinnerChecker(void){
	   char i,j,CX=0,CY=0,CcX=0,CcY=0,CdX=0,CdY=0,CdrX=0,CdrY=0,Full=1;
		 for(i=0;i<GameBoardArea;i++){
			 CX=0,CY=0,CcX=0,CcY=0;
		    for(j=0;j<GameBoardArea;j++){
					if(BoardArray[i][j]=='X')CX++;  // checking X rows
					if(BoardArray[i][j]=='O')CY++;	// checking O rows
          if(BoardArray[j][i]=='X')CcX++;  // checking X columns
          if(BoardArray[j][i]=='O')CcY++;	// checking O columns
          if(BoardArray[i][j]==0) Full=0;			
		 }
				if(BoardArray[i][i]=='X')CdX++;  // checking diagonal case for X
				if(BoardArray[i][i]=='O')CdY++;  // checking diagonal case for O
		    if(BoardArray[i][GameBoardArea-1-i]=='X')CdrX++; // checking the other diagonal case for X
				if(BoardArray[i][GameBoardArea-1-i]=='O')CdrY++; // checking the other diagonal case for O
				if(CX==GameBoardArea||CcX==GameBoardArea||CdX==GameBoardArea||CdrX==GameBoardArea)return 'x';
		    if(CY==GameBoardArea||CcY==GameBoardArea||CdY==GameBoardArea||CdrY==GameBoardArea)return 'o';
				
	 }
		 if(Full) return 'q';
	   return 0;
}

void GameStarting(char gt){
	
	 char i,j;
	CurrentPostion=0; 
	GameBoardArea=gt;
	game_element=(GameBoardArea*GameBoardArea-1);
	done=1;
	 player=0; 
	 for(i=0;i<gt;i++){
		 for(j=0;j<gt;j++)
		 BoardArray[i][j]=0;
	 }
}


 void set_Cursor() {
	 char posX,posY;
	 if( BoardArray[CurrentPostion/GameBoardArea][CurrentPostion%GameBoardArea]==0){
		        posX=((CurrentPostion%GameBoardArea)*4);
				    posY=((CurrentPostion/GameBoardArea)*2);
						BoardArray[CurrentPostion/GameBoardArea][CurrentPostion%GameBoardArea]='_';
						Nokia5110_SetCursor(posX,posY);
			  	  Nokia5110_OutChar('_');
		        Nokia5110_SetCursor(posX,posY);
						}
		 
	 }
	 void remove_Cursor(){
		 char posX,posY;
		 if(BoardArray[CurrentPostion/GameBoardArea][CurrentPostion%GameBoardArea]=='_'){
			    posX=((CurrentPostion%GameBoardArea)*4);
				  posY=((CurrentPostion/GameBoardArea)*2);
					BoardArray[CurrentPostion/GameBoardArea][CurrentPostion%GameBoardArea]=0;
					Nokia5110_SetCursor(posX,posY);
			  	Nokia5110_OutChar(' ');
					
					
				}
		 
	 }



int main(void){
	
	 unsigned long RightMove,LeftMove,Set_X_O;
	 TExaS_Init(SSI0_Real_Nokia5110_Scope);  // set system clock to 80 MHz
	 Random_Init(1);
 	 Nokia5110_Init();
	 Nokia5110_ClearBuffer();
	 Nokia5110_DisplayBuffer();      // draw buffer
	
	
	 
	
	
	 PortF_Init();
	 PORTE_INIT();
	    
	
	Nokia5110_DisplayBuffer();
					
	
	restart: 
	TheWinner=0;
	 //Delay100ms(2);
	 Nokia5110_Clear();
	
	 GameStarting(start()); 
	 Nokia5110_Clear();
	 Delay100ms(1);
	 CreateGame();
	 set_Cursor();
	 Nokia5110_SetCursor(0,0);
	
	
 
while(1){
		 RightMove = GPIO_PORTE_DATA_R&0x01;     // reading PF4 into RightMove
     LeftMove = GPIO_PORTE_DATA_R&0x02;     // reading PF0 into LeftMove
		 Set_X_O = GPIO_PORTF_DATA_R&0x10;     // reading PE0 into Set_X_O
		
	
			
			if((RightMove)){
				
        remove_Cursor();				
				
				CurrentPostion++;
				if(CurrentPostion>game_element)
				 CurrentPostion=game_element;
    	   
				while((GPIO_PORTE_DATA_R&0x01));
				  set_Cursor();
			}
				
			if((LeftMove))
			{
				remove_Cursor();
				
				CurrentPostion--;
				if(CurrentPostion<0)CurrentPostion=0;
				while((GPIO_PORTE_DATA_R&0x02));
				
				  set_Cursor();
				
				  			
				
				
			}
			if(!(Set_X_O)){
				while(!(GPIO_PORTF_DATA_R&0x10));
				if(!(player)){
					if(BoardArray[CurrentPostion/GameBoardArea][CurrentPostion%GameBoardArea]=='_')
					{
				  	Nokia5110_OutChar('X');
				  	BoardArray[CurrentPostion/GameBoardArea][CurrentPostion%GameBoardArea]='X';
						
						// increment cursor
						CurrentPostion++;
					  if(CurrentPostion>game_element)CurrentPostion=game_element;
						
						set_Cursor();
						player^=1;
					}
					
				}
					
				else{
					if(BoardArray[CurrentPostion/GameBoardArea][CurrentPostion%GameBoardArea]=='_')
						{
							Nokia5110_OutChar('O');
							BoardArray[CurrentPostion/GameBoardArea][CurrentPostion%GameBoardArea]='O';
							
							// increment cursor
							CurrentPostion++;
							if(CurrentPostion>game_element)CurrentPostion=game_element;

							set_Cursor();
						
							player^=1;
					}
				}
				TheWinner=WinnerChecker();
				
				
			}
			if(TheWinner){
				if(TheWinner=='x') {
					if(done){
					Nokia5110_Clear();
					Nokia5110_SetCursor(0,2);
					Nokia5110_OutString("player X Win");
					done=0;
					Delay100ms(4);
					Nokia5110_Clear();
					Nokia5110_SetCursor(0,3);
					Delay100ms(1);
					Nokia5110_OutString("Restarting");
					Delay100ms(1);
					Nokia5110_SetCursor(0,3);
					Nokia5110_OutString("Restarting.");
					Delay100ms(1);
					Nokia5110_SetCursor(0,3);
					Nokia5110_OutString("Restarting..");
					Delay100ms(1);
					Nokia5110_SetCursor(0,3);
					Nokia5110_OutString("Restarting...");
					Delay100ms(2);
					goto restart;
					}
				}
				if(TheWinner=='o') {
					if(done){
					Nokia5110_Clear();
					Nokia5110_SetCursor(0,2);
					Nokia5110_OutString("player O Win");
					done=0;
					Delay100ms(4);
					Nokia5110_Clear();
					Nokia5110_SetCursor(0,3);
					Delay100ms(1);
					Nokia5110_OutString("Restarting");
					Delay100ms(1);
					Nokia5110_SetCursor(0,3);
					Nokia5110_OutString("Restarting.");
					Delay100ms(1);
					Nokia5110_SetCursor(0,3);
					Nokia5110_OutString("Restarting..");
					Delay100ms(1);
					Nokia5110_SetCursor(0,3);
					Nokia5110_OutString("Restarting...");
					Delay100ms(2);
					goto restart;
					}
				}
				if(TheWinner=='q') {
					if(done){
					Nokia5110_Clear();
					Nokia5110_SetCursor(0,2);
					Nokia5110_OutString("Draw");
					Delay100ms(4);
					Nokia5110_Clear();
					Nokia5110_SetCursor(0,3);
					Delay100ms(1);
					Nokia5110_OutString("Restarting");
					Delay100ms(1);
					Nokia5110_SetCursor(0,3);
					Nokia5110_OutString("Restarting.");
					Delay100ms(1);
					Nokia5110_SetCursor(0,3);
					Nokia5110_OutString("Restarting..");
					Delay100ms(1);
					Nokia5110_SetCursor(0,3);
					Nokia5110_OutString("Restarting...");
					Delay100ms(2);
					goto restart;
					}
				}
				
			}
			
		
			
		
  }

}


void Delay100ms(unsigned long count){unsigned long volatile time;
  while(count>0){
    time = 727240;  // 0.1sec at 80 MHz
    while(time){
	  	time--;
    }
    count--;
  }
}
