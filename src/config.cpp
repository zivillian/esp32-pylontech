#include "config.h"

Config::Config()
    :_prefs(NULL)
    ,_modules(1)
    ,_interval(10000)
    ,_mqttHost("")
    ,_mqttPort(1883)
    ,_mqttPrefix("pylontech")
    ,_mqttUsername("")
    ,_mqttPassword("")
{}

void Config::begin(Preferences *prefs)
{
    _prefs = prefs;
    _modules = _prefs->getUChar("modules", _modules);
    _interval = _prefs->getUInt("interval", _interval);
    _mqttHost = _prefs->getString("mqtthost", _mqttHost);
    _mqttPort = _prefs->getUShort("mqttport", _mqttPort);
    _mqttPrefix = _prefs->getString("mqttprefix", _mqttPrefix);
    _mqttUsername = _prefs->getString("mqttuser", _mqttUsername);
    _mqttPassword = _prefs->getString("mqttpass", _mqttPassword);
}

uint8_t Config::getModuleCount(){
    return _modules;
}

void Config::setModuleCount(uint8_t value){
    if (_modules == value) return;
    _modules = value;
    _prefs->putUChar("modules", _modules);
}

uint32_t Config::getInterval(){
    return _interval;
}

void Config::setInterval(uint32_t value){
    if (_interval == value) return;
    _interval = value;
    _prefs->putUInt("interval", _interval);
}

String Config::getMqttHost()
{
    return _mqttHost;
}

void Config::setMqttHost(String value){
    if (_mqttHost.equals(value)) return;
    _mqttHost = value;
    _prefs->putString("mqtthost", value);
}

uint16_t Config::getMqttPort(){
    return _mqttPort;
}

void Config::setMqttPort(uint16_t value){
    if (_mqttPort == value) return;
    _mqttPort = value;
    _prefs->putUShort("mqttport", _mqttPort);
}

String Config::getMqttPrefix()
{
    return _mqttPrefix;
}

void Config::setMqttPrefix(String value){
    if (_mqttPrefix.equals(value)) return;
    _mqttPrefix = value;
    _prefs->putString("mqttprefix", value);
}

String Config::getMqttUsername()
{
    return _mqttUsername;
}

void Config::setMqttUsername(String value){
    if (_mqttUsername.equals(value)) return;
    _mqttUsername = value;
    _prefs->putString("mqttuser", value);
}

String Config::getMqttPassword()
{
    return _mqttPassword;
}

void Config::setMqttPassword(String value){
    if (_mqttPassword.equals(value)) return;
    _mqttPassword = value;
    _prefs->putString("mqttpass", value);
}