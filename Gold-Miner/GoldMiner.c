//GoldMiner.c
//An adaptation of Reel Gold Miniclip
//Marzooq Shah and Shriya Vijay
//Date Modified: Apr 13, 2022

// ******* Possible Hardware I/O connections*******************
// Slide pot pin 1 connected to ground
// Slide pot pin 2 connected to PD2/AIN5
// Slide pot pin 3 connected to +3.3V 
// buttons connected to PE0-PE3
// 32*R resistor DAC bit 0 on PB0 (least significant bit)
// 16*R resistor DAC bit 1 on PB1
// 8*R resistor DAC bit 2 on PB2  
// 4*R resistor DAC bit 3 on PB3
// 2*R resistor DAC bit 4 on PB4
// 1*R resistor DAC bit 5 on PB5 (most significant bit)
// LED on PD1
// LED on PD0


#include <stdint.h>
#include "../inc/tm4c123gh6pm.h"
#include "ST7735.h"
#include "Print.h"
#include "Random.h"
#include "TExaS.h"
#include "ADC.h"
#include "Images.h"
#include "Sound.h"
#include "Timer1.h"
#include "EdgeInterrupt.h"
#include "SysTick.h"
#include "Timer2.h"



#define ABS(x) (x<0)?(-x):x
#define isGold 1
#define InitGrapY 54
#define TRUE 1
#define FALSE 0
#define ENDOFSCREEN 140
#define NOCOLLISION 0
#define ROCKCOLLISION 1
#define GOLDCOLLISION 2




uint8_t NumRock =0;  //Number of Rock+Gold
uint8_t NumGold=0;		//Number of Gold

uint16_t data = 0;	 //Data from ADC

uint8_t flag = 0;		//Flag used to poll the timer 
uint8_t flag2 =3;		//3 => First time after collision
uint8_t grapFlag =0;//Flag used to see what the grappler is supposed to be doing

uint8_t time = 30;	//Time on the clock

uint8_t gold_id=0;	//What gold is colliding.
uint8_t id=0;				//What rock/gold sprite is the collision looking for
//uint16_t gameScore=0;		//Score = 0

//Language Phrashes go here

typedef enum {English, Spanish} Language_t;
Language_t myLanguage;

typedef enum {HELLO, GOODBYE, TIME, SCORE, LEVEL, COMPLETE, OVER} phrase_t;
//phrase_t phrase;

const char Hello_English[] ="Hello";
const char Hello_Spanish[] ="\xADHola!";
const char Goodbye_English[]="Goodbye";
const char Goodbye_Spanish[]="Adi\xA2s";
const char Time_English[]="Time:";
const char Time_Spanish[]="Tiempo:";
const char Score_English[]="Score:";
const char Score_Spanish[]="Puntaje:";
const char Level_English[]="Level:";
const char Level_Spanish[]="Nivel:";
const char Complete_English[]="Complete!";
const char Complete_Spanish[]="Completo!";
const char Over_English[]="Game Over";	
const char Over_Spanish[]="Juego Terminado";	

const char *Phrases[7][2]={
  {Hello_English,Hello_Spanish},
  {Goodbye_English,Goodbye_Spanish},
  {Time_English,Time_Spanish},
	{Score_English, Score_Spanish},
	{Level_English, Level_Spanish},
	{Complete_English, Complete_Spanish},
	{Over_English, Over_Spanish}
};







//Structs go here

//Rock/Gold sprite
struct rockgoldSprite {

	uint8_t x,y;
	uint8_t w, h;
	uint16_t centerX, centerY;
	const uint16_t *image;
	uint8_t gold;
 } ;
typedef struct rockgoldSprite rock_t;

rock_t rock[7];				//Have a maximum of 7 rock/gold sprites at once
 
 //Cart: the main sprite moves horizontally
struct cart{

