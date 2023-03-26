/*
 *@author Andrew Darby
 * SWENG 452 
 * Final Project
 * Simon Memory Game
 * Implemented on Raspberry Pi and Dual Breadboards
 * 12/04/2022
 * 
 */
#include <wiringPi.h>
#include <stdio.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <lcd.h>
#include <sys/time.h>
#include <wiringShift.h>
#include <signal.h>
#include <unistd.h>


#define RedButtonPin	0
#define BlueButtonPin 	1
#define RedLedPin 		2
#define BlueLedPin		3
#define GreenButtonPin  4
#define YellowButtonPin 5
#define GreenLedPin     6
#define YellowLedPin    7
#define   SDI   21   		//serial data input
#define   RCLK  22   		//memory clock input(STCP)
#define   SRCLK 23   		//shift register clock input(SHCP)


unsigned char SegCode[17] = {0x3f,0x06,0x5b,0x4f,0x66,0x6d,0x7d,0x07,0x7f,0x6f,0x77,0x7c,0x39,0x5e,0x79,0x71,0x80}; // store values for 7 segment display
const int MAXROUND = 20;
int roundNum = 1;			// global variable to keep track of round of play
int score = 0;				// global variable to track score
int hiScore = 0;			// global variable to track hi score
int counter = 0;


// method prototypes
void timer(int timer1);
void loop();
int displaySequence(int gameSeq[]);		// display sequence to player 
void introSequence();		// display random light sequence intro
int userInput(int gameSeq[], int round);
void createSeq(int gameSeq[], int round, int led);
void init(void);			// initialize all Pi pins
void hc595_shift(unsigned char dat);	// method to shift input for 7 seg display
void clearDisplay();
void countDown();


//int gameSequence[20];		// array of integers to hold sequence of flashes 	
// int values for buttons to store in array red = 1, blue = 2, green = 3, yellow = 4

int main(void)
{
	
	// When initialize wiring failed, print messageto screen
	if(wiringPiSetup() == -1){
		printf("setup wiringPi failed !");
		return 1; 
	}
	
	init();					//initialize all Pi pins
	clearDisplay();
	
	char gameOn = 'y';	 // char to hold y or n value to determine if player wants to continue playing 	
	int goOn; 			// int stores value for one of two ways to determine if main do while loop continues or end goOn == 1 game continues
						// goOn == 2, loop breaks
	int gameSeq[MAXROUND];				// array stores current values of led light sequence to be guessed					
	printf("Hello and welcome to Simon! The fast paced memory game you will love!\n");
	printf("Are you ready to begin? y/n: ");
	scanf("\n%c", &gameOn);
	printf("Follow Simon's sequence of lights, and use the corresponding colored buttons to enter your input.\n");
	introSequence();
	delay(2000);
	goOn = displaySequence(gameSeq);				// call initial startup of game 
	
	
	// main gameplay loop
	do {

		// displaySequence returns 2 , player entered incorrect sequence, game over
		if(goOn == 2)
		{
			printf("You lose! Game Over!\n");
			printf("Play again? y/n\n");
			scanf("\n%c", &gameOn);
			// if player choose n exit loop, program finished
			if(gameOn == 'n')
			{
				break;
				
			}
			// player chose to play another game 
			printf("Ok, let's play another game!\n"); //replay works to this point, then goes straight to game over
			score = 0;
			roundNum = 1;					// reset round back to round 1
			introSequence();			// dispaly intro sequence
			goOn = displaySequence(gameSeq);	// call displaySequence method to restart gameplay
		}
		// goOn == 1 means the player entered correct seqeuenc, score will increment and displaySequence is called for next round
		else if(goOn == 1)
		{
			score += 10;
			if(hiScore < score)
			{
				hiScore = score;
			}

			printf("Hi Score is: %d\n", hiScore);
			printf("Current Score is : %d\n", score);
			goOn = displaySequence(gameSeq);
		}
		
		
	}while(gameOn != 'n');
	printf("\nThank you for playing\n");
	
	
	
	return 0;
}

// method that runs through a random light sequence at the beginning of the game
void introSequence()
{
	printf("Simon will display a sample light sequence now: \n");
	unsigned seed = time(0);
	srand(seed);
	
	// for loop that creates intro led light up sequence 
	for(int i = 0; i < 8;i++)
	{
		int led = rand() % 4;
		if(led == 0)
		{
			digitalWrite(RedLedPin, LOW);
			delay(700);
			digitalWrite(RedLedPin, HIGH);
		}
		
		else if(led == 1)
		{
			digitalWrite(GreenLedPin, LOW);
			delay(700);
			digitalWrite(GreenLedPin, HIGH);
		}
		
		else if(led == 2)
		{
			digitalWrite(YellowLedPin, LOW);
			delay(700);
			digitalWrite(YellowLedPin, HIGH);
		}
		
		else 
		{
			digitalWrite(BlueLedPin, LOW);
			delay(700);
			digitalWrite(BlueLedPin, HIGH);
		}
		
	}	// end for loop
	
} // end method introSequence


