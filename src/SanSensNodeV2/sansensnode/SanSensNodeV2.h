/*
Simon ANDRE, apr 2020
tested on ESP32
*/

#pragma once
// #include <Arduino.h>
#include "DeepSleep.h"
#include "DevicePlugin.h"
#include <functional>
#include <PubSubClient.h>

#define EXP_NBDEVICESMAX 5

namespace SANSENSNODE_NAMESPACE
{
    // typedef flyingCollection::JsonStream<106> JsonColl;

    // template<Tdatarow
    class SanSensNodeV2
    {

    public:
        /**
         * @brief Construct a new San Sens Node V 2 object
         * 
         * @param nodename: name for this node
         * @param ssid: id of wifi network
         * @param wifipasswd: password of wifi network
         * @param mqttserver: adress of mqtt server
         * @param lowEnergyMode :true to be as energy efficient as we can at the trade off of the power (runing at 80Mhz...)
         * @param G : measurement cycle duration ie: sleep time (in seconds) (in awake mode and sleep mode)
         * @param Pfactor : publication and mqtt connection frequency (in G multiples)
         */
        SanSensNodeV2(const char *nodename, const char *ssid, const char *wifipasswd, const char *mqttserver, int G, int Pfactor);

        /**
         * @brief Fill the device collection
         * 
         * @param device 
         */
        void addDevice(DevicePlugin &device);

        /**
         * @brief called by main sketch
         * 
         */
        void Setup();

        /**
         * @brief called by main sketch
         * 
         */
        void Loop();

        /**
         * @brief return true if the last boot was done after a sleep period or not (regular start) 
         * 
         * @return true 
         * @return false 
         */
        bool bootAfterSleep();

        /**
         * @brief wait a given time still listening for serial event to possibly stop the execution flow to display the interractive menu
         * the atomic wait time between serial checks will be defined by SANSENSNODE_WAITLOOPDELAYMS
         * 
         * @param waittimems 
         * @return true : we exit from the menu requesting to break the current loop
         * @return false regular end of the wait loop
         */
        bool waitListeningIOevents(unsigned int waittimems);

        /**
         * @brief wait for the next G time. If awakemode is false, go to deep sleep.
         * 
         */
        bool waitNextG();

        // called to collect sensors data to be sent via mqtt
        void SetCollectDataCallback(std::function<bool(JsonColl &)> collectdatafunction);

        //setup the callback called to setup the sensors
        void SetSetupDeviceCallback(std::function<void(SubMenu &)> setupdevicesfunction);

        static void SetInputMessageCallback(std::function<void(flyingCollection::SanCodedStr const &)> inputmessagefunction);

        /**
         * @brief go to deep sleep (with disabling wifi, bluetooth and so)
         * 
         */
        void DeepSleep();

        static char *getVersion();

        SubMenu *getDeviceMenu();

    private:
        PubSubClient mqttClient;
        JsonColl *datacoll;
        bool initStringValue(const char *menuname);
        bool DisplayStringValue(const char *menuname);
        Menubase *_consolemenu;
        SubMenu *_device_menu;
        SANSENSNODE_NAMESPACE::DeepSleep *_deepsleep;
        uint8_t _measurementAttenmpts;
        const char *_mqttTopicBaseName{"/ssnet/"};
        const char *_lostTopic{"/ssnet/lost"}; // not implemented : when the sensor has not been initialized, it wait configuration data from this topic
        //std::vector<DevicePlugin*> _devices;
       
        DevicePlugin *_devicesarr[EXP_NBDEVICESMAX];
        uint8_t _deviceidx = 0;
      
      
      
        bool mqttConnect();

        //allow the client to process incoming messages and maintain its connection to the server.
        bool WaitforMqtt(int nb);

        bool mqttSubscribe();

        bool collectMeasurement(JsonColl *dc);

        // publish and subscribe to the respective MQTT chanels (open the connection and close after)
        bool mqttpubsub(JsonColl *dc);

        /**
         * @brief setup the WIFi connection
         * 
         * @return true 
         * @return false issue or relaod required from the IHM, need to exit
         */
        bool Setup_wifi();

        void wifiOff();

        void setupSerialMenu();

        bool collectMeasurement_internal(JsonColl &dc);

        static bool mqttCallback(char *topic, uint8_t *payload, unsigned int length);

        void HandleMqttReceive();

        static bool buildInfos();

        void rtInfos();
        static bool SetEnergyMode();
        static bool breakLoop();
    };
} // namespace SANSENSNODE_NAMESPACE