	uint8_t x,y;
	uint8_t xOld;
	uint8_t w, h;
	const uint16_t *image;
};
typedef struct cart cart_t;

cart_t YCart = {0,37,0,30,36,ycart};


//The grappler that grabs the gold sprites
struct grappler1 {

	uint8_t x,y;
	uint8_t xOld;
	uint8_t dy;			//Speed of grappler
	uint8_t w, h;
	uint16_t centerX, centerY;
	const uint16_t *image;
};
typedef struct grappler1 grappler1_t;

grappler1_t grap1 = {0,InitGrapY,0,10,15,18,0,0,OpenGrap};				//Regular grappler open mouth
grappler1_t grap2 = {0,0,0,10,16,24,0,0,GoldGrap2};								//Grappler with closed mouth and gold sprite attached to it



void DisableInterrupts(void); // Disable interrupts
void EnableInterrupts(void);  // Enable interrupts
void Delay100ms(uint32_t count); // time delay in 0.1 seconds
void MoveCart(void);//Moves the top cart
uint8_t Collision(rock_t rockTest); //Checks for collision with gold or rock
void GrapplerMove(void); //Moves the Grappler down
void spawn(uint8_t);					// Sets up the first screen
void GrapplerMoveUp(void);	//Moves up grappler after collision with rock or end of screen
void GrapplerMoveUpGold(void);	//Moves the grappler up with gold once it collides with gold sprite
void timeDisplay(void);						//Handler that decrements time (called every one second )
void printTime(uint8_t timer);		//Prints current time
void printScore(uint16_t yu);		//Prints score
void Welcome(void);		//Welcome display to chose language and stuff

void DeleteGold(rock_t rocki);
void LevelComplete(uint8_t lev, uint32_t sco);
void GameOver(uint32_t score);


void Timer1A_Handler(void){ // can be used to perform tasks in background
  
	TIMER1_ICR_R = TIMER_ICR_TATOCINT;// acknowledge TIMER1A timeout
	
	data = ADC_In();
	flag = 1;
	
}

// your function to convert 12 bit ADC sample to cart position on screen
// input: x is 12 bit ADC digital sample
// output: x-cordinate of cart on screen.
uint32_t Convert(uint32_t x){
  // write this
	uint16_t result;
	
	result = ((107*x)/4096);
  return result;
  
}



