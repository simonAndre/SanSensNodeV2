#pragma once
// #include "SanSensNodeV2.h"
#include <flyingCollection.h>
#include "specialTypes.h"
#include <consoleMenu.h>


namespace SANSENSNODE_NAMESPACE
{

    class SanSensNodeV2;
    /**
     * @brief base class for devices plugins
     * 
     */
    class SensorPlugin
    {
    private:
        SanSensNodeV2 *_sansens_instance;

    protected:
        std::string _sensorname;

    public:
         SensorPlugin(const char *devicename);
        virtual ~SensorPlugin();

        virtual bool collectdata(JsonColl &collector);

        /**
         * @brief device setup actions and console menu entries
         * 
         * @param device_menu : "device" section submenu, to hook up additional menu entries 
         */
        virtual void firstSetup();
        virtual void setupdevice(SubMenu &device_menu);
        virtual void onInputMessage(flyingCollection::SanCodedStr &data);
        virtual const char *getSensorName();
        void hookSanSensInstance(SanSensNodeV2 *instance);
        virtual SanSensNodeV2* getSanSensInstance();
        bool enabled;
        const char *enableMenuFunctionName();
    };
} // namespace SANSENSNODE_NAMESPACE