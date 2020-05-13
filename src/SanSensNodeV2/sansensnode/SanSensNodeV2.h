#pragma once
#include "DeepSleep.hpp"
#include "DevicePlugin.h"
#include <flyingCollection.h>

namespace SANSENSNODE_NAMESPACE
{

 
  


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
        SanSensNodeV2(const char *nodename, const char *ssid, const char *wifipasswd, const char *mqttserver, int G, int Pfactor)
        {

            if (_firstinit) // première initialisation, ensuite on rentre dans ce constructeur à chaque reveil donc on ne doit pas réinitialiser les variables stockées dnas la RAM de la RTC
            {
                logdebug("enter SanSensNodeV2 ctor first init\n");
                _awakemode = SANSENSNODE_STARTSAWAKEN;

                _serial = true;
                _Gi = 0;
                _Pi = 0;
                _EXPwifiwait = SANSENSNODE_WIFIWAITTIMEMS;
                _EXPmqttattemps = SANSENSNODE_MQTTATTEMPTSNB;
                _verboseMode = false;
                _nodename = nodename;
                _ssid = ssid;
                _password = wifipasswd;
                _mqtt_server = mqttserver;
                _maxMeasurementAttenmpts = SANSENSNODE_MAX_MEASURES_ATTEMPTS;
                _G_seconds = G;
                _Pfactor = Pfactor;
                _wifitrialsmax = SANSENSNODE_WIFITRIALSINIT;
                _mqttsubscribe = SANSENSNODE_MQTTSUBSCRIBEATSTART;
            }
            loglevel((log_level_e)_loglevel);
            this->mqttClient.setClient(__espClient);
            setupSerialMenu();
            ++_bootCount;
            logdebug("end SanSensNodeV2 ctor\n");
        }

        /**
 * @brief Fill the device collection
 * 
 * @param device 
 */
        void addDevice(DevicePlugin &device)
        {
            _devices.emplace_back(device);
            device.hookSanSensInstance(this);
        }

        /**
 * @brief called by main sketch
 * 
 */
        void Setup()
        {
            SetEnergyMode();

            _deepsleep = new SANSENSNODE_NAMESPACE::DeepSleep();
            _deepsleep->SetupTouchpadWakeup(SANSENSNODE_TOUCHPADTHRESHOLD, SANSENSNODE_TOUCHPADGPIO);

            if (_serial)
            {
                delay(SANSENSNODE_WAITFORSERIALDELAY); // pour attendre port serie
                Serial.begin(115200);
                loginfo("#boot=%i\n", _bootCount);
                logdebug("#awakemode:%i\n", _awakemode);
                logdebug("G duration = %i\n", _G_seconds);
                logdebug("P factor = %i\n", _Pfactor);
                logdebug("WIFI mode = %i\n", _wifiMode);
                logflush();
            }

            //setup the devices taken from the collection
            if (_device_menu)
            {
                for (auto &dev : _devices)
                {
                    dev.setupdevice(*_device_menu);
                }

                if (_setupdevicesCallback)
                    _setupdevicesCallback(*_device_menu);
            }

            if (_firstinit || (_deepsleep && (_deepsleep->getWakeup_reason() == ESP_SLEEP_WAKEUP_TOUCHPAD)))
            {
                //conditions for prompting menu at start (but with a small timeout to save battery in case of unwanted awake)
                // auto op=_consolemenu->getOptions();
                // op.firstExpirationTimeSec = 5;
                // _consolemenu->setOptions(op);
                // _consolemenu->launchMenu();
                printf("input anything to show the menu. Expire in %i seconds", SANSENSNODE_FIRSTBOOTDELAYWAITINGMENU);
                waitListeningIOevents(SANSENSNODE_FIRSTBOOTDELAYWAITINGMENU * 1000);
            }

            if (_firstinit)
                _firstinit = false;
        }

