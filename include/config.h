#ifndef CONFIG_H
    #define CONFIG_H
    #include <Arduino.h>
    #include <Preferences.h>
    #define debugSerial Serial
    #define pylonSerial Serial2
    #define DEBUG
    
    class Config{
        private:
            Preferences *_prefs;
            uint32_t _interval;
        public:
            uint32_t getInterval();
            void setInterval(uint32_t value);
            Config();
            void begin(Preferences *prefs);
    };

    #ifdef DEBUG
    #define dbg(x...) debugSerial.print(x);
    #define dbgln(x...) debugSerial.println(x);
    #else /* DEBUG */
    #define dbg(x...) ;
    #define dbgln(x...) ;
    #endif /* DEBUG */
#endif /* CONFIG_H */