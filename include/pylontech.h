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
    };
#endif /* PYLONTECH_H */