int main(void){
		uint32_t gameScore=0;
		uint8_t timerReduced=40;
		uint8_t endGame =FALSE;
		uint8_t nextLevel = FALSE;
		uint8_t level=1;
		uint8_t checkCollision=0;			//Checks Collision
		//uint8_t nextLevel = FALSE;   
		DisableInterrupts();
		TExaS_Init(NONE);       // Bus clock is 80 MHz 
		Output_Init();
		ADC_Init();
		Timer1_Init(8000000/10,3);		//10Hz sampling
		EdgeTrigger_Init();
		Sound_Init();
		SysTick_Init();
		Timer2_Init(*timeDisplay, 80000000);			//Calles timeDisplay every second
	
		EnableInterrupts();
	
		Welcome();																	//Allows you to chose between languages
	
		ST7735_FillScreen(ST7735_BLACK);            // set screen to black
		
		
	
		
		
		
		while(1){
			
			while(endGame==FALSE){
				time = timerReduced;
				spawn(level);																		//Loads up the initial sprites.
				time = timerReduced;
				while(nextLevel==FALSE && endGame==FALSE){		
					
					
					if (flag==1 && grapFlag==0) { MoveCart();}							//Call move cart if the flag is set i.e the data is sampled and grappler is not moving
				
				
				
					if (grapFlag==1) {																		//grapFlag==1 means the PE0 button was pressed to move grappler
						
			
						uint8_t i=0;
						while(checkCollision== NOCOLLISION && i<NumRock){	//Call Collision until there is a collision or all the sprites are checked for collision
						
							checkCollision =Collision(rock[i]);							//Check for Collision.
							
							id =i;					//id keeps track of what rock/gold sprite is being tested for collision
							i++;
						}
							
						
					
						
					
						if(checkCollision==NOCOLLISION && grap1.y < ENDOFSCREEN){			//If there is no collision and end of screen is not reached
							SysTick_Wait100ms(1);	//TEST
							GrapplerMove();																					//Move Grappler down and update grappler y co-ordinate
					
						}	else{grapFlag=2;}			//grapFlag 2 would mean either collision or end of screen
						
						
						
					
					}	
					
					if(grapFlag==2){					//grapFlag is 2 when collision or end of screen is hit, move grappler up now.
					
					
						if(checkCollision==ROCKCOLLISION || checkCollision==NOCOLLISION){				//If a Rock was hit or screen bottom is hit
					
							if(grap1.y!=InitGrapY){														//If Grappler is not back to initial position
								SysTick_Wait100ms(1);	//TEST
								GrapplerMoveUp();															//Move Grappler up and update grappler y position.
							}else{
							grapFlag=0;													//Grappler is back at place so the button press fiasco is over
							checkCollision=0;	
								
							}
						}
						
						if(checkCollision==GOLDCOLLISION){								//If a Gold Sprite was hit.
							
							if(flag2==3){
								
								DeleteGold(rock[id]);
								flag2=0;
							}
							if((grap2.y>InitGrapY)&&flag2==1){							//Wait for switch to be pressed
								
								GrapplerMoveUpGold();													//Moves up Grappler with gold
								
							} 
							if(grap2.y<=InitGrapY){										//else if Grappler is it's initial length
								//Sound_Stop();
								checkCollision=0;
								Sound_Gold();
								grap1.y = InitGrapY;
								rock[id]=rock[NumRock -1];
								NumRock--;																		//There is one less Rock now
								Sound_Gold();																	//Play sound of recieving the gold
								grapFlag=0;																	//Grapler is all the way up
								gameScore+= 100;																//For getting the gold up
								
								NumGold--;
								flag2=3;																		//Cuz im stupid like that sometimes
								printScore(gameScore);
							}
						
						}
					
				
					
					}
					
					//printScore(gameScore);
					printTime(time);
					
					if(time==0){endGame=TRUE;}
					if(NumGold==0){nextLevel=TRUE;}
					//SysTick_Wait100ms(1);	
			

				}
				if(NumGold==0){
					LevelComplete(level,gameScore);
					level++;
					timerReduced= timerReduced -5;
					nextLevel=FALSE;
					ST7735_FillScreen(ST7735_BLACK);
				}
			
			}
		
		GameOver(gameScore);		
	}
}


//MoveCart: Moves the cart when the slidepot has been moved
void MoveCart(void){									
			
			YCart.x	= Convert(data);
			grap1.x = YCart.x + 9;					//Width of the cart is 20 so grap1.x is positioned below the centre of the cart
		
			if(YCart.x != YCart.xOld){
				ST7735_FillRect(YCart.xOld, YCart.y - YCart.h, YCart.w, YCart.h+1, ST7735_BLACK);
			}
			if(grap1.x != grap1.xOld){
				ST7735_FillRect(grap1.xOld, grap1.y - grap1.h, grap1.w, grap1.h+1, ST7735_BLACK);
			}
			
			ST7735_DrawBitmap(YCart.x, YCart.y, YCart.image, YCart.w, YCart.h); //Draw Cart
			
			ST7735_DrawBitmap(grap1.x, grap1.y, grap1.image, grap1.w, grap1.h); //Draw grappler
			
			YCart.xOld = YCart.x;
			grap1.xOld = grap1.x;
		
			flag = 0;

}

//Moves Grappler down and updates grappler y co-ordinate
void GrapplerMove(void){

		
		grap1.y += grap1.dy;

		ST7735_DrawBitmap(grap1.x, grap1.y, grap1.image, grap1.w, grap1.h); //Draw grappler 
		
}