// method to generate sequence to display
void createSeq(int gameSeq[], int round,int led)
{
	unsigned seed = time(0);
	srand(seed);
	
	// if round is 1, then will create our initial seq of 4 led flashes
	if(roundNum == 1)
	{
		//printf("Creating initial seq\n");
		for(int k = 0; k < 4; k++)
		{
			// red = 1, blue = 2, green = 3, yellow = 4
			led = rand() % 4;
			
			// if statements to create initial seq
			if(led == 0)
			{
				gameSeq[k] = 1;
				//printf("red ");	
			}
		
			else if(led == 1)
			{
				gameSeq[k] = 3;
				//printf("green ");
			}
		
			else if(led == 2)
			{	
				gameSeq[k] = 4;
				//printf("yellow ");
			}
		
			else 
			{
				gameSeq[k] = 2;
				//printf("blue ");
			}
			
		} // end for loop
	}// end if statement
	
	else if(roundNum > 1)			// else if round is greater than 1, will add 1 more led flash to the sequence
	{
		//printf("Adding single led to sequence\n");
		printf("Round Number = %d\n", roundNum);
		// red = 1, blue = 2, green = 3, yellow = 4
			 led = rand() % 4;
			
			// if statements to add additional led flash to every round after 1
			if(led == 0)
			{
				gameSeq[roundNum + 2] = 1;
				//printf("red ");	
			}
		
			else if(led == 1)
			{
				gameSeq[roundNum + 2] = 3;
				//printf("Green ");
			}
		
			else if(led == 2)
			{	
				gameSeq[roundNum + 2] = 4;
				//printf("yellow ");
			}
		
			else 
			{
				gameSeq[roundNum + 2] = 2;
				//printf("blue ");
			}
	}// end else if
	
} // end method createSeq

// method displays the sequence of lights that the player must guess
int displaySequence(int gameSeq[])
{
	printf("\nPrepare for the light sequence!\n");
	countDown();	
	hc595_shift(SegCode[roundNum]);
	
	
	int gameStatus = 0;					// keeps track if player guessed sequence or failed
	// red = 1, blue = 2, green = 3, yellow = 4
	int led = 0;
	
	createSeq(gameSeq, roundNum, led);
	
	
	// for loop displays light for user 
	for(int i = 0; i < roundNum + 3; i++)
	{

		led = gameSeq[i];
		// if statements to display leds in correct sequence
		if(led == 1)
		{
			digitalWrite(RedLedPin, LOW);
			//gameSeq[i] = 1;
			//printf("red\n");
			delay(1000);
			digitalWrite(RedLedPin, HIGH);
			delay(700);
		}
		
		else if(led == 3)
		{
			digitalWrite(GreenLedPin, LOW);
			//gameSeq[i] = 3;
			//printf("green \n");
			delay(1000);
			digitalWrite(GreenLedPin, HIGH);
			delay(700);
		}
		
		else if(led == 4)
		{
			digitalWrite(YellowLedPin, LOW);
			//gameSeq[i] = 4;
			//printf("yellow \n");
			delay(1000);
			digitalWrite(YellowLedPin, HIGH);
			delay(700);
		}
		
		else 
		{
			digitalWrite(BlueLedPin, LOW);
			//gameSeq[i] = 2;
			//printf("blue \n");
			delay(1000);
			digitalWrite(BlueLedPin, HIGH);
			delay(700);
		}
	
	} // end for loop 
	
	gameStatus = userInput(gameSeq, roundNum);
	roundNum++;
	return gameStatus;
}// end method displaySequence


