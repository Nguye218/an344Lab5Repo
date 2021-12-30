/*******************************************************************************
* EECE344 Lab 2
* MemChkSum() - Fills a block of memory with fillsum
*
*
* Andy Nguyen, 10/18/2021
* Todd Morton, 9/21/2020
*******************************************************************************/


#include "MCUType.h"               /* Include header files                    */
#include "MemoryTools.h"


INT16U MemChkSum(INT8U *startaddr, INT8U *endaddr){


	INT16U fillsum = 0;
	INT8U data =0;
	INT8U over_flow = 0;



	 while(startaddr <= endaddr && over_flow == 0){
		 data = *startaddr;
		 	 	 fillsum += (INT16U)data;
		         /*check overflow*/
		         if(startaddr == (INT8U *)0xFFFFFFFFU){
		             over_flow = 1;
		         }
		         else{
		             startaddr++;
		         }
		     }
		     return fillsum;

}
