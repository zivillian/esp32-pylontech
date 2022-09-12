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
            uint8_t _modules;
            uint32_t _interval;
            String _mqttHost;
            uint16_t _mqttPort;
            String _mqttPrefix;
            String _mqttUsername;
            String _mqttPassword;

        public:
            uint8_t getModuleCount();
            void setModuleCount(uint8_t value);
            uint32_t getInterval();
            void setInterval(uint32_t value);
            String getMqttHost();
            void setMqttHost(String value);
            uint16_t getMqttPort();
            void setMqttPort(uint16_t value);
            String getMqttPrefix();
            void setMqttPrefix(String value);
            String getMqttUsername();
            void setMqttUsername(String value);
            String getMqttPassword();
            void setMqttPassword(String value);
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