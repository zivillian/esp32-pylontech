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
    Address = strtoul(data.substring(3,5).c_str(), 0, 16);
    Cid1 = (ControlIdentifyCode)strtoul(data.substring(5,7).c_str(), 0, 16);
    Cid2 = (CommandInformation)strtoul(data.substring(7,9).c_str(), 0, 16);
    auto length = strtoul(data.substring(9,13).c_str(), 0, 16);
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
    auto chksum = strtoul(data.substring(13 + lenid, 13 + lenid + 4).c_str(), 0, 16);
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