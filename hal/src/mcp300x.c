#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>

#include "hal/devconf.h"

#include "hal/spi.h"
#include "hal/mcp300x.h"

#ifdef SPIDEV_0_TRF0
extern int SPI0_0_FD;

extern uint8_t TG0_RXBUFF[SPIDEV_0_TRF_BUFFER_SIZE];
extern uint8_t TG0_TXBUFF[SPIDEV_0_TRF_BUFFER_SIZE];
#endif

#ifdef SPIDEV_1_TRF1
extern int SPI0_1_FD;
extern uint8_t TG1_RXBUFF[SPIDEV_1_TRF_BUFFER_SIZE];
extern uint8_t TG1_TXBUFF[SPIDEV_1_TRF_BUFFER_SIZE];
#endif

// RAW ADC value, need to be converted to voltage based on Vref
unsigned int ADCResult = 0;
uint8_t* tx;
uint8_t* rx;

// ADC Vref, Changeable via extern or setVref()
float ADCVref = 1;

/*
Setup SPI device initialization. Takes spidev*_* for ADC SPI configuration.
Opens the device on call, then sets up a pointer for data transaction
*/


enum ADCBus ADC_TRFBUS;

int mcpInit(enum ADCBus transferBus) {
  ADC_TRFBUS = transferBus;
  switch (transferBus) {
    case ADC_SPIDEV0_0: 
      #ifdef SPIDEV_0_TRF0
        tx = TG0_TXBUFF; 
        rx = TG0_RXBUFF;
      #endif

      break;

    case ADC_SPIDEV0_1:
      #ifdef SPIDEV_1_TRF1
        tx = TG1_TXBUFF;
        rx = TG1_RXBUFF;
      #endif
      break;

    default:
      return -1;
  }
  return 0;
}

/*
Begin Device sampling. Checks if spidev is initialized and starts ADC sampling based
on the seleceted channel. (Ref to mcpChannel selection).

Once spi transaction is completed, values are saved to the ADCResult, then the voltage
can be retrived via mcpGetResult() based on Vref.
*/
int mcpSample(enum mcpChannel channel){
  tx[0] = (uint8_t)(0x06 | ((channel & 0x04) >> 2));
  tx[1] = (uint8_t)((channel & 0x03) << 6);
  tx[2] = 0x00;

  char r = -1;
  switch (ADC_TRFBUS) {
    case ADC_SPIDEV0_0:       
      #ifdef SPIDEV_0_TRF0
        r = spi0_0_TRANSFER();
      #endif
      break;

    case ADC_SPIDEV0_1:
        #ifdef SPIDEV_1_TRF1
        r = spi0_1_TRANSFER();
      #endif
      break;
    default:
      break;
  }

  if(r != 0){
      fprintf(stderr, "SPI device transfer Failed...\n");
      return -1;
  }

  ADCResult = ((rx[1] & 0x0F) << 8) | rx[2];
  return ADCResult;
}

/*
Returns ADC voltage based on the sampled data and Vref
*/
float mcpGetResult(void){
  return (ADCVref * ADCResult) / 4095.0;
}

// Set the ADC reference voltage
void setVref(float vref){
  ADCVref = vref;
}

float getVref(){
  return ADCVref;
}
