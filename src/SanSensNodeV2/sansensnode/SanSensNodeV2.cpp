#include <Arduino.h>
#include "../Configuration.h"
#include "../Namespace.h"
#include <flyingCollection.h>
#include "specialTypes.h"
#include <WiFi.h>
#include "../platform_logger.h"
#include <iterator>
#include "SanSensNodeV2.h"

namespace SANSENSNODE_NAMESPACE
{

    static char _menuswitch_buff[SANSENSNODE_STRING_BUFFER_SIZE];
    static char _sansensnodeversion[10];
    static char _mqttpayload[SANSENSNODE_MQTT_RECEIVEMESSAGE_SIZE];
    static uint16_t _mqttpayloadLength = 0;
    static bool _breakCurrentLoop = false;

    static WiFiClient __espClient;
    static RTC_DATA_ATTR int _bootCount = 0;                                                     // store "boot" (restart after sleep) counting (in the RTC resilient RAM memory)
    static RTC_DATA_ATTR bool _firstinit = true;                                                 //
    static RTC_DATA_ATTR bool _awakemode;                                                        // while in awake mode => don't go to sleep on each G cycle until a sleep order has been received
    static RTC_DATA_ATTR bool _mqttsubscribe;                                                    // if not subscribe : doesn't respond to mqtt event (only send to server)s
    static RTC_DATA_ATTR uint16_t _G_seconds;                                                    // measurement cycle time ie: sleep time (in seconds) (in awake mode and sleep mode)
    static RTC_DATA_ATTR uint16_t _Pfactor;                                                      // publication and mqtt connection frequency (in G multiples)
    static RTC_DATA_ATTR bool _serial;                                                           // false to disable output serial log
    static RTC_DATA_ATTR uint8_t _cpuFreq = SANSENSNODE_STARTINGCPUFREQ;                         // CPU operating frequency
    static RTC_DATA_ATTR uint8_t _wifitrialsmax;                                                 // nb max of attemps to check the wifi network for a wakeup cycle
    static RTC_DATA_ATTR uint32_t _Gi;                                                           // store the current cycle index (G or measurement cycle)
    static RTC_DATA_ATTR uint32_t _Pi;                                                           // store the current publication index (P)
    static RTC_DATA_ATTR uint16_t _EXPwifiwait;                                                  // EXPERIMENTAL : temps d'attente suite allmuage wifi pour que le MQTT puisse répondre
    static RTC_DATA_ATTR uint16_t _waitforMqttSend = SANSENSNODE_MQTT_WAITFORMQTTSENDLOOP;       // nb of loop to wait for mqtt send to server
    static RTC_DATA_ATTR uint16_t _waitforMqttReceive = SANSENSNODE_MQTT_WAITFORMQTTRECEIVELOOP; // nb of loop to wait for mqtt receive from server
    static RTC_DATA_ATTR uint8_t _EXPmqttattemps;                                                // EXPERIMENTAL : nb de tentatives pour connexion au server mqtt
    static RTC_DATA_ATTR uint8_t _wifiMode = 4;                                                  // WIFI MODE :    0=WIFI_MODE_NULL (no WIFI),1=WIFI_MODE_STA (WiFi station mode),2=WIFI_MODE_AP (WiFi soft-AP mode)
    static RTC_DATA_ATTR uint8_t _loglevel = LOG_LEVEL;                                          // log level : 0=Off, 1=Critical, 2=Error, 3=Warning, 4=Info, 5=Debug
    static RTC_DATA_ATTR char *_mqttTopicBaseName{nullptr};
    // 3=WIFI_MODE_APSTA (WiFi station + soft-AP mode) 4=WIFI_MODE_MAX
    static RTC_DATA_ATTR char *_nodename{nullptr};
    static RTC_DATA_ATTR char *_ssid{nullptr};
    static RTC_DATA_ATTR char *_password{nullptr};
    static RTC_DATA_ATTR char *_mqtt_server{nullptr};

    static RTC_DATA_ATTR bool _verboseMode;
    static RTC_DATA_ATTR bool _menuEnabled = true;
    static RTC_DATA_ATTR uint16_t _EXPmodetestmqttmessagestart = 100;

    static RTC_DATA_ATTR uint8_t _maxMeasurementAttenmpts;

    static std::function<void(flyingCollection::SanCodedStr const &)> _inputmessageCallback;
    static std::function<bool(JsonColl &)> _collectdataCallback;
    static std::function<void(SubMenu &)> _setupdevicesCallback;

    static std::vector<SensorPlugin *> _sensors;

    const uint16_t _jsonoutbuffersize = 150;

