#ifndef _MCP300X_H_
#define _MCP300X_H_

#include <stdint.h>

// ADC input selection
enum mcpChannel{
  SINGLE_CH0 = 0x08,
  SINGLE_CH1 = 0x09,
  SINGLE_CH2 = 0x0A,
  SINGLE_CH3 = 0x0B,
  SINGLE_CH4 = 0x0C,
  SINGLE_CH5 = 0x0D,
  SINGLE_CH6 = 0x0E,
  SINGLE_CH7 = 0x0F,

  DIFF_CH0_1 = 0x00,
  DIFF_CH1_0 = 0x01,
  DIFF_CH2_3 = 0x02,
  DIFF_CH3_2 = 0x03,
  DIFF_CH4_5 = 0x04,
  DIFF_CH5_4 = 0x05,
  DIFF_CH6_7 = 0x06,
  DIFF_CH7_6 = 0x07
};

// Set the ADC reference voltage
void setVref(float vref);
float getVref();


/*
Setup SPI device initialization. Takes spidev*_* for ADC SPI configuration.
Opens the device on call, then sets up a pointer for data transaction

Possability to add Vref Setting here
*/
enum ADCBus{
  ADC_SPIDEV0_0,
  ADC_SPIDEV0_1
};

int mcpInit(enum ADCBus transferBus);

/*
Begin Device sampling. Checks if spidev is initialized and starts ADC sampling based
on the seleceted channel. (Ref to mcpChannel selection).

Once spi transaction is completed, values are saved to the ADCResult, then the voltage
can be retrived via mcpGetResult() based on Vref.
*/
int mcpSample(enum mcpChannel channel);

/*
Returns ADC voltage based on the sampled data and Vref
*/
float mcpGetResult(void);

#endif