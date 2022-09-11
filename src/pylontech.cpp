#include "pylontech.h"

Pylonframe::Pylonframe()
    :Cid1(ControlIdentifyCode::Default)
    ,Info("")
    ,HasError(false)
{}

Pylonframe::Pylonframe(String data)
    :Pylonframe()
{
    if (data.length() < 16 || 
        data[0] != '~' ||
        !data.endsWith("\r")){
        HasError = true;
        return;
    }
    MajorVersion = data[1] - 48;
    MinorVersion = data[2] - 48;
    Address = strtoul(data.substring(3,5).c_str(), 0, HEX);
    Cid1 = (ControlIdentifyCode)strtoul(data.substring(5,7).c_str(), 0, HEX);
    Cid2 = (CommandInformation)strtoul(data.substring(7,9).c_str(), 0, HEX);
    auto length = strtoul(data.substring(9,13).c_str(), 0, HEX);
    auto lchksum = length >> 12;
    auto lenid = length & 0x0fff;
    if (lchksum != Lchecksum(lenid)){
        HasError = true;
        return;
    }
    if (data.length() < 13 + lenid + 4){
        HasError = true;
        return;
    }
    Info = data.substring(13,  13 + lenid);
    auto chksum = strtoul(data.substring(13 + lenid, 13 + lenid + 4).c_str(), 0, HEX);
    if (chksum != CalculateChecksum(data.substring(1, data.length()-5))){
        HasError = true;
        return;
    }
    if (data.length() > 13 + lenid + 4 + 1){
        HasError = true;
        return;
    }
}

void Pylonframe::WriteTo(Print *target){
    target->print('~');
    auto output = Pylonframe::ChecksumPrint(target);
    output.print(MajorVersion);
    output.print(MinorVersion);
    output.printf("%02X", Address);
    output.printf("%02X", Cid1);
    output.printf("%02X", Cid2);
    switch (Cid2)
    {
        case CommandInformation::FirmwareInfo:
        case CommandInformation::Serialnumber:
        case CommandInformation::GetChargeDischargeManagementInfo:
        case CommandInformation::AnalogValueFixedPoint:
            //payload is the address with length two (according to the spec)
            output.print(Lchecksum(2));
            output.print("002");
            break;
        default:
            //otherwise it's an empty payload
            output.print("0000");
            break;
    }
    output.WriteChecksum();
    target->print('\r');
}

uint16_t Pylonframe::CalculateChecksum(String data){
    uint16_t result = 0;
    for (size_t i = 0; i < data.length(); i++)
    {
        result += data[i];
    }
    return ChecksumPrint::Final(result);
}

uint8_t Pylonframe::Lchecksum(uint16_t length){
    if (length == 0) return 0;
    auto sum = (length & 0xf) +((length >> 4) & 0xf) + ((length >> 8) & 0xf);
    auto modulus = sum % 16;
    return (uint16_t)(0b1111 - modulus + 1);
}

Pylonframe::ChecksumPrint::ChecksumPrint(Print *inner)
    :_inner(inner)
    ,_checksum(0)
{}

size_t Pylonframe::ChecksumPrint::write(uint8_t arg){
    _checksum += arg;
    return _inner->write(arg);
}

uint16_t Pylonframe::ChecksumPrint::Final(uint16_t checksum){
    auto remainder = checksum % 65536;
    return (uint16_t)((~remainder) + 1);
}

void Pylonframe::ChecksumPrint::WriteChecksum(){
    _inner->printf("%04X", Final(_checksum));
}

Pylonframe::PylonInfo::PylonInfo(String info)
    :Info(info)
{}

String Pylonframe::PylonInfo::HexDecode(String data){
    String result;
    if ((data.length() & 1) != 0) return result;
    for (size_t i = 0; i < data.length(); i+=2)
    {
        result += (char)strtol(data.substring(i,i+2).c_str(), 0, HEX);
    }
    return result;
}

int16_t Pylonframe::PylonInfo::GetInt16(unsigned int begin){
    return strtol(Info.substring(begin, begin+4).c_str(), 0, HEX);
}

uint16_t Pylonframe::PylonInfo::GetUInt16(unsigned int begin){
    return strtoul(Info.substring(begin, begin+4).c_str(), 0, HEX);
}

Pylonframe::PylonSerialnumber::PylonSerialnumber(String info)
    :PylonInfo(info)
{}

uint8_t Pylonframe::PylonSerialnumber::Address(){
    return strtoul(Info.substring(0,2).c_str(), 0, HEX);
}

String Pylonframe::PylonSerialnumber::Serialnumber(){
    return HexDecode(Info.substring(2));    
}

void Pylonframe::PylonSerialnumber::print(Print *out){
    out->printf("Address: %u\n", Address());
    out->printf("Serialnumber: %s\n", Serialnumber().c_str());
}

Pylonframe::PylonManufacturerInfo::PylonManufacturerInfo(String info)
    :PylonInfo(info)
{}

String Pylonframe::PylonManufacturerInfo::Battery(){
    return HexDecode(Info.substring(0, 20));
}

uint8_t Pylonframe::PylonManufacturerInfo::SoftwareMajorVersion(){
    return strtoul(Info.substring(20,22).c_str(), 0, HEX);
}

uint8_t Pylonframe::PylonManufacturerInfo::SoftwareMinorVersion(){
    return strtoul(Info.substring(22,24).c_str(), 0, HEX);
}

