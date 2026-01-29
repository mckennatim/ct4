## README esp-ct4a
Working 4 sensor readings with threshold 

Data structure sensors[4] in CONFIG

in loop()
* for (int i=0; i<4; i++) ... readCurrent()
* if over threshold sendMQQT() dummy function