        /**
 * @brief called by main sketch
 * 
 */
        void Loop()
        {

            _Gi++;
            _breakCurrentLoop = false;

            SetLogTimeStart();

            loginfo("G index:%i, millis=%i\n", _Gi, millis());

            _measurementAttenmpts = 1;

            datacoll = new JsonColl();

            logdebug("measure start\n");
            while (!SanSensNodeV2::collectMeasurement(datacoll) && _measurementAttenmpts <= _maxMeasurementAttenmpts)
            {
                _measurementAttenmpts++;
            }
            logdebug("measure end\n");
            logflush();

            if (_wifiMode > 0 && datacoll && _Gi % _Pfactor == 0)
                SanSensNodeV2::mqttpubsub(datacoll);
            if (!_breakCurrentLoop)
                waitNextG();
        }

        /**
 * @brief return true if the last boot was done after a sleep period or not (regular start) 
 * 
 * @return true 
 * @return false 
 */
        bool bootAfterSleep()
        {
            return esp_sleep_get_wakeup_cause() != 0;
        }

        /**
 * @brief wait a given time still listening for serial event to possibly stop the execution flow to display the interractive menu
 * the atomic wait time between serial checks will be defined by SANSENSNODE_WAITLOOPDELAYMS
 * 
 * @param waittimems 
 * @return true : we exit from the menu requesting to break the current loop
 * @return false regular end of the wait loop
 */
        bool waitListeningIOevents(unsigned int waittimems)
        {
            if (waittimems > 500)
                loginfo("wait");
            for (size_t i = 0; i < waittimems / SANSENSNODE_WAITLOOPDELAYMS; i++)
            {
                if (i % (1000 / SANSENSNODE_WAITLOOPDELAYMS) == 0)
                {
                    printf("."); //display 1 . every 500ms
                }
                if (_consolemenu->LoopCheckSerial() && _breakCurrentLoop)
                    return true;
                delay(SANSENSNODE_WAITLOOPDELAYMS);
            }
            return false;
        }

        /**
 * @brief wait for the next G time. If awakemode is false, go to deep sleep.
 * 
 */
        bool waitNextG()
        {
            if (_awakemode)
            {
                loginfo("Wait for %i s", _G_seconds);
                logflush();
                if (waitListeningIOevents(1000 * _G_seconds))
                    return true;
            }
            else
            {
                loginfo("Sleep for %i s\n", _G_seconds);
                logflush();
                SanSensNodeV2::DeepSleep();
            }
            return false;
        }

        // called to collect sensors data to be sent via mqtt
        void SetCollectDataCallback(std::function<bool(JsonColl&)> collectdatafunction)
        {
            _collectdataCallback = collectdatafunction;
        }

        //setup the callback called to setup the sensors
        void SetSetupDeviceCallback(std::function<void(SubMenu &)> setupdevicesfunction)
        {
            _setupdevicesCallback = setupdevicesfunction;
        }

        static void SetInputMessageCallback(std::function<void(flyingCollection::SanCodedStr const &)> inputmessagefunction)
        {
            _inputmessageCallback = inputmessagefunction;
        }

        /**
     * @brief go to deep sleep (with disabling wifi, bluetooth and so)
     * 
     */
        void DeepSleep()
        {
            loginfo("Going to sleep,bye.\n");
            logflush();
            // WiFi.disconnect(true);
            // WiFi.mode(WIFI_OFF);

            // btStop();
            // adc_power_off();
            // esp_wifi_stop();
            // esp_bt_controller_disable();

            if (_deepsleep)
            {
                _deepsleep->SetupTimerWakeup(_G_seconds);
                _deepsleep->GotoSleep();
            }
        }

        static char *getVersion()
        {
            sprintf(_sansensnodeversion, "%i.%i.%i", SANSENSNODE_VERSION_MAJOR, SANSENSNODE_VERSION_MINOR, SANSENSNODE_VERSION_REVISION);
            return _sansensnodeversion;
        }

