/*******************************************************************************
* EECE344 Lab 5: Basic Security System where 'A' is Armed and 'D' is Disarmed
* 				 when in the armed state the LED's on the K65 board with blink
* 				 signifying that it is now armed. Also in the armed state the
* 				 touch sensors are now active and if touched they will activate
* 				 the alarm system and have the alarm wave be sent out and the
* 				 LED corresponding to the pad will now blink faster. Pressing
* 				 'D' will disarm the Alarm.
*
*12/3/2021
*Andy Nguyen
*Todd Morton
*******************************************************************************/

 //Include project header file//
#include "MCUType.h"
#include "BasicIO.h"
#include "K65TWR_ClkCfg.h"
#include "Key.h"
#include "LCD.h"
#include "SysTickDelay.h"
#include "K65TWR_GPIO.h"
#include "MemoryTools.h"
#include "AlarmWave.h"
#include "MK65F18.h"
#include"K65TWR_TSI.h"

#define SLICE_PER 10             /* 10ms time slice period  */
#define COL 1
#define ROW 1
#define ROW2 2

#define HIGH 0x001FFFFF
#define LOW 0x00000000



static void lab5ControlTask(void);
static void LEDTask(void);
static void SensorScanTask(void);

const INT8U clear = 1;
const INT8U num_nib = 4;
static INT8U firstTime = 1;

static INT8U LEDsetMode;
static INT16U FlagSensor;
static INT8U SensorTrigger_1;
static INT8U SensorTrigger_2;
static INT8C Cur_Key;
static INT8U Wait = 0;
static INT8U AlarmFlag = 0;
static INT8U Count = 0;

// statics const for LCD display messages //
 INT8C CS_M[] = "CheckSum: 0x";

 INT8C Armed[] = "ARMED";
 INT8C Disarmed[] = "DISARMED";
 INT8C Alarm[] = "ALARM";

typedef enum {ARMED,DISARMED,ALARM} UISTATE_T;                  //create states for the state machine
UISTATE_T AlarmState = DISARMED;

typedef enum{LED_ON,LED_OFF} LEDMODE_T;
LEDMODE_T LEDState = LED_OFF;

void main (void){

	K65TWR_BootClock();				//k65 Clock init
    GpioDBugBitsInit();
    SysTickDlyInit();
    KeyInit();
    LcdDispInit();
    AlarmWaveInit();
    GpioLED8Init();
    GpioLED9Init();
    TSIInit();


     // check sum pointers //
     INT8U *high_addr = (INT8U *)0x001FFFFF;		//high address for check sum
     INT8U *low_addr = (INT8U *)0x00000000;			//low address for check sum
     INT16U check_sum = 0;


 	// checksum on LCD //
 	check_sum = MemChkSum(low_addr, high_addr); //check sum


 	LcdDispClear();								// clear disp
 	LcdCursorMove(ROW2,COL);					// shifting row and col
 	LcdDispString(CS_M);						// disp pre-written message

    LcdDispHexWord((INT32U)check_sum, num_nib);


    ///////////////////////////////////////////////////////////////////

	/*create time scheduler for the kernel*/
    while(1){                               /* Endless Time Slice cyclic scheduler  */

        SysTickWaitEvent(SLICE_PER);        //DebugBit 0
        lab5ControlTask();					//DebugBit 1
        KeyTask();							//DebugBit 2
        TSITask();
        SensorScanTask();					//DebugBit 3
        LEDTask();							//DebugBit 4


    }

}

/******************************************************************************
*lab5ControlTask(void) - This Enables the Alarm and Disables the Alarm

******************************************************************************/
static void lab5ControlTask(void){


	    DB1_TURN_ON();													// setting Debug bits

	    Cur_Key = KeyGet();

			switch (AlarmState){

			case (DISARMED):


				if(firstTime == 1){

					firstTime = 0;				//setting the check to 0 to say its checked
					LcdCursorMove(ROW,COL);
					LcdDispString(Disarmed);
					AlarmWaveSetMode(0);

					}


				if(Cur_Key == DC1){						// A pressed

					LcdDispLineClear(1);
					AlarmState = ARMED;
					firstTime = 1;						//reseting check

					}

				else {

				}

			break;


			case (ARMED):


				if(firstTime == 1){

						firstTime = 0;
						LcdCursorMove(ROW,COL);							// shifting row and col to display on bottom row
						LcdDispString(Armed);							// display message string
						AlarmWaveSetMode(0);

				}


					if(Cur_Key == DC4){

						LcdDispLineClear(1);							// clearing line
						AlarmState = DISARMED;
						firstTime = 1;
					}

			break;

			case (ALARM):


				if(Wait == 49){						//waits ever 500ms
					Wait = 0;
					if(AlarmFlag == 1){				// if flag is enable , enable the alarm
					AlarmFlag = 0;					// reset flag
					AlarmWaveSetMode(1);

					LcdDispLineClear(1);			//displays alarm message
					LcdCursorMove(ROW,COL);
					LcdDispString(Alarm);
					}

					else {
						AlarmFlag = 1;
						AlarmWaveSetMode(0);
					}
				}

				else{
					Wait++;
				}


				if(Cur_Key == DC4){

					LcdDispLineClear(1);							// clearing line
					AlarmState = DISARMED;
					firstTime = 1;
				}

		break;


				default:
					LcdDispLineClear(1);
					AlarmState = DISARMED;

				break;


		}
		    DB1_TURN_OFF();

    }



