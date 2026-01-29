#ifndef CONFIG_h
#define CONFIG_h
#include "ct.h" // Needs to know what CT_Config is
// ==================== HARDWARE CONFIGURATION ====================
// ESP32 I2C pins
#define I2C_SDA 21  // Default for ESP32 (Original)
#define I2C_SCL 22
// ADS1115 address (default 0x48, change if ADDR pin is connected)
#define ADS1115_ADDRESS 0x48

extern CT_Config sensors[4];

#endif // CONFIG_h