// method receives current displayed sequence and intakes users button pushes/compares sequences as the user goes(will also add in timer)
int userInput(int gameSeq[], int roundNum)
{
	int count = 0;
	int userSeq[MAXROUND];
	unsigned seed = time(0);
	srand(seed);

	printf("Prepare to enter sequence\n");
	countDown();	
	
	// declare our start and end times
	time_t t1, t2;
	double diff; 
	time(&t1);
	double temp= 0;
	
	printf("You have 5 seconds to enter sequence...go!\n");
	// while loop reads in users input from buttons and stores in array
	while(count < roundNum + 3 && diff < 5)
	{	
		while(1)
		{
			
		// red = 1, blue = 2, green = 3, yellow = 4
		// Indicate that button has pressed down
			if(digitalRead(BlueButtonPin) == 0){
				// Led on
				digitalWrite(BlueLedPin, LOW);
				//printf("...blue button pressed\n");
				userSeq[count] = 2;
				//printf("gameSeq = %d and userSeq = %d\n", gameSeq[count], userSeq[count]);
				if (userSeq[count] != gameSeq[count])
				{
					digitalWrite(BlueLedPin, HIGH);
					return 2;
				}
				count++;
				delay(500);							
				digitalWrite(BlueLedPin, HIGH);
				time(&t2);
				
			}else if(digitalRead(RedButtonPin) == 0){
				digitalWrite(RedLedPin, LOW);
				//printf("...red button pressed\n");
				userSeq[count] = 1;
				//printf("gameSeq = %d and userSeq = %d\n", gameSeq[count], userSeq[count]);
				
				if (userSeq[count] != gameSeq[count])
				{
					digitalWrite(RedLedPin, HIGH);
					return 2;
				}
				//printf("red button has been pressed %d times\n", redCount);
				delay(500);
				count++;
				digitalWrite(RedLedPin, HIGH);
				time(&t2);
				
			}else if(digitalRead(GreenButtonPin) == 0){
				digitalWrite(GreenLedPin, LOW);
				//printf("...green button pressed\n");
				userSeq[count] = 3;
				//printf("gameSeq = %d and userSeq = %d\n", gameSeq[count], userSeq[count]);
				
				if (userSeq[count] != gameSeq[count])
				{
					digitalWrite(GreenLedPin, HIGH);
					return 2;
				}
				count++;
				
				delay(500);
				digitalWrite(GreenLedPin, HIGH);
				time(&t2);
				
			}else if(digitalRead(YellowButtonPin) == 0){
				digitalWrite(YellowLedPin, LOW);
				//printf("...yellow button pressed\n");
				userSeq[count] = 4;
				//printf("gameSeq = %d and userSeq = %d\n", gameSeq[count], userSeq[count]);
				//yellowCount++;
				
				if (userSeq[count] != gameSeq[count])
				{
					digitalWrite(YellowLedPin, HIGH);
					return 2;
				}
				count++;	
				delay(500);
				digitalWrite(YellowLedPin, HIGH);
				time(&t2);
			}
			delay(200);
			time(&t2);
			diff = difftime(t2, t1);
			temp = temp + diff;
			// if more than diff time expires, game is over
			if (diff > 10 )
			{
				printf("Time has expired\n");
				return 2;
			}
		
			// if count (of sequence) ==  the correct # in given sequence, stop accepting input and go to next round
			if (count == roundNum + 3)
			{
				break;
			}
			
		} // end inner while loop
	} // end outer while loop
	
	printf("Round complete... Well done!\n");
	return 1;
	
}// end method user user input

// method intializes all Pi components
void init(void)
{
	// set up the functioning of pins
	pinMode(BlueLedPin, OUTPUT);
	pinMode(RedLedPin, OUTPUT); 
	pinMode(GreenLedPin, OUTPUT); 
	pinMode(YellowLedPin, OUTPUT); 
	pinMode(BlueButtonPin, INPUT);
	pinMode(RedButtonPin, INPUT);
	pinMode(GreenButtonPin, INPUT);
	pinMode(YellowButtonPin, INPUT);
	// Pull up to 3.3V,make GPIO1 a stable level 
	pullUpDnControl(RedButtonPin, PUD_UP);
	pullUpDnControl(BlueButtonPin, PUD_UP);
	pullUpDnControl(GreenButtonPin, PUD_UP);
	pullUpDnControl(YellowButtonPin, PUD_UP);
    // set all LEDs to off to start 
	digitalWrite(BlueLedPin, HIGH);
	digitalWrite(RedLedPin, HIGH);
	digitalWrite(GreenLedPin, HIGH);
	digitalWrite(YellowLedPin, HIGH);
	
	pinMode(SDI, OUTPUT); //make P0 output
	pinMode(RCLK, OUTPUT); //make P0 output
	pinMode(SRCLK, OUTPUT); //make P0 output

	digitalWrite(SDI, 0);
	digitalWrite(RCLK, 0);
	digitalWrite(SRCLK, 0);
	
}// end method init

// method shifts hex input to send proper digit to 7 segment display
void hc595_shift(unsigned char dat){
	int i;

	for(i=0;i<8;i++){
		digitalWrite(SDI, 0x80 & (dat << i));
		digitalWrite(SRCLK, 1);
		delay(1);
		digitalWrite(SRCLK, 0);
	}

		digitalWrite(RCLK, 1);
		delay(1);
		digitalWrite(RCLK, 0);
} // end method hc595_shift

// method to clear 7 seg display
void clearDisplay()
{
    int i;
    for (i = 0; i < 8; i++)
    {
        digitalWrite(SDI, 1);
        digitalWrite(SRCLK, 1);
        delayMicroseconds(1);
        digitalWrite(SRCLK, 0);
    }
    digitalWrite(RCLK, 1);
    delayMicroseconds(1);
    digitalWrite(RCLK, 0);
}// end mehtod clearDisplay

// method shows countdown for user before they enter their answer
void countDown()
{
	for (int i = 3; i >= 0; i--)
	{
		
			hc595_shift(SegCode[i]);
			printf("%d\n", i);
			delay(1000);
		
	}
	
	//clearDisplay();
	//hc595_shift(SegCode[roundNum]);
}

