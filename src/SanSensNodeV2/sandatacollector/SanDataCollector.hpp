#pragma once

namespace SANSENSNODE_NAMESPACE
{
const int capacity = JSON_OBJECT_SIZE(25);
StaticJsonDocument<capacity> _jsondoc;

class SanDataCollector
{
private:
    size_t _buffersize;

public:
    SanDataCollector()
    {
        _buffersize = capacity;
        reset();
    }

    void reset()
    {
        _jsondoc.clear();
    }
    // add key / value as string
    void Add_s(const char *key, const char *value)
    {
        _jsondoc[key] = value;
    }
    // add key / value as int
    void Add_i(const char *key, int value)
    {
        _jsondoc[key] = value;
    }
    // add key / value as float
    void Add_f(const char *key, float value)
    {
        _jsondoc[key] = value;
    }
    void Add_d(const char *key, double value)
    {
        _jsondoc[key] = value;
    }
    // add key / value as bool
    void Add_b(const char *key, bool value)
    {
        _jsondoc[key] = value;
    }
    unsigned int getbufferSize()
    {
        return _buffersize;
    }
    //serialize content to string
    unsigned int Serialize(char *outbuff)
    {
        size_t n = serializeJson(_jsondoc, outbuff, _buffersize);
        return n;
    }
};
} // namespace SANSENSNODE_NAMESPACE
