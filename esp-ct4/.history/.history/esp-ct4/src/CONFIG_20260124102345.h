// ==================== HARDWARE CONFIGURATION ====================
// ESP32 I2C pins
#define I2C_SDA 21  // Default for ESP32 (Original)
#define I2C_SCL 22


// ADS1115 address (default 0x48, change if ADDR pin is connected)
#define ADS1115_ADDRESS 0x48
 // No argument
// CT Sensor Configuration
// #define CT_RATIO 15.0f          // 15A per 1V (SCT-013-015)
// #define LSB_VOLTAGE 0.125f          // 0.125 mV per count (GAIN_ONE)
// #define BIAS_ADC_OFFSET 12893.0f    // Your zero offset
// #define BIAS_VOLTAGE 1.612f         // Your measured bias voltage
// #define VREF_LM4040 2.5f       // LM4040 reference voltage
// #define R1_VALUE 10000.0f       // LM4040 series resistor (1.2kΩ)
// #define R2_VALUE 87000.0f      // Bias divider resistor (10kΩ)
// #define R3_VALUE 153000.0f      // Bias divider resistor (20kΩ)

// // Measurement Configuration
// #define SAMPLES_PER_READING 2000  // For RMS calculation
// #define GAIN_SWITCH_THRESHOLD 2.0f // Amps: switch gain below this
// #define CT_TURNS 1  