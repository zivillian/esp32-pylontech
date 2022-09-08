#include "pylontech.h"

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

void Pylonframe::ChecksumPrint::WriteChecksum(){
    auto remainder = _checksum % 65536;
    auto checksum = (uint16_t)((~remainder) + 1);
    _inner->printf("%04X", checksum);
}