    SanSensNodeV2::SanSensNodeV2(const char *nodename, const char *ssid, const char *wifipasswd, const char *mqttserver,const char* mqtttopicbasename, int G, int Pfactor)
    {
        if (_firstinit) // première initialisation, ensuite on rentre dans ce constructeur à chaque reveil donc on ne doit pas réinitialiser les variables stockées dnas la RAM de la RTC
        {
            logdebug("enter SanSensNodeV2 ctor first init\n");
            _awakemode = SANSENSNODE_STARTSAWAKEN;

            _serial = true;
            _Gi = 0;
            _Pi = 0;
            _EXPwifiwait = SANSENSNODE_WIFIWAITTIMEMS;
            _EXPmqttattemps = SANSENSNODE_MQTT_ATTEMPTSNB;
            _verboseMode = false;
            _nodename = (char *)nodename;
            _ssid = (char *)ssid;
            _password = (char *)wifipasswd;
            _mqtt_server = (char *)mqttserver;
            _maxMeasurementAttenmpts = SANSENSNODE_MAX_MEASURES_ATTEMPTS;
            _G_seconds = G;
            _Pfactor = Pfactor;
            _wifitrialsmax = SANSENSNODE_WIFITRIALSINIT;
            _mqttsubscribe = SANSENSNODE_MQTT_SUBSCRIBEATSTART;
            _mqttTopicBaseName = (char *)mqtttopicbasename;
        }
        _sensors.clear();
        _sensors.reserve(4);

        loglevel((log_level_e)_loglevel);
        this->mqttClient.setClient(__espClient);
        if (_menuEnabled)
            setupSerialMenu();
        ++_bootCount;
        if (_serial)
        {
            delay(SANSENSNODE_WAITFORSERIALDELAY); // pour attendre port serie
            Serial.begin(115200);
        }
        logdebug("end SanSensNodeV2 ctor,_bootCount=%i\n", _bootCount);
        logflush();
    }

    void SanSensNodeV2::addDevice(SensorPlugin *device)
    {
        logdebug("add device [%s], addr:%x\n ", device->getSensorName(), device);
        _sensors.push_back(device);
        device->hookSanSensInstance(this);
    }

    void SanSensNodeV2::Setup()
    {

        SetEnergyMode();

        _deepsleep = new SANSENSNODE_NAMESPACE::DeepSleep();
        _deepsleep->SetupTouchpadWakeup(SANSENSNODE_TOUCHPADTHRESHOLD, SANSENSNODE_TOUCHPADGPIO);

        if (_serial)
        {
            loginfo("#boot=%i\n", _bootCount);
            logdebug("#awakemode:%i\n", _awakemode);
            logdebug("G duration = %i\n", _G_seconds);
            logdebug("P factor = %i\n", _Pfactor);
            logdebug("WIFI mode = %i\n", _wifiMode);
            logflush();
        }

     
        // setup the sensors taken from the collection

        for (auto sensor : _sensors)
        {
            if (sensor)
            {
                // logdebug("EXP %i entering setupdevice on %s\n", i, dev->getDeviceName());
                logdebug("EXP entering setupdevice on %s\n", sensor->getSensorName());
                if (_firstinit)
                    sensor->firstSetup();
                if (_sensors_menu)
                {
                    SubMenu *sensormenu = _sensors_menu->addSubMenu(sensor->getSensorName());
                    if (sensormenu)
                    {
                        sensormenu->addMenuitemUpdater("enabled", &(sensor->enabled))->addCallback(breakLoop);
                        sensor->setMenu(*sensormenu);
                        //menu sensor enbling/disabling
                        // _sensors_menu->addMenuitem()
                        //     ->SetDynLabel([&, sensor]() { return sensor->enableMenuFunctionName(); })
                        //     ->addLambda([&, sensor]() {
                        //         sensor->enabled = !sensor->enabled;
                        //         if (sensor->enabled)
                        //             sensor->setupdevice(*_sensors_menu);
                        //     });
                    }
                }
                if (sensor->enabled)
                    sensor->setupsensor();
            }
        }

        logdebug("entering _setupdevicesCallback\n");
        if (_setupdevicesCallback)
            _setupdevicesCallback(*_sensors_menu);

        if (_firstinit || (_deepsleep && (_deepsleep->getWakeup_reason() == ESP_SLEEP_WAKEUP_TOUCHPAD)))
        {
            printf("input anything to show the menu. Expire in %i seconds", SANSENSNODE_FIRSTBOOTDELAYWAITINGMENU);
            waitListeningIOevents(SANSENSNODE_FIRSTBOOTDELAYWAITINGMENU * 1000);
        }

        if (_firstinit)
            _firstinit = false;

        logdebug("end SanSensNode setup\n");
        logflush();
    }

