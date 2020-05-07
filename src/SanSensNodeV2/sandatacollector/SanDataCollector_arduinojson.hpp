#pragma once
#include <ArduinoJson.h>

namespace SANSENSNODE_NAMESPACE
{
const int JsonCapacity = 250; //JSON_OBJECT_SIZE(50);

// template <size_t JsonCapacity=250>
class SanDataCollector
{
private:
    StaticJsonDocument<JsonCapacity> _jsondoc;

public:
    SanDataCollector()
    {
        reset();
    }

    void reset()
    {
        _jsondoc.clear();
    }

    /**
 * @brief  add key / value as T
 * 
 * @tparam T 
 * @param key string key
 * @param value  of type T
 */
    template <typename T>
    void Add(const char *key, T value)
    {
        _jsondoc[key] = value;
    }

  
    //serialize content to string
    unsigned int Serialize(char *outbuff,size_t buffersize)
    {
        size_t n = serializeJson(_jsondoc, outbuff, buffersize);
        return n;
    }
};
} // namespace SANSENSNODE_NAMESPACE
