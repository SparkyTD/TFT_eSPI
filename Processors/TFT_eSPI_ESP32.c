        ////////////////////////////////////////////////////
        // TFT_eSPI driver functions for ESP32 processors //
        ////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////
// Global variables
////////////////////////////////////////////////////////////////////////////////////////

#include <SPI.h>

// Select the SPI port to use, ESP32 has 2 options
#if !defined (TFT_PARALLEL_8_BIT)
    //SPIClass& spi = SPI;
    //SPIClass spi = SPIClass(VSPI);
    SPIClass *spi = new SPIClass(FSPI); // FSPI works with 35, 36, 37
    uint8_t pin_cs = -1;
    uint8_t pin_dc = -1;
    uint8_t pin_reset = -1;
#endif

#ifdef ESP32_DMA
  // DMA SPA handle
  spi_device_handle_t dmaHAL;
  #ifdef USE_HSPI_PORT
    spi_host_device_t spi_host = HSPI_HOST;
  #else
    spi_host_device_t spi_host = VSPI_HOST;
  #endif
#endif

#if !defined (TFT_PARALLEL_8_BIT)
  // Volatile for register reads:
  volatile uint32_t* _spi_cmd       = (volatile uint32_t*)(SPI_CMD_REG(SPI_PORT));
  volatile uint32_t* _spi_user      = (volatile uint32_t*)(SPI_USER_REG(SPI_PORT));
  // Register writes only:
  volatile uint32_t* _spi_mosi_dlen = (volatile uint32_t*)(SPI_MOSI_DLEN_REG(SPI_PORT));
  volatile uint32_t* _spi_w         = (volatile uint32_t*)(SPI_W0_REG(SPI_PORT));
#endif

////////////////////////////////////////////////////////////////////////////////////////
#if defined (TFT_SDA_READ) && !defined (TFT_PARALLEL_8_BIT)
////////////////////////////////////////////////////////////////////////////////////////

/***************************************************************************************
** Function name:           beginSDA
** Description:             Detach SPI from pin to permit software SPI
***************************************************************************************/
void TFT_eSPI::begin_SDA_Read(void)
{
  pinMatrixOutDetach(TFT_MOSI, false, false);
  pinMode(TFT_MOSI, INPUT);
  pinMatrixInAttach(TFT_MOSI, VSPIQ_IN_IDX, false);
  SET_BUS_READ_MODE;
}

/***************************************************************************************
** Function name:           endSDA
** Description:             Attach SPI pins after software SPI
***************************************************************************************/
void TFT_eSPI::end_SDA_Read(void)
{
  pinMode(TFT_MOSI, OUTPUT);
  pinMatrixOutAttach(TFT_MOSI, VSPID_OUT_IDX, false, false);
  pinMode(TFT_MISO, INPUT);
  pinMatrixInAttach(TFT_MISO, VSPIQ_IN_IDX, false);
  SET_BUS_WRITE_MODE;
}
////////////////////////////////////////////////////////////////////////////////////////
#endif // #if defined (TFT_SDA_READ)
////////////////////////////////////////////////////////////////////////////////////////


/***************************************************************************************
** Function name:           read byte  - supports class functions
** Description:             Read a byte from ESP32 8 bit data port
***************************************************************************************/
// Parallel bus MUST be set to input before calling this function!
uint8_t TFT_eSPI::readByte(void)
{
  uint8_t b = 0xAA;

#if defined (TFT_PARALLEL_8_BIT)
  RD_L;
  uint32_t reg;           // Read all GPIO pins 0-31
  reg = gpio_input_get(); // Read three times to allow for bus access time
  reg = gpio_input_get();
  reg = gpio_input_get(); // Data should be stable now
  RD_H;

  // Check GPIO bits used and build value
  b  = (((reg>>TFT_D0)&1) << 0);
  b |= (((reg>>TFT_D1)&1) << 1);
  b |= (((reg>>TFT_D2)&1) << 2);
  b |= (((reg>>TFT_D3)&1) << 3);
  b |= (((reg>>TFT_D4)&1) << 4);
  b |= (((reg>>TFT_D5)&1) << 5);
  b |= (((reg>>TFT_D6)&1) << 6);
  b |= (((reg>>TFT_D7)&1) << 7);
#endif

  return b;
}

////////////////////////////////////////////////////////////////////////////////////////
#ifdef TFT_PARALLEL_8_BIT
////////////////////////////////////////////////////////////////////////////////////////