        SubMenu *getDeviceMenu()
        {
            return this->_device_menu;
        }

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
        std::vector<DevicePlugin> _devices;

        bool
        mqttConnect()
        {
            uint8_t i = 0;
            // Loop until we're reconnected
            while (!mqttClient.connected())
            {
                loginfo("MQTT connection: ");
                // Attempt to connect
                if (!mqttClient.connect(_nodename))
                {
                    logwarning("failed, rc=%i\n", mqttClient.state());
                    if (++i >= _EXPmqttattemps)
                    {
                        logflush();
                        return false;
                    }
                }
                loginfo("OK\n");
                logflush();
            }
            return true;
        }

        //allow the client to process incoming messages and maintain its connection to the server.
        bool WaitforMqtt(int nb)
        {
            int l;
            for (int l = 0; l <= nb; l++)
            {
                if (!mqttClient.loop())
                { //This should be called regularly to allow the client to process incoming messages and maintain its connection to the server.
                    logdebug("WaitforMqtt : mqtt client disconnected\n");
                    return true; // the client is no longer connected
                }
                delay(10);
                // if (waitListeningIOevents(10))
                //     return false;
            }
            return true;
        }

        bool mqttSubscribe()
        {
            if (!mqttClient.connected())
            {
                logwarning("can't subscribe: no mqtt connection\n");
                logflush();
                return false;
            }
            std::string intopic(SanSensNodeV2::_mqttTopicBaseName);
            intopic.append(_nodename);
            intopic.append("/in");
            if (!mqttClient.subscribe(intopic.c_str(), 1))
            {
                logwarning("subscription to topic %s failed!\n", intopic.c_str());
                logflush();
                return false;
            }
            loginfo("mqtt subscription OK\n");
            logflush();
            return true;
        }

        bool collectMeasurement(JsonColl *dc)
        {
            if (dc && !collectMeasurement_internal(*dc))
                return false;

            for (auto &dev : _devices)
            {
                dev.collectdata(*dc);
            }

            if (_collectdataCallback && !_collectdataCallback(*dc))
                return false;

            return true;
        }

        // publish and subscribe to the respective MQTT chanels (open the connection and close after)
        bool mqttpubsub(JsonColl *dc)
        {
            logdebug("enter mqttpubsub\n");

            _mqttpayloadLength = 0;
            uint32_t m0 = millis();
            if (!Setup_wifi())
                return false;
            if (!mqttClient.connected())
                if (!mqttConnect())
                    return false;
            if (_mqttsubscribe)
                mqttSubscribe(); // todo : action en cas de défaut de souscription??

            // WaitforMqtt(15); // to wait for incoming messages

            if (dc)
            {
                if (_measurementAttenmpts >= _maxMeasurementAttenmpts)
                    dc->add("SensorIssue", true);

                std::string outopic(SanSensNodeV2::_mqttTopicBaseName);
                outopic.append(_nodename);
                outopic.append("/out");
                logdebug("publish on %s\n", outopic.c_str());

                auto vecres = dc->getJsons();
                logdebug("nb jsons strings to publish: %i\n", vecres.size());
                int i = 1;
                for (const std::string &json : vecres)
                {
                    if (!mqttClient.publish(outopic.c_str(), json.c_str(), true))
                    {
                        logerror("publish failed at %i\n", i);
                        break;
                    }
                    if (!WaitforMqtt(_waitforMqttSend))
                        return false;
                    logdebug("publish pass %i OK : %s\n", i, json.c_str());
                }

                _Pi++;
                logflush();
            }
            if (_mqttsubscribe)
            {
                logdebug("wait for incomming mqtt message\n");
                if (!WaitforMqtt(_waitforMqttReceive))
                    return false;
            }
            wifiOff();
            logflush();
            HandleMqttReceive();

            loginfo("wifi was on for %ims\n", millis() - m0);
            logdebug("exit mqttpubsub\n");
            logflush();
            return true;
        }
        /**
 * @brief setup the WIFi connection
 * 
 * @return true 
 * @return false issue or relaod required from the IHM, need to exit
 */
        bool Setup_wifi()
        {
            delay(10);

            logdebug("Connecting to %s, wifi mode=%i\n", _ssid, _wifiMode);
            logflush();

            WiFi.begin(_ssid, _password);
            WiFi.mode((wifi_mode_t)_wifiMode);
            int nb = 0;
            while (WiFi.status() != WL_CONNECTED && nb++ < _wifitrialsmax)
            {
                if (waitListeningIOevents(_EXPwifiwait))
                    return false;
            }
            if (WiFi.status() != WL_CONNECTED)
            {
                logerror("Wifi not connected, expired wifitrialsmax\n");
                logflush();
                if (waitNextG())
                    return false;
            }

            loginfo("WiFi connected, IP address: ");
            Serial.print(WiFi.localIP());
            Serial.print(",\n");
            logflush();

            mqttClient.setServer(_mqtt_server, SANSENSNODE_MQTTPORT);
            mqttClient.setCallback(mqttCallback);

            return true;
        }