String Pylonframe::PylonManufacturerInfo::Manufacturer(){
    return HexDecode(Info.substring(24, 64));
}

void Pylonframe::PylonManufacturerInfo::print(Print *out){
    out->printf("Battery: %s\n", Battery().c_str());
    out->printf("SoftwareVersion: %u.%u\n", SoftwareMajorVersion(), SoftwareMinorVersion());
    out->printf("Manufacturer: %s\n", Manufacturer().c_str());
}

Pylonframe::PylonFirmwareInfo::PylonFirmwareInfo(String info)
    :PylonInfo(info)
{}

uint8_t Pylonframe::PylonFirmwareInfo::Address(){
    return strtoul(Info.substring(0,2).c_str(), 0, HEX);
}

uint8_t Pylonframe::PylonFirmwareInfo::ManufactureMajorVersion(){
    return strtoul(Info.substring(2,4).c_str(), 0, HEX);
}

uint8_t Pylonframe::PylonFirmwareInfo::ManufactureMinorVersion(){
    return strtoul(Info.substring(4,6).c_str(), 0, HEX);
}

uint8_t Pylonframe::PylonFirmwareInfo::MainlineMajorVersion(){
    return strtoul(Info.substring(6,8).c_str(), 0, HEX);
}

uint8_t Pylonframe::PylonFirmwareInfo::MainlineMinorVersion(){
    return strtoul(Info.substring(8,10).c_str(), 0, HEX);
}

uint8_t Pylonframe::PylonFirmwareInfo::MainlinePatchVersion(){
    return strtoul(Info.substring(10,12).c_str(), 0, HEX);
}

void Pylonframe::PylonFirmwareInfo::print(Print *out){
    out->printf("Address: %u\n", Address());
    out->printf("ManufactureVersion: %u.%u\n", ManufactureMajorVersion(), ManufactureMinorVersion());
    out->printf("MainlineVersion: %u.%u.%u\n", MainlineMajorVersion(), MainlineMinorVersion(), MainlinePatchVersion());
}

Pylonframe::PylonSystemParameter::PylonSystemParameter(String info)
    :PylonInfo(info)
{}

PylonInfoFlags Pylonframe::PylonSystemParameter::InfoFlags(){
    return (PylonInfoFlags)strtoul(Info.substring(0,2).c_str(), 0, HEX);
}

float Pylonframe::PylonSystemParameter::CellHighVoltageLimit(){
    return GetInt16(2) * 0.001;
}

float Pylonframe::PylonSystemParameter::CellLowVoltageLimit(){
    return GetInt16(6) * 0.001;
}

float Pylonframe::PylonSystemParameter::CellUnderVoltageLimit(){
    return GetInt16(10) * 0.001;
}

float Pylonframe::PylonSystemParameter::ChargeHighTemperatureLimit(){
    return (GetInt16(14) - 2731) * 0.1;
}

float Pylonframe::PylonSystemParameter::ChargeLowTemperatureLimit(){
    return (GetInt16(18) - 2731) * 0.1;
}

float Pylonframe::PylonSystemParameter::ChargeCurrentLimit(){
    return GetInt16(22) * 0.01;
}

float Pylonframe::PylonSystemParameter::ModuleHighVoltageLimit(){
    return GetUInt16(26) * 0.001;
}

float Pylonframe::PylonSystemParameter::ModuleLowVoltageLimit(){
    return GetUInt16(30) * 0.001;
}

float Pylonframe::PylonSystemParameter::ModuleUnderVoltageLimit(){
    return GetUInt16(34) * 0.001;
}

float Pylonframe::PylonSystemParameter::DischargeHighTemperatureLimit(){
    return (GetInt16(38) - 2731) * 0.1;
}

float Pylonframe::PylonSystemParameter::DischargeLowTemperatureLimit(){
    return (GetInt16(42) - 2731) * 0.1;
}

float Pylonframe::PylonSystemParameter::DischargeCurrentLimit(){
    return GetInt16(46) * 0.01;
}

void Pylonframe::PylonSystemParameter::print(Print *out) {
    out->printf("CellHighVoltageLimit: %.3f\n", CellHighVoltageLimit());
    out->printf("CellLowVoltageLimit: %.3f\n", CellLowVoltageLimit());
    out->printf("CellUnderVoltageLimit: %.3f\n", CellUnderVoltageLimit());
    out->printf("ChargeHighTemperatureLimit: %.1f\n", ChargeHighTemperatureLimit());
    out->printf("ChargeLowTemperatureLimit: %.1f\n", ChargeLowTemperatureLimit());
    out->printf("ChargeCurrentLimit: %.3f\n", ChargeCurrentLimit());
    out->printf("ModuleHighVoltageLimit: %.3f\n", ModuleHighVoltageLimit());
    out->printf("ModuleLowVoltageLimit: %.3f\n", ModuleLowVoltageLimit());
    out->printf("ModuleUnderVoltageLimit: %.3f\n", ModuleUnderVoltageLimit());
    out->printf("DischargeHighTemperatureLimit: %.1f\n", DischargeHighTemperatureLimit());
    out->printf("DischargeLowTemperatureLimit: %.1f\n", DischargeLowTemperatureLimit());
    out->printf("DischargeCurrentLimit: %.3f\n", DischargeCurrentLimit());
    
}