#ifndef _SPI_H_
#define _SPI_H_

#ifdef SPIDEV_0_TRF0
  int spi0_0_INIT();
  int spi0_0_TRANSFER();
#endif

#ifdef SPIDEV_1_TRF1
  int spi0_1_INIT();
  int spi0_1_TRANSFER();
#endif

void spiEnd(int* fd);

#endif