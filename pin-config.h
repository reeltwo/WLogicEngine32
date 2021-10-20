#ifndef PIN_CONFIG_h
#define PIN_CONFIG_h

////////////////////////////////
// WReactor32 Pin Out
// ------------------
// Should be no reason to change these unless you are using a different board
////////////////////////////////

#define SERIAL2_TX_PIN        0   // outputs PWM signal at boot Pull-up
#define SERIAL_TX_PIN         1   // debug output
#define BAT_READ_PIN          2   // Pull-Down
#define SERIAL_RX_PIN         3   // high at boot
#define SD_CS_PIN             4   // SD Card CS pin
#define SERIAL1_TX_PIN        5   // VSPI SS/CS
#define RESERVED_FLASH_6      6   // connected to SPI flash
#define RESERVED_FLASH_7      7   // connected to SPI flash
#define RESERVED_FLASH_8      8   // connected to SPI flash
#define RESERVED_FLASH_9      9   // connected to SPI flash
#define RESERVED_FLASH_10     10  // connected to SPI flash
#define RESERVED_FLASH_11     11  // connected to SPI flash
#define SPI_CS                12  // TDI - HSPI MISO (Boot fail if pulled high)
#define SERIAL2_RX_PIN        13  // TCK - HSPI MOSI
#define LED_STATUS_PIN        14  // TMS - HSPI SCK
#define FRONT_LOGIC_PIN       15  // HSPI SS/CS
#define RESERVED_GPIO_16      16  // PSRAM
#define RESERVED_GPIO_17      17  // PSRAM
#define SPI_SCK               18  // VSPI SCK
#define SPI_MISO              19  // VSPI MISO
#define NONEXISTENT_PIN_20    20
#define I2C_SDA               21
#define I2C_SCL               22
#define SPI_MOSI              23  // VSPI MOSI
#define NONEXISTENT_PIN_24    24
#define I2S_DOUT              25  // I2S audio output
#define I2S_LRC               26  // I2S audio output
#define I2S_BCLK              27  // I2S audio output
#define NONEXISTENT_PIN_28    28
#define NONEXISTENT_PIN_29    29
#define NONEXISTENT_PIN_30    30
#define NONEXISTENT_PIN_31    31
#define REAR_LOGIC_CLOCK_PIN  32
#define REAR_LOGIC_PIN  	  33
#define SERIAL1_RX_PIN        34  // input only
#define UNUSED_INPUT_35       35  // input only
#define BUTTON_PIN            36  // VP - input only
#define NONEXISTENT_PIN_37    37
#define NONEXISTENT_PIN_38    38
#define ANALOG_POT_PIN        39  // input only

#endif