/***************************************************************************************
** Function name:           GPIO direction control  - supports class functions
** Description:             Set parallel bus to INPUT or OUTPUT
***************************************************************************************/
void TFT_eSPI::busDir(uint32_t mask, uint8_t mode)
{
  gpioMode(TFT_D0, mode);
  gpioMode(TFT_D1, mode);
  gpioMode(TFT_D2, mode);
  gpioMode(TFT_D3, mode);
  gpioMode(TFT_D4, mode);
  gpioMode(TFT_D5, mode);
  gpioMode(TFT_D6, mode);
  gpioMode(TFT_D7, mode);
  return;
  /*
  // Arduino generic native function, but slower
  pinMode(TFT_D0, mode);
  pinMode(TFT_D1, mode);
  pinMode(TFT_D2, mode);
  pinMode(TFT_D3, mode);
  pinMode(TFT_D4, mode);
  pinMode(TFT_D5, mode);
  pinMode(TFT_D6, mode);
  pinMode(TFT_D7, mode);
  return; //*/
}

/***************************************************************************************
** Function name:           GPIO direction control  - supports class functions
** Description:             Set ESP32 GPIO pin to input or output (set high) ASAP
***************************************************************************************/
void TFT_eSPI::gpioMode(uint8_t gpio, uint8_t mode)
{
  if(mode == INPUT) GPIO.enable_w1tc = ((uint32_t)1 << gpio);
  else GPIO.enable_w1ts = ((uint32_t)1 << gpio);

  ESP_REG(DR_REG_IO_MUX_BASE + esp32_gpioMux[gpio].reg) // Register lookup
    = ((uint32_t)2 << FUN_DRV_S)                        // Set drive strength 2
    | (FUN_IE)                                          // Input enable
    | ((uint32_t)2 << MCU_SEL_S);                       // Function select 2
  GPIO.pin[gpio].val = 1;                               // Set pin HIGH
}
////////////////////////////////////////////////////////////////////////////////////////
#endif // #ifdef TFT_PARALLEL_8_BIT
////////////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////////////
#if defined (RPI_WRITE_STROBE) && !defined (TFT_PARALLEL_8_BIT) // Code for RPi TFT
////////////////////////////////////////////////////////////////////////////////////////

/***************************************************************************************
** Function name:           pushBlock - for ESP32 or ESP8266 RPi TFT
** Description:             Write a block of pixels of the same colour
***************************************************************************************/
void TFT_eSPI::pushBlock(uint16_t color, uint32_t len)
{
  uint8_t colorBin[] = { (uint8_t) (color >> 8), (uint8_t) color };
  if(len) spi.writePattern(&colorBin[0], 2, 1); len--;
  while(len--) {WR_L; WR_H;}
}

/***************************************************************************************
** Function name:           pushPixels - for ESP32 or ESP8266 RPi TFT
** Description:             Write a sequence of pixels
***************************************************************************************/
void TFT_eSPI::pushPixels(const void* data_in, uint32_t len)
{
  uint8_t *data = (uint8_t*)data_in;

  if(_swapBytes) {
      while ( len-- ) {tft_Write_16(*data); data++;}
      return;
  }

  while ( len >=64 ) {spi.writePattern(data, 64, 1); data += 64; len -= 64; }
  if (len) spi.writePattern(data, len, 1);
}

////////////////////////////////////////////////////////////////////////////////////////
#elif !defined (SPI_18BIT_DRIVER) && !defined (TFT_PARALLEL_8_BIT) // Most SPI displays
////////////////////////////////////////////////////////////////////////////////////////

