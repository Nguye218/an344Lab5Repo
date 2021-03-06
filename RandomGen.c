/****************************************************************************************
* RandomGen.c - Some random stuff based on the Kinetis RNGA module.
* 11/02/2016 TDM
*****************************************************************************************
* Project master header file
****************************************************************************************/
#include "MCUType.h"
#include "RandomGen.h"
/****************************************************************************************
* Private Resources
****************************************************************************************/

/****************************************************************************************
* Module Defines
****************************************************************************************/

/****************************************************************************************
* RNGInit() - Turns on the RNGA without an error interrupt.
* - Public
****************************************************************************************/
void RNGInit(void){
    SIM->SCGC3 |= SIM_SCGC3_RNGA(1);
    RNG->CR |= RNG_CR_INTM(1)|RNG_CR_GO(1);
}
/****************************************************************************************
* RNGGet() - Returns a random number from the RNGA. Blocks based on OREG_LVL
****************************************************************************************/
INT32U RNGGet(void){
    while((RNG->SR & RNG_SR_OREG_LVL_MASK) == 0x00){}
    return RNG->OR;
}
