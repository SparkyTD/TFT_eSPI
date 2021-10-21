
// This is the command sequence that initialises the ST7735 driver
//
// This setup information is in a format accepted by the commandList() function
// which reduces FLASH space, but on an ESP8266 there is plenty available!
//
// See ILI9341_Setup.h file for an alternative simpler format


static const uint8_t PROGMEM InitCommands[] = {                  // Initialization commands for 7735B screens
        19,                        // 18 commands in list:

        ST7735_SWRESET,            // 1: Software reset, no args, w/delay
        TFT_INIT_DELAY, 50,    // 		50 ms delay

        ST7735_SLPOUT,            // 2: Out of sleep mode, no args, w/delay
        TFT_INIT_DELAY, 100,    // 		255 = 500 ms delay

        ST7735_GAMSET, 1,        // 3: Set default gamma
        0x04,                // 		16-bit newColor

        ST7735_FRMCTR1, 2,        // 4: Frame Rate
        0x0b,
        0x14,

        ST7735_PWCTR1, 2,        // 5: VRH1[4:0] & VC[2:0]
        0x08,
        0x00,

        ST7735_PWCTR2, 1,        // 6: BT[2:0]
        0x05,

        ST7735_VMCTR1, 2,        // 7: VMH[6:0] & VML[6:0]
        0x41,
        0x30,

        ST7735_DRVCTR, 1,        // 8: LCD Driving control
        0xc1,

        ST7735_NCOLFRQ, 1,        // 9: Set pumping newColor freq
        0x1b,

        ST7735_COLMOD,                // 10: Set newColor format
        1 + TFT_INIT_DELAY,
        0x55,                    //     	16-bit newColor
        100,                    // 		100ms delay

        ST7735_CASET, 4,            // 11: Set Column Address
        0x00,
        0x00,
        0x00,
        0x7f,

        ST7735_RASET, 4,            // 12: Set Page Address
        0x00,
        0x00,
        0x00,
        0x9f,

        ST7735_MADCTL, 1,           // 13: Set Scanning Direction
        0xc8,

        ST7735_DISSET6, 1,          // 14: Set Source Output Direciton
        0x00,

        ST7735_GAMMAEN, 1,          // 15: Enable Gamma bit
        0x00,

        ST7735_GMCTRP1,            // 16: magic
        15 + TFT_INIT_DELAY,
        0x28, 0x24, 0x22, 0x31,
        0x2b, 0x0e, 0x53, 0xa5,
        0x42, 0x16, 0x18, 0x12,
        0x1a, 0x14, 0x03,
        50,

        ST7735_GMCTRN1,            // 17: more magic
        15 + TFT_INIT_DELAY,
        0x17, 0x1b, 0x1d, 0x0e,
        0x14, 0x11, 0x2c, 0xa5,
        0x3d, 0x09, 0x27, 0x2d,
        0x25, 0x2b, 0x3c,
        50,

        ST7735_NORON,                // 18: Normal display on, no args, w/delay
        TFT_INIT_DELAY,
        10,                     // 10 ms delay
        ST7735_DISPON,                // 19: Main screen turn on, no args, w/delay
        TFT_INIT_DELAY,
        255                        // 255ms delay
};

