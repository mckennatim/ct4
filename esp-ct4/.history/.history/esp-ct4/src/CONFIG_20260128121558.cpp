#include "CONFIG.h"
#include "ct.h"

CT_Config sensors[4] = {
  {0, GAIN_ONE, 0.0001875,  104.7, 0.922, 15, .3, "Heat_Pump"},
  {1, GAIN_ONE, 0.0001875,  104.7, 0.922, 15, .3, "Solar"},
  {2, GAIN_ONE, 0.0001875,  104.7, 0.922, 50, .3, "EV"},
  {3, GAIN_ONE, 0.0001875,  104.7, 0.922, 15, .3, "empty"},
};

