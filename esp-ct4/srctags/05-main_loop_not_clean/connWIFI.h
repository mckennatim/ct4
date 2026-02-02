#ifndef connWIFI_h
#define connWIFI_h

#include <Arduino.h>

// This function will block until connected or timeout
bool setupWIFI();

// Optional: Function to reset settings (for testing/debugging)
void resetWIFI();

#endif