void GPIOPortE_Handler(void){
  
			
			

	if((GPIO_PORTE_RIS_R&0x01)==1){					//PE0 interrupt sw1
	
			static uint32_t last=0;
				uint32_t now = GPIO_PORTE_DATA_R&0x01;
			
			if((now==1)&&(last==0)){
				//RisingEdge
				grapFlag =1;
			
			}
			if((now==0)&&(last==1)){
			//FallingEdge
			
			}
			
			if (now==1){
			//Button held
				grapFlag=1;
				
			
			}
			last = now;
	
	}
	
	if(((GPIO_PORTE_RIS_R&0x02)>>1)==1){					//PF4 interrupt sw2
		
			static uint32_t last2=0;
			uint32_t now2 = (GPIO_PORTE_DATA_R&0x02)>>1; 
		
			
			
			if((now2==1)&&(last2==0)){
				//RisingEdge
				flag2=1;
 			
			}
			if((now2==0)&&(last2==1)){
			//FallingEdge
				
			}
			
			if (now2==1){
			//Button held
				flag2=1;
				
			}
			last2 = now2;
	
	}
	
	GPIO_PORTE_ICR_R = 0x03;      // acknowledge flag4
	
	
	
}


//Collision: Input => Rock/Gold Sprite
//           Output=> returns 0 for no collision and 1 for rock collision and 2 for gold collision

uint8_t Collision(rock_t rockTest){

		
		
		
	rockTest.centerX = rockTest.x + (rockTest.w/2);
	rockTest.centerY = rockTest.y - (rockTest.h/2);
	
	grap1.centerX = grap1.x + (grap1.w/2);
	
	
	int16_t distance1, distance2;
	distance1 = rockTest.centerX - grap1.centerX;
	distance2 =  rockTest.centerY - grap1.y;
	distance1 = ABS(distance1);
	distance2 = ABS(distance2);


	
		if((distance2 < 14)&&(distance1 < 8)){ 
		//Check if collision is gold
		if(rockTest.gold == isGold){
			
			
			return GOLDCOLLISION;
			}
		
		Sound_Rock();												//rock is hit make sound
		return ROCKCOLLISION;
	 	
		}
	
	return NOCOLLISION;
	
	

}

void GrapplerMoveUp(void){

		
			
	
		ST7735_FillRect(grap1.x, grap1.y - grap1.h, grap1.w, grap1.h+1, ST7735_BLACK); //Erase old Grappler
			
		grap1.y -= grap1.dy;						//Update position of grappler
		ST7735_DrawBitmap(grap1.x, grap1.y, grap1.image, grap1.w, grap1.h); //Draw grappler witd updated position
		
		

		
}

//Deletes Rock and Initializes grappler with gold co-ordinates
void DeleteGold(rock_t rock){

	
		ST7735_FillRect(rock.x, rock.y - rock.h, rock.w, rock.h+1, ST7735_BLACK); //Delete the Gold Sprite after collision
		
		
		grap2.y = rock.y;
		grap2.x = grap1.x;
	
		ST7735_DrawBitmap(grap2.x, grap2.y, grap2.image, grap2.w, grap2.h); //Draw grappler with Gold
		
}

//Moves up grappler with gold
void GrapplerMoveUpGold(void){
	
	

		
		
		
	
		
		ST7735_DrawBitmap(grap2.x, grap2.y, grap2.image, grap2.w, grap2.h); //Draw grappler with Gold
		SysTick_Wait100ms(2);	//TEST
		
	  ST7735_FillRect(grap2.x, grap2.y - grap2.h, grap2.w, grap2.h+1, ST7735_BLACK); //Delete the old Trace of Grappler with Gold	

		grap2.y -= grap2.dy;		//Grappler with gold moves up
			
			
			
		
		
}

void timeDisplay(void){
			if(time!=0){
			time--;
			}
}