/******************************************************************************
*void SensorScanTask(void) - This controls the sensors for pad one and two
* 							 on the K65 board, also tells the LED to turn on/off
* 							 if the pads were touched.

******************************************************************************/
void SensorScanTask(void){


	DB3_TURN_ON();



	FlagSensor = TSIGetSensorFlags();

	    if(AlarmState == DISARMED){

	    	SensorTrigger_1 = 0;
	    	SensorTrigger_2 = 0;


	        if((FlagSensor & (1<<BRD_PAD1_CH)) != 0){
	          	 LEDsetMode = 1;
	             LEDState = LED_ON;

	        }

	        else if (((FlagSensor & (1<<BRD_PAD2_CH)) != 0)){
	        	  LEDsetMode = 2;
	              LEDState = LED_ON;

	        }
	        else{
	        	  LEDState = LED_OFF;

	        }
	    }

	    else{
	    }


	    if((AlarmState == ARMED) || (AlarmState == ALARM)){

	        if((FlagSensor & (1<<BRD_PAD1_CH)) != 0){
	        	SensorTrigger_1 = 1;
	        	LEDsetMode = 3;
	            AlarmState = ALARM;

	        }

	        else if(((FlagSensor & (1<<BRD_PAD2_CH)) != 0)){
	        	SensorTrigger_2 = 1;
	        	LEDsetMode = 4;
	            AlarmState = ALARM;

	        }

	        else{
	        	LEDState = LED_ON;
	        }
	    }

	    else{
	    }



	DB3_TURN_OFF();

}



/******************************************************************************
*void LEDTask(void) - This handles the LED. this is an independent function
*		   that can be called and wont be effected by any other task. turns
*		   on and off LED 8  and LED 9 on K65 board. This disables the LEDs
*		   when in the 'Disarmed' state and when in the 'Armed' state blinks
*		   the LED's at 250ms each back and forth.

******************************************************************************/
void LEDTask(void){


	DB4_TURN_ON();


	 switch(LEDState){

	    case LED_ON:
	        if((AlarmState == DISARMED) && (LEDsetMode == 1)){       // checking if pad 1 was touched

	            if(Count <= 25){                                    // 250ms

	               LED8_TURN_ON();
	               LED9_TURN_OFF();
	            }

	            else if((Count >= 25) && (Count <= 50)){

	               LED8_TURN_OFF();
	               LED9_TURN_OFF();
	            }
	            else if(Count > 50){

	               Count = 0;
	            }

	        }
	        else{
	        	LEDState = LED_OFF;
	         }


	        if((AlarmState == DISARMED) && (LEDsetMode == 2)){       // checking if pad 2 was touched

	            if(Count <= 25){                                     // 250ms count

	               LED9_TURN_ON();
	               LED8_TURN_OFF();

	            }
	            else if((Count >= 25) && (Count <= 50)){

	               LED9_TURN_OFF();
	               LED8_TURN_OFF();

	            }
	            else if(Count > 50){
	               Count = 0;
	            }

	        }
	        else{
	        	LEDState = LED_OFF;
	         }
	        if(AlarmState == ALARM){      							    // Pad 1 check

	            if (LEDsetMode == 3){

	                if(Count <= 5){                                     // 50ms count
	                   LED8_TURN_ON();
	                }
	                else if((Count >= 5) && (Count <= 10)){
	                   LED8_TURN_OFF();
	                }
	                else if(Count > 10){
	                   Count = 0;
	                }
	                else{
	                }
	            }
	            else{
	            }

	            if (LEDsetMode == 4){

	                if(Count <= 5){                                      // 50ms
	                LED9_TURN_ON();
	                }
	                else if((Count >= 5) && (Count <= 10)){

	                LED9_TURN_OFF();
	                }
	                else if(Count > 10){
	                Count = 0;
	                }
	                else{
	                }

	            }
	            else{
	            }


	             if ((SensorTrigger_1 & SensorTrigger_2) == 1){			// if both sensors are triggered
	                if(Count <= 5){                                     // 50ms -- pad touched then led turns on
	                   LED9_TURN_ON();
	                   LED8_TURN_ON();
	                }
	                else if((Count >= 5) && (Count <= 10)){
	                   LED8_TURN_OFF();
	                   LED9_TURN_OFF();
	                }
	                else if(Count > 10){
	                   Count = 0;
	               }
	                else{
	                }
	            }
	             else{
	             }
	        }


	        if(AlarmState == ARMED){

	            if(Count <= 25){                                    // blinking in the armed state  -- > 250ms
	               LED8_TURN_ON();
	               LED9_TURN_OFF();
	            }
	            else if((Count >= 25) && (Count <= 50)){			// blinks led 8 and 9 back and forth
	               LED9_TURN_ON();
	               LED8_TURN_OFF();
	            }
	            else if(Count > 10){								// if the count is less then 100ms the in gets reset to 0
	               Count = 0;
	            }
	        }

	        else{
	        	LEDState = LED_OFF;
	        }
	        break;


	    case LED_OFF:				//LED off state

	        LED8_TURN_OFF();
	        LED9_TURN_OFF();

	        break;
	    default:

	        break;
	    }

	    Count++;

	DB4_TURN_OFF();
}