    SanSensNodeV2::~SanSensNodeV2()
    {
        printf("~SanSensNodeV2\n");
    }

    void SanSensNodeV2::Loop()
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

    bool SanSensNodeV2::bootAfterSleep()
    {
        return esp_sleep_get_wakeup_cause() != 0;
    }

    /**
 * @brief true if hte start is issued by a true start (or reset), false if its a wakeup after a deepsleep (in this case, we can recover some state from the RTC RAM)
 * 
 * @return true 
 * @return false 
 */
    bool SanSensNodeV2::isFirstInit()
    {
        return _firstinit;
    }

    bool SanSensNodeV2::waitListeningIOevents(unsigned int waittimems)
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
            {
                if (!_menuEnabled)
                {
                    _menuEnabled = true;
                    setupSerialMenu();
                }
                return true;
            }
            delay(SANSENSNODE_WAITLOOPDELAYMS);
        }
        return false;
    }

    bool SanSensNodeV2::waitNextG()
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

    void SanSensNodeV2::SetCollectDataCallback(std::function<bool(JsonColl &)> collectdatafunction)
    {
        _collectdataCallback = collectdatafunction;
    }

    //setup the callback called to setup the sensors
    void SanSensNodeV2::SetSetupDeviceCallback(std::function<void(SubMenu &)> setupdevicesfunction)
    {
        _setupdevicesCallback = setupdevicesfunction;
    }

    void SanSensNodeV2::SetInputMessageCallback(std::function<void(flyingCollection::SanCodedStr const &)> inputmessagefunction)
    {
        _inputmessageCallback = inputmessagefunction;
    }

    void SanSensNodeV2::DeepSleep()
    {
        loginfo("Going to sleep,bye.\n");
        logflush();

        if (_deepsleep)
        {
            _deepsleep->SetupTimerWakeup(_G_seconds);
            _deepsleep->GotoSleep();
        }
    }

    char *SanSensNodeV2::getVersion()
    {
        sprintf(_sansensnodeversion, "%i.%i.%i", SANSENSNODE_VERSION_MAJOR, SANSENSNODE_VERSION_MINOR, SANSENSNODE_VERSION_REVISION);
        return _sansensnodeversion;
    }

    SubMenu *SanSensNodeV2::getSensorsMenu()
    {
        return this->_sensors_menu;
    }

    /*
private methods

*/
    bool SanSensNodeV2::mqttConnect()
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
    bool SanSensNodeV2::WaitforMqtt(int nb)
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

    bool SanSensNodeV2::mqttSubscribe()
    {
        logdebug("enter mqttSubscribe\n");
        if (!mqttClient.connected())
        {
            logwarning("can't subscribe: no mqtt connection\n");
            logflush();
            return false;
        }

        std::string intopic(_mqttTopicBaseName);
        intopic.append(_nodename);
        intopic.append(SANSENSNODE_MQTT_INTOPICSUFFIX);
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

    bool SanSensNodeV2::collectMeasurement(JsonColl *dc)
    {
        if (dc && !collectMeasurement_internal(*dc))
            return false;

        for (auto sensor : _sensors)
        {
            if (sensor && sensor->enabled)
            {
                logdebug("entering collectdata on %s\n", sensor->getSensorName());
                sensor->collectdata(*dc);
            }
        }

        if (_collectdataCallback && !_collectdataCallback(*dc))
            return false;

        return true;
    }

    // publish and subscribe to the respective MQTT chanels (open the connection and close after)
    bool SanSensNodeV2::mqttpubsub(JsonColl *dc)
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
            logdebug("a jsoncoll is ready to be sent\n");
            if (_measurementAttenmpts >= _maxMeasurementAttenmpts)
                dc->add("SensorIssue", true);

            std::string outopic(_mqttTopicBaseName);
            outopic.append(_nodename);
            outopic.append(SANSENSNODE_MQTT_OUTTOPICSUFFIX);
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

    void mqttCallback(char *topic, uint8_t *payload, unsigned int length)
    {
        for (int i = 0; i < length; i++)
        {
            _mqttpayload[i] = (char)payload[i];
        }
        _mqttpayload[length] = '\0';
        _mqttpayloadLength = length;
    }

    bool SanSensNodeV2::Setup_wifi()
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

        loginfo("WiFi connected, IP address:%s", WiFi.localIP().toString().c_str());
        logflush();

        mqttClient.setServer(_mqtt_server, SANSENSNODE_MQTT_PORT);
        mqttClient.setCallback(mqttCallback);

        return true;
    }

    void SanSensNodeV2::wifiOff()
    {
        WiFi.disconnect(true);
        WiFi.mode(WIFI_OFF);
    }

    void SanSensNodeV2::setupSerialMenu()
    {
        _consolemenu = new Menu<38 + SANSENSNODE_SENSORS_EXP_SENSORSARRSIZE>(); // menu-entries in this class ( 2 more in the sketch) (todo : to template)
        //define options
        MenuOptions menuoptions;
        menuoptions.addBack = true;
        menuoptions.addExitForEachLevel = true;
        menuoptions.expirationTimeSec = 120;
        _consolemenu->setOptions(menuoptions);
        // menus & submenus definition
        // root menus

        SubMenu *root = _consolemenu->getRootMenu();
        SubMenu *device_menu = root->addSubMenu("device");
        _sensors_menu = root->addSubMenu("sensors");
        SubMenu *publication_menu = root->addSubMenu("publication")->addCallbackToChilds(breakLoop);
        SubMenu *network_menu = root->addSubMenu("network")->addCallbackToChilds(breakLoop);
        SubMenu *infos_menu = root->addSubMenu("infos");

        publication_menu->addMenuitemUpdater("awake mode", &_awakemode);
        publication_menu->addMenuitemUpdater("verbose mode", &_verboseMode);

        device_menu->addMenuitemUpdater("set G", &_G_seconds);
        device_menu->addMenuitemUpdater("measure fails retry nb", &_maxMeasurementAttenmpts);

        publication_menu->addMenuitemUpdater("set P", &_Pfactor);
        publication_menu->addMenuitemUpdater("wait mqtt send", &_waitforMqttSend);
        publication_menu->addMenuitemUpdater("wait mqtt receive", &_waitforMqttReceive);
        publication_menu->addMenuitemUpdater("MQTT subscribe?", &_mqttsubscribe);
        publication_menu->addMenuitemUpdater("wifiwait", &_EXPwifiwait);
        publication_menu->addMenuitemUpdater("wifiattemps", &_EXPmqttattemps);
        publication_menu->addMenuitemUpdater("wifi mode (1 to 4)", &_wifiMode);

        device_menu->addMenuitemUpdater("control menu", &_menuEnabled);

        SubMenu *smfreq = device_menu->addSubMenu("set CPU freq")->addCallbackToChilds(SetEnergyMode)->addCallbackToChilds(breakLoop);
        smfreq->addMenuitem()->SetLabel("80 Mhz")->addLambda([]() { _cpuFreq = 80; });
        smfreq->addMenuitem()->SetLabel("160 Mhz")->addLambda([]() { _cpuFreq = 160; });
        smfreq->addMenuitem()->SetLabel("240 Mhz")->addLambda([]() { _cpuFreq = 240; });

        infos_menu->addMenuitemCallback("build infos", buildInfos);
        infos_menu->addMenuitem()->SetLabel("RT infos")->addLambda([this]() { rtInfos(); });
        // infos_menu->addMenuitem()->SetLabel("RT infos")->addLambda([_consolemenu]() { rtInfos(_consolemenu); });
        // infos_menu->addMenuitemCallback("RT infos", rtInfos);
        infos_menu->addMenuitemUpdater("Log level (0=off->5=debug)", &_loglevel)->addLambda([]() { loglevel((log_level_e)_loglevel); });
        network_menu->addMenuitemUpdater("node name", _nodename, 20);
        network_menu->addMenuitemUpdater("wifi ssid", _ssid, 30);
        network_menu->addMenuitemUpdater("wifi password", _password, 30);
        network_menu->addMenuitemUpdater("mqtt server", _mqtt_server, 15);
        network_menu->addMenuitemUpdater("mqtt topic base", _mqttTopicBaseName, 15);
    }

    bool SanSensNodeV2::collectMeasurement_internal(JsonColl &dc)
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
        }
        return true;
    }

    void SanSensNodeV2::HandleMqttReceive()
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

        for (auto sensor : _sensors)
        {
            if (sensor && sensor->enabled)
            {
                logdebug("onInputMessage on %s\n", sensor->getSensorName());
                sensor->onInputMessage(pyldic);
            }
        }

        if (_inputmessageCallback)
            _inputmessageCallback(pyldic);
    }

    bool SanSensNodeV2::buildInfos()
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

    void SanSensNodeV2::rtInfos()
    {
        printf("cpu freq:%i\n", getCpuFrequencyMhz());
        // printf("buffer capacity:%i\n", _lastbuffercapacity);
        printf("console menu holds %i items\n", _consolemenu->size());
        logflush();
    };

    bool SanSensNodeV2::SetEnergyMode()
    {
        setCpuFrequencyMhz(_cpuFreq);
        return true;
    }

    bool SanSensNodeV2::breakLoop()
    {
        _breakCurrentLoop = true;
        return true;
    }
} // namespace SANSENSNODE_NAMESPACE