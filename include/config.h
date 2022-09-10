#ifndef CONFIG_H
    #define CONFIG_H
    #include <Arduino.h>
    #define debugSerial Serial
    #define pylonSerial Serial2
    #define DEBUG
    
    #ifdef DEBUG
    #define dbg(x...) debugSerial.print(x);
    #define dbgln(x...) debugSerial.println(x);
    #else /* DEBUG */
    #define dbg(x...) ;
    #define dbgln(x...) ;
    #endif /* DEBUG */
#endif /* CONFIG_H */