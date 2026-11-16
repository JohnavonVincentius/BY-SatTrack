#include <sys/ioctl.h>
#include <linux/spi/spidev.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdint.h>
#include <unistd.h>

#include <hal/devconf.h>

#ifdef SPIDEV_0_TRF0
int SPI0_0_FD = -1;

uint8_t TG0_RXBUFF[SPIDEV_0_TRF_BUFFER_SIZE] = {0};
uint8_t TG0_TXBUFF[SPIDEV_0_TRF_BUFFER_SIZE] = {0};

struct spi_ioc_transfer TG0 = {
    .rx_buf = (uint64_t)(uintptr_t)TG0_RXBUFF, 
    .tx_buf = (uint64_t)(uintptr_t)TG0_TXBUFF,
    .len = sizeof(TG0_TXBUFF),
    .speed_hz = SPIDEV_0_TRF_CLK_SPEED,
    .bits_per_word = SPIDEV_0_TRF_BITS,
};
#endif

#ifdef SPIDEV_1_TRF1
int SPI0_1_FD;

uint8_t TG1_RXBUFF[SPIDEV_1_TRF_BUFFER_SIZE] = {0};
uint8_t TG1_TXBUFF[SPIDEV_1_TRF_BUFFER_SIZE] = {0};

struct spi_ioc_transfer TG1 = {
    .rx_buf = (uint64_t)(uintptr_t)TG1_RXBUFF, 
    .tx_buf = (uint64_t)(uintptr_t)TG1_TXBUFF,
    .len = sizeof(TG1_TXBUFF),
    .speed_hz = SPIDEV_1_TRF_CLK_SPEED,
    .bits_per_word = SPIDEV_1_TRF_BITS,
};

#endif

#ifdef SPIDEV_0_TRF0
int spi0_0_INIT() {
  SPI0_0_FD = open(SPIDEV_0_TRF0, O_RDWR);
  if (SPI0_0_FD < 0) {
      perror("Failed to open SPI device");
      return -1;
  }

  uint8_t mode = SPIDEV_0_TRF_MODE;
  uint8_t bits = SPIDEV_0_TRF_BITS;
  uint32_t speed = SPIDEV_0_TRF_CLK_SPEED;

  if (ioctl(SPI0_0_FD, SPI_IOC_WR_MODE, &mode) < 0) {
      perror("Failed to set SPI mode");
      close(SPI0_0_FD);
      return -1;
  }

  if (ioctl(SPI0_0_FD, SPI_IOC_WR_BITS_PER_WORD, &bits) < 0) {
      perror("Failed to set bits per word");
      close(SPI0_0_FD);
      return -1;
  }

  if (ioctl(SPI0_0_FD, SPI_IOC_WR_MAX_SPEED_HZ, &speed) < 0) {
      perror("Failed to set max speed");
      close(SPI0_0_FD);
      return -1;
  }
  return 0;
}

int spi0_0_TRANSFER(){
    if (ioctl(SPI0_0_FD, SPI_IOC_MESSAGE(1), &TG0) < 1) {
    perror("Failed to perform SPI transaction");
    return -1;
  }
  return 0;
}
#endif

#ifdef SPIDEV_1_TRF1
int spi0_1_INIT() {
  SPI0_1_FD = open(SPIDEV_1_TRF1, O_RDWR);
  if (SPI0_1_FD < 0) {
      perror("Failed to open SPI device");
      return -1;
  }

  uint8_t mode = SPIDEV_1_TRF_MODE;
  uint8_t bits = SPIDEV_1_TRF_BITS;
  uint32_t speed = SPIDEV_1_TRF_CLK_SPEED;

  if (ioctl(SPI0_1_FD, SPI_IOC_WR_MODE, &mode) < 0) {
      perror("Failed to set SPI mode");
      close(SPI0_1_FD);
      return -1;
  }

  if (ioctl(SPI0_1_FD, SPI_IOC_WR_BITS_PER_WORD, &bits) < 0) {
      perror("Failed to set bits per word");
      close(SPI0_1_FD);
      return -1;
  }

  if (ioctl(SPI0_1_FD, SPI_IOC_WR_MAX_SPEED_HZ, &speed) < 0) {
      perror("Failed to set max speed");
      close(SPI0_1_FD);
      return -1;
  }
  return 0;
}

int spi0_1_TRANSFER(){
    if (ioctl(SPI0_1_FD, SPI_IOC_MESSAGE(1), &TG1) < 1) {
    perror("Failed to perform SPI transaction");
    return -1;
  }
  return 0;
}
#endif


void spiEnd() {
    #ifdef SPIDEV_0_TRF0
  if (SPI0_0_FD >= 0) { /* Added safety NULL check for the pointer itself */
      close(SPI0_0_FD);
      SPI0_0_FD = -1;
  }  
    #endif
    
  #ifdef SPIDEV_1_TRF1
  if (SPI0_1_FD >= 0) { /* Added safety NULL check for the pointer itself */
      close(SPI0_1_FD);
      SPI0_1_FD = -1;
  }
  #endif
}