        void wifiOff()
        {
            WiFi.disconnect(true);
            WiFi.mode(WIFI_OFF);
        }

        void setupSerialMenu()
        {
            _consolemenu = new Menu<26>(); // menu-entries in this class ( 2 more in the sketch) (todo : to template)
            //define options
            MenuOptions menuoptions;
            menuoptions.addBack = true;
            menuoptions.addExitForEachLevel = true;
            menuoptions.expirationTimeSec = 120;
            _consolemenu->setOptions(menuoptions);
            // menus & submenus definition
            // root menus

            SubMenu *root = _consolemenu->getRootMenu();
            _device_menu = root->addSubMenu("device");
            SubMenu *measure_menu = root->addSubMenu("measure")->addCallbackToChilds(breakLoop);
            SubMenu *publication_menu = root->addSubMenu("publication")->addCallbackToChilds(breakLoop);
            SubMenu *infos_menu = root->addSubMenu("infos");

            publication_menu->addMenuitemUpdater("awake mode", &_awakemode);
            publication_menu->addMenuitemUpdater("verbose mode", &_verboseMode);

            measure_menu->addMenuitemUpdater("set G", &_G_seconds);
            measure_menu->addMenuitemUpdater("measure trails max nb", &_maxMeasurementAttenmpts);

            publication_menu->addMenuitemUpdater("set P", &_Pfactor);
            publication_menu->addMenuitemUpdater("wait mqtt send", &_waitforMqttSend);
            publication_menu->addMenuitemUpdater("wait mqtt receive", &_waitforMqttReceive);
            publication_menu->addMenuitemUpdater("MQTT subscribe?", &_mqttsubscribe);
            publication_menu->addMenuitemUpdater("wifiwait", &_EXPwifiwait);
            publication_menu->addMenuitemUpdater("wifiattemps", &_EXPmqttattemps);
            publication_menu->addMenuitemUpdater("wifi mode (1 to 4)", &_wifiMode);

            SubMenu *smfreq = _device_menu->addSubMenu("set CPU freq")->addCallbackToChilds(SetEnergyMode)->addCallbackToChilds(breakLoop);
            smfreq->addMenuitem()->SetLabel("80 Mhz")->addLambda([]() { _cpuFreq = 80; });
            smfreq->addMenuitem()->SetLabel("160 Mhz")->addLambda([]() { _cpuFreq = 160; });
            smfreq->addMenuitem()->SetLabel("240 Mhz")->addLambda([]() { _cpuFreq = 240; });

            infos_menu->addMenuitemCallback("build infos", buildInfos);
            infos_menu->addMenuitem()->SetLabel("RT infos")->addLambda([this]() { rtInfos(); });
            // infos_menu->addMenuitem()->SetLabel("RT infos")->addLambda([_consolemenu]() { rtInfos(_consolemenu); });
            // infos_menu->addMenuitemCallback("RT infos", rtInfos);
            infos_menu->addMenuitemUpdater("Log level (0=off->5=debug)", &_loglevel)->addLambda([]() { loglevel((log_level_e)_loglevel); });
        }

