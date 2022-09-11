#ifndef PYLONTECH_H
    #define PYLONTECH_H
    #include <Arduino.h>
    typedef std::function<void(String name, String value)> PublishFunction;

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
        //internal
        NoReponse = 0xfd,
        Timeout = 0xfe,
    };

    enum PylonInfoFlags{
        UnreadAlarmValueChange = 0x01,
        UnreadSwitchingValueChange = 0x10
    };

    enum ChargeDischargeStatus{
        ChargeEnabled = 0x80,
        DischargeEnabled = 0x40,
        ChargeImmediately1 = 0x20,
        ChargeImmediately2 = 0x10,
        FullChargeRequest = 0x08,
    };

    enum AlarmFlag{
        None = 0,
        BelowLowerLimit = 0x01,
        AboverHigherLimit = 0x02,
        OtherError = 0xF0
    };

    enum AlarmStatus1{
        ModuleUnderVoltage = 0x80,
        ChargeOverTemperature = 0x40,
        DischargeOverTemperature = 0x20,
        DischargeOverCurrent = 0x10,
        ChargeOverCurrent = 0x04,
        CellUnderVoltage = 0x02,
        ModuleOverVoltage = 0x01
    };

    enum AlarmStatus2{
        UsingBatteryModulePower = 0x08,
        DischargeMosfet = 0x04,
        ChargeMosfet = 0x02,
        PreMosfet = 0x01
    };

    enum AlarmStatus3{
        EffectiveChargeCurrent = 0x80,
        EffectiveDischargeCurrent = 0x40,
        Heater = 0x20,
        FullyCharged = 0x08,
        Buzzer = 0x01
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
            Pylonframe(uint8_t address, CommandInformation cid2);
            Pylonframe(uint8_t major, uint8_t minor, uint8_t address, CommandInformation cid2);
            Pylonframe(String data);
            void WriteTo(Print *target);
            uint16_t CalculateChecksum(String data);
            void print(Print *out);

            class PylonInfo{
                protected:
                    String Info;
                    static String HexDecode(String data);
                    int16_t GetInt16(unsigned int begin);
                    uint16_t GetUInt16(unsigned int begin);
                    uint32_t GetUInt24(unsigned int begin);
                public:
                    PylonInfo(String info);
                    virtual void print(Print *out)=0;
                    virtual void publish(PublishFunction callback) = 0;
            };

            class PylonSerialnumber:PylonInfo{
                public:
                    uint8_t Address();
                    String Serialnumber();
                    PylonSerialnumber(String info);
                    void print(Print *out);
                    void publish(PublishFunction callback);
            };

            class PylonManufacturerInfo:PylonInfo{
                public:
                    String Battery();
                    uint8_t SoftwareMajorVersion();
                    uint8_t SoftwareMinorVersion();
                    String Manufacturer();
                    PylonManufacturerInfo(String info);
                    void print(Print *out);
                    void publish(PublishFunction callback);
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
                    void publish(PublishFunction callback);
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
                    void publish(PublishFunction callback);
            };

            class PylonChargeDischargeManagementInfo:PylonInfo{
                public:
                    uint8_t Address();
                    float ChargeVoltageLimit();
                    float DischargeVoltageLimit();
                    float ChargeCurrentLimit();
                    float DischargeCurrentLimit();
                    ChargeDischargeStatus Status();
                    PylonChargeDischargeManagementInfo(String info);
                    void print(Print *out);
                    void publish(PublishFunction callback);
            };

            class PylonAlarmInfo:PylonInfo{
                private:
                    String Name(AlarmFlag flag);
                public:
                    PylonInfoFlags InfoFlags();
                    uint8_t Address();
                    uint8_t CellCount();
                    AlarmFlag CellVoltage(size_t cell);
                    uint8_t TemperatureCount();
                    AlarmFlag BmsTemperature();
                    AlarmFlag TemperatureCell1to4();
                    AlarmFlag TemperatureCell5to8();
                    AlarmFlag TemperatureCell9to12();
                    AlarmFlag TemperatureCell13to15();
                    AlarmFlag MosfetTemperature();
                    AlarmFlag ChargeCurrent();
                    AlarmFlag ModuleVoltage();
                    AlarmFlag DischargeCurrent();
                    AlarmStatus1 Status1();
                    AlarmStatus2 Status2();
                    AlarmStatus3 Status3();
                    uint16_t CellError();
                    PylonAlarmInfo(String info);
                    void print(Print *out);
                    void publish(PublishFunction callback);
            };

            class PylonAnalogValue:PylonInfo{
                private:
                    uint8_t UserDefined();
                public:
                    PylonInfoFlags InfoFlags();
                    uint8_t Address();
                    uint8_t CellCount();
                    float CellVoltage(size_t cell);
                    uint8_t TemperatureCount();
                    float BmsTemperature();
                    float TemperatureCell1to4();
                    float TemperatureCell5to8();
                    float TemperatureCell9to12();
                    float TemperatureCell13to15();
                    float Current();
                    float ModuleVoltage();
                    float RemainingCapacity();
                    float TotalCapacity();
                    uint16_t CycleNumber();
                    PylonAnalogValue(String info);
                    void print(Print *out);
                    void publish(PublishFunction callback);
            };
    };

    class Pylonclient{
        private:
            HardwareSerial *_serial;
            SemaphoreHandle_t _serialLock;
        public:
            void Begin(HardwareSerial *serial);
            Pylonframe SendCommand(Pylonframe request);
    };
#endif /* PYLONTECH_H */