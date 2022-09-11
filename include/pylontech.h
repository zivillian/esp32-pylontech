#ifndef PYLONTECH_H
    #define PYLONTECH_H
    #include <Arduino.h>

    enum ControlIdentifyCode {
         Default = 0x46
    };

    enum CommandInformation {
        //response
        Normal = 0x00,
        VersionError = 0x01,
        ChecksumError = 0x02,
        LChecksumError = 0x03,
        InvalidCid2 = 0x04,
        CommandFormatError = 0x05,
        InvalidData = 0x06,
        AdrError = 0x90,
        CommunicationError = 0x91,
        //request
        AnalogValueFixedPoint = 0x42,
        AlarmInfo = 0x44,
        SystemParameterFixedPoint = 0x47,
        ProtocolVersion = 0x4f,
        ManufacturerInfo = 0x51,
        GetChargeDischargeManagementInfo = 0x92,
        Serialnumber = 0x93,
        SetChargeDischargeManagementInfo = 0x94,
        Turnoff = 0x95,
        FirmwareInfo = 0x96,
    };

    enum PylonInfoFlags{
        UnreadAlarmValueChange = 0x01,
        UnreadSwitchingValueChange = 0x10
    };

    class Pylonframe{
        private:
            class ChecksumPrint:public Print{
                private:
                    Print *_inner;
                    uint16_t _checksum;
                public:
                    ChecksumPrint(Print *inner);
                    size_t write(uint8_t) override;
                    size_t write(const char *str);
                    void WriteChecksum();
                    static uint16_t Final(uint16_t checksum);
            };
            uint8_t Lchecksum(uint16_t length);
        public:
            uint8_t MajorVersion;
            uint8_t MinorVersion;
            uint8_t Address;
            ControlIdentifyCode Cid1;
            CommandInformation Cid2;
            String Info;
            bool HasError;
            Pylonframe();
            Pylonframe(String data);
            void WriteTo(Print *target);
            uint16_t CalculateChecksum(String data);

            class PylonInfo{
                protected:
                    String Info;
                    static String HexDecode(String data);
                    int16_t GetInt16(unsigned int begin);
                    uint16_t GetUInt16(unsigned int begin);
                public:
                    PylonInfo(String info);
                    virtual void print(Print *out)=0;
            };

            class PylonSerialnumber:PylonInfo{
                public:
                    uint8_t Address();
                    String Serialnumber();
                    PylonSerialnumber(String info);
                    void print(Print *out);
            };

            class PylonManufacturerInfo:PylonInfo{
                public:
                    String Battery();
                    uint8_t SoftwareMajorVersion();
                    uint8_t SoftwareMinorVersion();
                    String Manufacturer();
                    PylonManufacturerInfo(String info);
                    void print(Print *out);
            };

            class PylonFirmwareInfo:PylonInfo{
                public:
                    uint8_t Address();
                    uint8_t ManufactureMajorVersion();
                    uint8_t ManufactureMinorVersion();
                    uint8_t MainlineMajorVersion();
                    uint8_t MainlineMinorVersion();
                    uint8_t MainlinePatchVersion();
                    PylonFirmwareInfo(String info);
                    void print(Print *out);
            };

            class PylonSystemParameter:PylonInfo{
                public:
                    PylonInfoFlags InfoFlags();
                    float CellHighVoltageLimit();
                    float CellLowVoltageLimit();
                    float CellUnderVoltageLimit();
                    float ChargeHighTemperatureLimit();
                    float ChargeLowTemperatureLimit();
                    float ChargeCurrentLimit();
                    float ModuleHighVoltageLimit();
                    float ModuleLowVoltageLimit();
                    float ModuleUnderVoltageLimit();
                    float DischargeHighTemperatureLimit();
                    float DischargeLowTemperatureLimit();
                    float DischargeCurrentLimit();
                    PylonSystemParameter(String info);
                    void print(Print *out);
            };
    };
#endif /* PYLONTECH_H */