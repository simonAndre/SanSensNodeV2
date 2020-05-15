#pragma once
// #include "SanSensNodeV2.h"
#include <flyingCollection.h>
#include "specialTypes.h"
#include <consoleMenu.h>

namespace SANSENSNODE_NAMESPACE
{

    class SanSensNodeV2;

    /**
 * @brief : struc to hold sensors metadata inthe remanant RAM (minimal data to hold during the deep sleep) 
 * 
 */
    struct SensorRemanantMD
    {
        bool enabled : 1;
    };

    /**
     * @brief base class for devices plugins
     * 
     */
    class SensorPlugin
    {
    private:
    protected:
        std::string _sensorname{"XXsensor"};
        SanSensNodeV2 *_sansens_instance;
        SubMenu *_sensor_menu;

    public:
        SensorPlugin(const char *devicename);
        virtual ~SensorPlugin();

        uint8_t idx;
        bool enabled;

        virtual bool collectdata(JsonColl &collector);

        /**
         * @brief device setup actions and console menu entries
         * 
         * @param device_menu : "device" section submenu, to hook up additional menu entries 
         */
        virtual void firstSetup();
        virtual void setupsensor();
        virtual void setMenu(SubMenu &sensor_menu);
        virtual void onInputMessage(flyingCollection::SanCodedStr &data);
        virtual const char *getSensorName();
        void hookSanSensInstance(SanSensNodeV2 *instance);
        virtual SanSensNodeV2 *getSanSensInstance();
        const char *enableMenuFunctionName();
        SubMenu *GetMenu();
    };
} // namespace SANSENSNODE_NAMESPACE