/***************************************************************************************
** Function name:           pushBlock - for ESP32
** Description:             Write a block of pixels of the same colour
***************************************************************************************/
/*
void TFT_eSPI::pushBlock(uint16_t color, uint32_t len){
  
  uint32_t color32 = (color<<8 | color >>8)<<16 | (color<<8 | color >>8);
  bool empty = true;
  volatile uint32_t* spi_w = (volatile uint32_t*)_spi_w;
  if (len > 31)
  {
    *_spi_mosi_dlen =  511;
    spi_w[0]  = color32;
    spi_w[1]  = color32;
    spi_w[2]  = color32;
    spi_w[3]  = color32;
    spi_w[4]  = color32;
    spi_w[5]  = color32;
    spi_w[6]  = color32;
    spi_w[7]  = color32;
    spi_w[8]  = color32;
    spi_w[9]  = color32;
    spi_w[10] = color32;
    spi_w[11] = color32;
    spi_w[12] = color32;
    spi_w[13] = color32;
    spi_w[14] = color32;
    spi_w[15] = color32;
    while(len>31)
    {
      while ((*_spi_cmd)&SPI_USR);
      *_spi_cmd = SPI_USR;
      len -= 32;
    }
    empty = false;
  }

  if (len)
  {
    if(empty) {
      for (uint32_t i=0; i <= len; i+=2) *spi_w++ = color32;
    }
    len = (len << 4) - 1;
    while (*_spi_cmd&SPI_USR);
    *_spi_mosi_dlen = len;
    *_spi_cmd = SPI_USR;
  }
  while ((*_spi_cmd)&SPI_USR); // Move to later in code to use transmit time usefully?
}
//*/
//*
void TFT_eSPI::pushBlock(uint16_t color, uint32_t len){

  volatile uint32_t* spi_w = _spi_w;
  uint32_t color32 = (color<<8 | color >>8)<<16 | (color<<8 | color >>8);  
  uint32_t i = 0;
  uint32_t rem = len & 0x1F;
  len =  len - rem;

  // Start with partial buffer pixels
  if (rem)
  {
    while (*_spi_cmd&SPI_USR);
    for (i=0; i < rem; i+=2) *spi_w++ = color32;
    *_spi_mosi_dlen = (rem << 4) - 1;
    *_spi_cmd = SPI_USR;
    if (!len) return; //{while (*_spi_cmd&SPI_USR); return; }
    i = i>>1; while(i++<16) *spi_w++ = color32;
  }

  while (*_spi_cmd&SPI_USR);
  if (!rem) while (i++<16) *spi_w++ = color32;
  *_spi_mosi_dlen =  511;

  // End with full buffer to maximise useful time for downstream code
  while(len)
  {
    while (*_spi_cmd&SPI_USR);
    *_spi_cmd = SPI_USR;
      len -= 32;
  }

  // Do not wait here
  //while (*_spi_cmd&SPI_USR);
}
//*/
/***************************************************************************************
** Function name:           pushSwapBytePixels - for ESP32
** Description:             Write a sequence of pixels with swapped bytes
***************************************************************************************/
void TFT_eSPI::pushSwapBytePixels(const void* data_in, uint32_t len){
  uint8_t* data = (uint8_t*)data_in;
  uint32_t color[16];

  if (len > 31)
  {
    WRITE_PERI_REG(SPI_MOSI_DLEN_REG(SPI_PORT), 511);
    while(len>31)
    {
      uint32_t i = 0;
      while(i<16)
      {
        color[i++] = DAT8TO32(data);
        data+=4;
      }
      while (READ_PERI_REG(SPI_CMD_REG(SPI_PORT))&SPI_USR);
      WRITE_PERI_REG(SPI_W0_REG(SPI_PORT),  color[0]); 
      WRITE_PERI_REG(SPI_W1_REG(SPI_PORT),  color[1]);
      WRITE_PERI_REG(SPI_W2_REG(SPI_PORT),  color[2]);
      WRITE_PERI_REG(SPI_W3_REG(SPI_PORT),  color[3]);
      WRITE_PERI_REG(SPI_W4_REG(SPI_PORT),  color[4]);
      WRITE_PERI_REG(SPI_W5_REG(SPI_PORT),  color[5]);
      WRITE_PERI_REG(SPI_W6_REG(SPI_PORT),  color[6]);
      WRITE_PERI_REG(SPI_W7_REG(SPI_PORT),  color[7]);
      WRITE_PERI_REG(SPI_W8_REG(SPI_PORT),  color[8]);
      WRITE_PERI_REG(SPI_W9_REG(SPI_PORT),  color[9]);
      WRITE_PERI_REG(SPI_W10_REG(SPI_PORT), color[10]);
      WRITE_PERI_REG(SPI_W11_REG(SPI_PORT), color[11]);
      WRITE_PERI_REG(SPI_W12_REG(SPI_PORT), color[12]);
      WRITE_PERI_REG(SPI_W13_REG(SPI_PORT), color[13]);
      WRITE_PERI_REG(SPI_W14_REG(SPI_PORT), color[14]);
      WRITE_PERI_REG(SPI_W15_REG(SPI_PORT), color[15]);
      SET_PERI_REG_MASK(SPI_CMD_REG(SPI_PORT), SPI_USR);
      len -= 32;
    }
  }

  if (len > 15)
  {
    uint32_t i = 0;
    while(i<8)
    {
      color[i++] = DAT8TO32(data);
      data+=4;
    }
    while (READ_PERI_REG(SPI_CMD_REG(SPI_PORT))&SPI_USR);
    WRITE_PERI_REG(SPI_MOSI_DLEN_REG(SPI_PORT), 255);
    WRITE_PERI_REG(SPI_W0_REG(SPI_PORT),  color[0]); 
    WRITE_PERI_REG(SPI_W1_REG(SPI_PORT),  color[1]);
    WRITE_PERI_REG(SPI_W2_REG(SPI_PORT),  color[2]);
    WRITE_PERI_REG(SPI_W3_REG(SPI_PORT),  color[3]);
    WRITE_PERI_REG(SPI_W4_REG(SPI_PORT),  color[4]);
    WRITE_PERI_REG(SPI_W5_REG(SPI_PORT),  color[5]);
    WRITE_PERI_REG(SPI_W6_REG(SPI_PORT),  color[6]);
    WRITE_PERI_REG(SPI_W7_REG(SPI_PORT),  color[7]);
    SET_PERI_REG_MASK(SPI_CMD_REG(SPI_PORT), SPI_USR);
    len -= 16;
  }

  if (len)
  {
    while (READ_PERI_REG(SPI_CMD_REG(SPI_PORT))&SPI_USR);
    WRITE_PERI_REG(SPI_MOSI_DLEN_REG(SPI_PORT), (len << 4) - 1);
    for (uint32_t i=0; i <= (len<<1); i+=4) {
      WRITE_PERI_REG(SPI_W0_REG(SPI_PORT)+i, DAT8TO32(data)); data+=4;
    }
    SET_PERI_REG_MASK(SPI_CMD_REG(SPI_PORT), SPI_USR);
  }
  while (READ_PERI_REG(SPI_CMD_REG(SPI_PORT))&SPI_USR);

}

