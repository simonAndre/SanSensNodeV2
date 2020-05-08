#pragma once

namespace SANSENSNODE_NAMESPACE
{
    class JsonColl;
    class SanCodedStr;
    
    /**
     * @brief base class for devices plugins
     * 
     */
    class devicePlugin
    {
    private:
        /* data */
    public:
        devicePlugin();
        ~devicePlugin();

        virtual bool collectdata(JsonColl *collector);
        virtual void setupdevice(SubMenu &device_menu);
        virtual void onInputMessage(SanCodedStr data);
    };
}