void printTime(uint8_t timer){
	
			if (time<10){ST7735_FillRect(90,142,120,160,ST7735_BLACK);}
			ST7735_SetCursor(18,15);
			LCD_OutDec(timer);

}

void printScore(uint16_t yu){
			

			ST7735_SetCursor(6,15);
			LCD_OutDec(yu);

}
void spawn(uint8_t level){
			
			
			 ST7735_SetCursor(0,15);
			ST7735_OutString((char*)Phrases[SCORE][myLanguage]);					//Have a Score counter on bottom left
	
			
			ST7735_SetCursor(10 ,15);
			ST7735_OutString((char*)Phrases[TIME][myLanguage]);					//Have a timer counter on bottom right
			
			
	
			NumRock = 3;					
			NumGold = 2;
			
			rock[0]=(rock_t){20,75,16,11,0,0,Stone2,0};					//Special way to initialize
			rock[1]=(rock_t){85,88,16,11,0,0,Gold2,isGold};
			rock[2]=(rock_t){40,110,16,11,0,0,Gold2,isGold};
			
			if(level>=2){
			
			rock[3]=(rock_t){70,80,16,11,0,0,Stone2,0};					//Special way to initialize
			rock[4]=(rock_t){100,100,16,11,0,0,Gold2,isGold};	
			
			NumRock=5;
			NumGold=3;
			
			}
			
			if(level>=3){
			
			rock[5]=(rock_t){120,70,16,11,0,0,Stone2,0};					//Special way to initialize
			rock[6]=(rock_t){45,130,16,11,0,0,Gold2,isGold};	
			
			NumRock=7;
			NumGold=4;
			
			}
	

			for(uint8_t i =0; i<NumRock; i++){
			ST7735_DrawBitmap(rock[i].x, rock[i].y, rock[i].image, rock[i].w, rock[i].h); 
			}

}

void Welcome(void){

		
		ST7735_FillScreen(ST7735_WHITE);
		
		ST7735_SetCursor(7,7);
		ST7735_OutString("Gold Miner");
		
		ST7735_DrawBitmap(0,140,Eng,60,24);
		ST7735_DrawBitmap(60,140,Span2,60,18);
	
		SysTick_Wait100ms(2);
	

while(grapFlag==0)	{
		uint16_t lang = Convert(data);
		
		if(lang<54){
		
			ST7735_DrawBitmap(0,140,EngCircled,60,24);
			ST7735_DrawBitmap(60,140,Span2,60,24);
			myLanguage = English;
		
		}
		
		else if(lang>=54){
			
			ST7735_DrawBitmap(60,140,Span2Circled,60,24);
			ST7735_DrawBitmap(0,140,Eng,60,24);
			myLanguage = Spanish;
			
		}
		
	}

		grapFlag=0;

}

void LevelComplete(uint8_t lev,uint32_t sco){
	
			ST7735_FillScreen(ST7735_BLACK);
	
			ST7735_SetCursor(7,7);
			ST7735_OutString((char*)Phrases[LEVEL][myLanguage]);
			
			ST7735_SetCursor(13,7);
			ST7735_OutUDec(lev);
	
			ST7735_SetCursor(5,10);
			ST7735_OutString((char*)Phrases[SCORE][myLanguage]);
			
			ST7735_SetCursor(13,10);
			ST7735_OutUDec(sco);
			
			while(grapFlag==0){}
				grapFlag=0;

}

void GameOver(uint32_t sco){
			
			ST7735_FillScreen(ST7735_BLACK);
			
			ST7735_SetCursor(3,7);
			ST7735_OutString((char*)Phrases[OVER][myLanguage]);
		
			ST7735_SetCursor(7,10);
			ST7735_OutString((char*)Phrases[SCORE][myLanguage]);
			
			ST7735_SetCursor(12,10);
			ST7735_OutUDec(sco);
	
			while(1){}

}