/***************************************************************************************
** Function name:           pushPixels - for ESP32
** Description:             Write a sequence of pixels
***************************************************************************************/
void TFT_eSPI::pushPixels(const void* data_in, uint32_t len){

  if(_swapBytes) {
    pushSwapBytePixels(data_in, len);
    return;
  }

  uint32_t *data = (uint32_t*)data_in;

  if (len > 31)
  {
    WRITE_PERI_REG(SPI_MOSI_DLEN_REG(SPI_PORT), 511);
    while(len>31)
    {
      while (READ_PERI_REG(SPI_CMD_REG(SPI_PORT))&SPI_USR);
      WRITE_PERI_REG(SPI_W0_REG(SPI_PORT),  *data++);
      WRITE_PERI_REG(SPI_W1_REG(SPI_PORT),  *data++);
      WRITE_PERI_REG(SPI_W2_REG(SPI_PORT),  *data++);
      WRITE_PERI_REG(SPI_W3_REG(SPI_PORT),  *data++);
      WRITE_PERI_REG(SPI_W4_REG(SPI_PORT),  *data++);
      WRITE_PERI_REG(SPI_W5_REG(SPI_PORT),  *data++);
      WRITE_PERI_REG(SPI_W6_REG(SPI_PORT),  *data++);
      WRITE_PERI_REG(SPI_W7_REG(SPI_PORT),  *data++);
      WRITE_PERI_REG(SPI_W8_REG(SPI_PORT),  *data++);
      WRITE_PERI_REG(SPI_W9_REG(SPI_PORT),  *data++);
      WRITE_PERI_REG(SPI_W10_REG(SPI_PORT), *data++);
      WRITE_PERI_REG(SPI_W11_REG(SPI_PORT), *data++);
      WRITE_PERI_REG(SPI_W12_REG(SPI_PORT), *data++);
      WRITE_PERI_REG(SPI_W13_REG(SPI_PORT), *data++);
      WRITE_PERI_REG(SPI_W14_REG(SPI_PORT), *data++);
      WRITE_PERI_REG(SPI_W15_REG(SPI_PORT), *data++);
      SET_PERI_REG_MASK(SPI_CMD_REG(SPI_PORT), SPI_USR);
      len -= 32;
    }
  }

  if (len)
  {
    while (READ_PERI_REG(SPI_CMD_REG(SPI_PORT))&SPI_USR);
    WRITE_PERI_REG(SPI_MOSI_DLEN_REG(SPI_PORT), (len << 4) - 1);
    for (uint32_t i=0; i <= (len<<1); i+=4) WRITE_PERI_REG((SPI_W0_REG(SPI_PORT) + i), *data++);
    SET_PERI_REG_MASK(SPI_CMD_REG(SPI_PORT), SPI_USR);
  }
  while (READ_PERI_REG(SPI_CMD_REG(SPI_PORT))&SPI_USR);
}

#endif // End of display interface specific functions

