
#include <ArduinoJson.h>
#include "SanDataCollector.h"

const int capacity = JSON_OBJECT_SIZE(25);
StaticJsonDocument<capacity> _jsondoc;

SanDataCollector::SanDataCollector()
{
    _buffersize = capacity;
    reset();
}

SanDataCollector::~SanDataCollector()
{
}

void SanDataCollector::reset()
{
    _jsondoc.clear();
}

unsigned int SanDataCollector::getbufferSize()
{
    return _buffersize;
}

void SanDataCollector::Add_s(const char *key, const char *value)
{
    _jsondoc[key] = *value;
}
void SanDataCollector::Add_i(const char *key, int value)
{
    _jsondoc[key] = value;
}
void SanDataCollector::Add_f(const char *key, float value)
{
    _jsondoc[key] = value;
}
void SanDataCollector::Add_d(const char *key, double value)
{
    _jsondoc[key] = value;
}
void SanDataCollector::Add_b(const char *key, bool value)
{
    _jsondoc[key] = value;
}

size_t SanDataCollector::Serialize(char *outbuff)
{
    size_t n = serializeJson(_jsondoc, outbuff, _buffersize);
    // Serial.print("Publish message: ");
    // Serial.println(_buff);
    return n;
}
