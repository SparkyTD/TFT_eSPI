        ////////////////////////////////////////////////////
        // TFT_eSPI driver functions for ESP32 processors //
        ////////////////////////////////////////////////////

#ifndef _TFT_eSPI_ESP32H_
#define _TFT_eSPI_ESP32H_

// Processor ID reported by getSetup()
#define PROCESSOR_ID 0x32

// Include processor specific header
#include "soc/spi_reg.h"
#include "driver/spi_master.h"

// SUPPORT_TRANSACTIONS is mandatory for ESP32 so the hal mutex is toggled
#if !defined (SUPPORT_TRANSACTIONS)
  #define SUPPORT_TRANSACTIONS
#endif

// ESP32 specific SPI port selection
#ifdef USE_HSPI_PORT
  #define SPI_PORT HSPI
#else
  #define SPI_PORT VSPI
#endif

#ifdef RPI_DISPLAY_TYPE
  #define CMD_BITS (16-1)
#else
  #define CMD_BITS (8-1)
#endif

// Initialise processor specific SPI functions, used by init()
#define INIT_TFT_DATA_BUS // Not used

// Define a generic flag for 8 bit parallel
#if defined (ESP32_PARALLEL) // Specific to ESP32 for backwards compatibility
  #if !defined (TFT_PARALLEL_8_BIT)
    #define TFT_PARALLEL_8_BIT // Generic parallel flag
  #endif
#endif

// Ensure ESP32 specific flag is defined for 8 bit parallel
#if defined (TFT_PARALLEL_8_BIT)
  #if !defined (ESP32_PARALLEL)
    #define ESP32_PARALLEL
  #endif
#endif

// Processor specific code used by SPI bus transaction startWrite and endWrite functions
#if !defined (ESP32_PARALLEL)
  #if (TFT_SPI_MODE == SPI_MODE1) || (TFT_SPI_MODE == SPI_MODE2)
    #define SET_BUS_WRITE_MODE *_spi_user = SPI_USR_MOSI | SPI_CK_OUT_EDGE
    #define SET_BUS_READ_MODE  *_spi_user = SPI_USR_MOSI | SPI_USR_MISO | SPI_DOUTDIN | SPI_CK_OUT_EDGE
  #else
    #define SET_BUS_WRITE_MODE *_spi_user = SPI_USR_MOSI
    #define SET_BUS_READ_MODE  *_spi_user = SPI_USR_MOSI | SPI_USR_MISO | SPI_DOUTDIN
  #endif
#else
    // Not applicable to parallel bus
    #define SET_BUS_WRITE_MODE
    #define SET_BUS_READ_MODE
#endif

// Code to check if DMA is busy, used by SPI bus transaction transaction and endWrite functions
#if !defined(TFT_PARALLEL_8_BIT) && !defined(SPI_18BIT_DRIVER)
  #define ESP32_DMA
  // Code to check if DMA is busy, used by SPI DMA + transaction + endWrite functions
  #define DMA_BUSY_CHECK  dmaWait()
#else
  #define DMA_BUSY_CHECK
#endif

#if defined(TFT_PARALLEL_8_BIT)
  #define SPI_BUSY_CHECK
#else
  #define SPI_BUSY_CHECK while (*_spi_cmd&SPI_USR)
#endif

// If smooth font is used then it is likely SPIFFS will be needed
#ifdef SMOOTH_FONT
  // Call up the SPIFFS (SPI FLASH Filing System) for the anti-aliased fonts
  #define FS_NO_GLOBALS
  #include <FS.h>
  #include "SPIFFS.h" // ESP32 only
  #define FONT_FS_AVAILABLE
#endif

////////////////////////////////////////////////////////////////////////////////////////
// Define the DC (TFT Data/Command or Register Select (RS))pin drive code
////////////////////////////////////////////////////////////////////////////////////////
#define DC_C GPIO.out_w1tc = (1 << pin_dc)//;GPIO.out_w1tc = (1 << TFT_DC)
#define DC_D GPIO.out_w1ts = (1 << pin_dc)//;GPIO.out_w1ts = (1 << TFT_DC)

////////////////////////////////////////////////////////////////////////////////////////
// Define the CS (TFT chip select) pin drive code
////////////////////////////////////////////////////////////////////////////////////////
#define CS_L GPIO.out_w1tc = (1 << pin_cs); GPIO.out_w1tc = (1 << pin_cs)
#define CS_H GPIO.out_w1ts = (1 << pin_cs)//;GPIO.out_w1ts = (1 << pin_cs)

////////////////////////////////////////////////////////////////////////////////////////
// Define the WR (TFT Write) pin drive code
////////////////////////////////////////////////////////////////////////////////////////
#define WR_L
#define WR_H

////////////////////////////////////////////////////////////////////////////////////////
// Define the touch screen chip select pin drive code
////////////////////////////////////////////////////////////////////////////////////////
#define T_CS_L // No macro allocated so it generates no code
#define T_CS_H // No macro allocated so it generates no code

// Replacement slimmer macros
  #define TFT_WRITE_BITS(D, B) *_spi_mosi_dlen = B-1;    \
                               *_spi_w = D;             \
                               *_spi_cmd = SPI_USR;      \
                        while (*_spi_cmd & SPI_USR);

  // Write 8 bits
  #define tft_Write_8(C) TFT_WRITE_BITS(C, 8)

  // Write 16 bits with corrected endianess for 16 bit colours
  #define tft_Write_16(C) TFT_WRITE_BITS((C)<<8 | (C)>>8, 16)

  // Future option for transfer without wait
  #define tft_Write_16N(C) *_spi_mosi_dlen = 16-1;       \
                           *_spi_w = ((C)<<8 | (C)>>8); \
                           *_spi_cmd = SPI_USR;

  // Write 16 bits
  #define tft_Write_16S(C) TFT_WRITE_BITS(C, 16)

  // Write 32 bits
  #define tft_Write_32(C) TFT_WRITE_BITS(C, 32)

  // Write two address coordinates
  #define tft_Write_32C(C,D)  TFT_WRITE_BITS((uint16_t)((D)<<8 | (D)>>8)<<16 | (uint16_t)((C)<<8 | (C)>>8), 32)

  // Write same value twice
  #define tft_Write_32D(C) TFT_WRITE_BITS((uint16_t)((C)<<8 | (C)>>8)<<16 | (uint16_t)((C)<<8 | (C)>>8), 32)

////////////////////////////////////////////////////////////////////////////////////////
// Macros to read from display using SPI or software SPI
////////////////////////////////////////////////////////////////////////////////////////
#if !defined (TFT_PARALLEL_8_BIT)
  // Read from display using SPI or software SPI
  // Use a SPI read transfer
  #define tft_Read_8() spi->transfer(0)
#endif

// Concatenate a byte sequence A,B,C,D to CDAB, P is a uint8_t pointer
#define DAT8TO32(P) ( (uint32_t)P[0]<<8 | P[1] | P[2]<<24 | P[3]<<16 )

#endif // Header end