        bool collectMeasurement_internal(JsonColl &dc)
        {
            //collect data and fill _datacollector with it
            dc.add("device", _nodename);
            dc.add("Gi", _Gi);
            dc.add("Pi", _Pi);
            double timeSinceStartup = esp_timer_get_time() / 1000000;
            dc.add("uptime", timeSinceStartup);
            if (_verboseMode)
            {
                dc.add("G_span", _G_seconds);
                dc.add("P_nb", _Pfactor);
                dc.add("freeram", (int)heap_caps_get_free_size(MALLOC_CAP_8BIT));
                dc.add("boot count", _bootCount);
                // dc.add("cpumhz", _cpuFreq);
                // dc.add("Serial", _serial);
                // dc.add("wifitrialsmax", _wifitrialsmax);
                // dc.add("mqttattemps", _EXPmqttattemps);
            }
            return true;
        }

        static bool mqttCallback(char *topic, uint8_t *payload, unsigned int length)
        {
            for (int i = 0; i < length; i++)
            {
                _mqttpayload[i] = (char)payload[i];
            }
            _mqttpayload[length] = '\0';
            _mqttpayloadLength = length;
            return true;
        }

        void HandleMqttReceive()
        {
            if (_mqttpayloadLength == 0)
            {
                logdebug("no mqtt message received\n");
                logflush();
                return;
            }

            logdebug("%i=Message received of length\n", _mqttpayloadLength);
            logdebug("incomming mess:%s\n", _mqttpayload); //to delete
            logflush();
            bool asleep;

            flyingCollection::SanCodedStr pyldic(_mqttpayload);
            logdebug("SanCodedStrings initialized\n"); //to delete
            logflush();
            if (pyldic.tryGetValue("sleep", asleep))
            {
                _awakemode = !asleep;
                logdebug("sleep:%i\n", _awakemode);
            }

            if (pyldic.tryGetValue("serial", _serial))
            {
                logdebug("serial:%i\n", _serial);
                if (_serial)
                    Serial.begin(115200);
                else
                    Serial.end();
            }
            if (pyldic.tryGetValue("G", _G_seconds))
            {
                logdebug("G:%is\n", _G_seconds);
            }
            if (pyldic.tryGetValue("P", _Pfactor))
            {
                logdebug("P:%i x G(=%is)\n", _Pfactor, _G_seconds);
            }

            for (auto &dev : _devices)
            {
                dev.onInputMessage(pyldic);
            }

            if (_inputmessageCallback)
                _inputmessageCallback(pyldic);
        }

        static bool buildInfos()
        {
#ifdef SANSENSNODE_SKETCHVERSION
            printf("sketch V:%s\n", SANSENSNODE_SKETCHVERSION);
#endif
            printf("SanSensNodeV2 V:%s", getVersion());
            printf("consoleMenu V:%s", Menubase::getVersion());
            printf("__GNUG__:%i", __GNUG__);
            printf("__cplusplus:%i", __cplusplus);
            printf("__TIMESTAMP__:%i", __TIMESTAMP__);
            logflush();
            return false;
        }

        void rtInfos() //void rtInfos(Menubase *cm)
        {
            printf("cpu freq:%i\n", getCpuFrequencyMhz());
            // printf("buffer capacity:%i\n", _lastbuffercapacity);
            printf("console menu holds %i items\n", _consolemenu->size());
            logflush();
        };

        static bool SetEnergyMode()
        {
            setCpuFrequencyMhz(_cpuFreq);
            return true;
        }
        static bool breakLoop()
        {
            _breakCurrentLoop = true;
            return true;
        }
    };
} // namespace SANSENSNODE_NAMESPACE
