#include "config.h"

Config::Config()
    :_prefs(NULL)
    ,_interval(10000)
{}

void Config::begin(Preferences *prefs)
{
    _prefs = prefs;
    _interval = _prefs->getUInt("interval", _interval);
}

uint32_t Config::getInterval(){
    return _interval;
}

void Config::setInterval(uint32_t value){
    if (_interval == value) return;
    _interval = value;
    _prefs->putUInt("interval", _interval);
}