#pragma once
#include "DeepSleep.hpp"

namespace SANSENSNODE_NAMESPACE
{

typedef bool (*pf_callback_collectdata)(SanDataCollector *);
typedef void (*pf_callback_setupdevices)(void);
typedef bool (*pf_callback_inputmessage)(SanCodedStrings);

static char _menuswitch_buff[SANSENSNODE_STRING_BUFFER_SIZE];
static char _sansensnodeversion[10];
static char _mqttpayload[SANSENSNODE_MQTTRECEIVEMESSAGE_SIZE];
static uint16_t _mqttpayloadLength = 0;
static bool _breakCurrentLoop = false;

static WiFiClient __espClient;
static RTC_DATA_ATTR int _bootCount = 0;                                                // store "boot" (restart after sleep) counting (in the RTC resilient RAM memory)
static RTC_DATA_ATTR bool _firstinit = false;                                           //
static RTC_DATA_ATTR bool _awakemode;                                                   // while in awake mode => don't go to sleep on each G cycle until a sleep order has been received
static RTC_DATA_ATTR bool _mqttsubscribe;                                               // if not subscribe : doesn't respond to mqtt event (only send to server)s
static RTC_DATA_ATTR uint16_t _G_seconds;                                               // measurement cycle time ie: sleep time (in seconds) (in awake mode and sleep mode)
static RTC_DATA_ATTR uint16_t _Pfactor;                                                 // publication and mqtt connection frequency (in G multiples)
static RTC_DATA_ATTR bool _serial;                                                      // false to disable output serial log
static RTC_DATA_ATTR uint8_t _cpuFreq = SANSENSNODE_STARTINGCPUFREQ;                    // CPU operating frequency
static RTC_DATA_ATTR uint8_t _wifitrialsmax;                                            // nb max of attemps to check the wifi network for a wakeup cycle
static RTC_DATA_ATTR uint32_t _Gi;                                                      // store the current cycle index (G or measurement cycle)
static RTC_DATA_ATTR uint32_t _Pi;                                                      // store the current publication index (P)
static RTC_DATA_ATTR uint16_t _EXPwifiwait;                                             // EXPERIMENTAL : temps d'attente suite allmuage wifi pour que le MQTT puisse répondre
static RTC_DATA_ATTR uint16_t _waitforMqttSend = SANSENSNODE_WAITFORMQTTSENDLOOP;       // nb of loop to wait for mqtt send to server
static RTC_DATA_ATTR uint16_t _waitforMqttReceive = SANSENSNODE_WAITFORMQTTRECEIVELOOP; // nb of loop to wait for mqtt receive from server
static RTC_DATA_ATTR uint8_t _EXPmqttattemps;                                           // EXPERIMENTAL : nb de tentatives pour connexion au server mqtt
static RTC_DATA_ATTR uint8_t _wifiMode = 4;                                             // WIFI MODE :    0=WIFI_MODE_NULL (no WIFI),1=WIFI_MODE_STA (WiFi station mode),2=WIFI_MODE_AP (WiFi soft-AP mode)
static RTC_DATA_ATTR uint8_t _loglevel = LOG_LEVEL;                                     // log level : 0=Off, 1=Critical, 2=Error, 3=Warning, 4=Info, 5=Debug
                                                                                        // 3=WIFI_MODE_APSTA (WiFi station + soft-AP mode) 4=WIFI_MODE_MAX
static RTC_DATA_ATTR const char *_nodename{nullptr};
static RTC_DATA_ATTR const char *_ssid{nullptr};
static RTC_DATA_ATTR const char *_password{nullptr};
static RTC_DATA_ATTR const char *_mqtt_server{nullptr};
static RTC_DATA_ATTR uint8_t _EXPtestduplicatepublish = 1; // EXPERIMENTAL :

static RTC_DATA_ATTR bool _verboseMode;
static RTC_DATA_ATTR uint8_t _maxMeasurementAttenmpts;

static pf_callback_inputmessage _inputmessageCallback;

const uint16_t _jsonoutbuffersize = 150;

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

        if (!_firstinit) // première initialisation, ensuite on rentre dans ce constructeur à chaque reveil donc on ne doit pas réinitialiser les variables stockées dnas la RAM de la RTC
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
            _firstinit = true;
            _mqttsubscribe = SANSENSNODE_MQTTSUBSCRIBEATSTART;
        }
        loglevel((log_level_e)_loglevel);
        this->client.setClient(__espClient);
        setupSerialMenu();
        ++_bootCount;
        logdebug("end SanSensNodeV2 ctor\n");
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

        if (_setupdevicesCallback)
            _setupdevicesCallback();
        else
            logdebug("no _setupdevicesCallback defined\n");
        logflush();

        if (_deepsleep && (_deepsleep->getWakeup_reason() == ESP_SLEEP_WAKEUP_TOUCHPAD))
        {
            logdebug("awake by touchpad => menu\n");
            _consolemenu->launchMenu();
        }
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

        SanDataCollector *datacollector{nullptr};
        datacollector = new SanDataCollector();

        logdebug("measure start\n");
        while (!SanSensNodeV2::collectMeasurement(datacollector) && _measurementAttenmpts <= _maxMeasurementAttenmpts)
        {
            _measurementAttenmpts++;
        }
        logdebug("measure end\n");
        logflush();

        if (_wifiMode > 0 && datacollector && _Gi % _Pfactor == 0)
            SanSensNodeV2::mqttpubsub(datacollector);
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
    void SetCollectDataCallback(pf_callback_collectdata collectdatafunction)
    {
        _collectdataCallback = collectdatafunction;
    }

    //setup the callback called to setup the sensors
    void SetSetupDeviceCallback(pf_callback_setupdevices setupdevicesfunction)
    {
        _setupdevicesCallback = setupdevicesfunction;
    }
    static void SetInputMessageCallback(pf_callback_inputmessage inputmessagefunction)
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
    PubSubClient client;
    bool initStringValue(const char *menuname);
    bool DisplayStringValue(const char *menuname);
    Menubase *_consolemenu;
    SubMenu *_device_menu;
    SANSENSNODE_NAMESPACE::DeepSleep *_deepsleep;
    uint8_t _measurementAttenmpts;
    pf_callback_collectdata _collectdataCallback;
    pf_callback_setupdevices _setupdevicesCallback;
    const char *_mqttTopicBaseName{"/ssnet/"};
    const char *_lostTopic{"/ssnet/lost"}; // not implemented : when the sensor has not been initialized, it wait configuration data from this topic

    bool mqttConnect()
    {
        uint8_t i = 0;
        // Loop until we're reconnected
        while (!client.connected())
        {
            loginfo("MQTT connection: ");
            // Attempt to connect
            if (!client.connect(_nodename))
            {
                logwarning("failed, rc=%i\n", client.state());
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
            if (!client.loop())
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
        if (!client.connected())
        {
            logwarning("can't subscribe: no mqtt connection\n");
            logflush();
            return false;
        }
        std::string intopic(SanSensNodeV2::_mqttTopicBaseName);
        intopic.append(_nodename);
        intopic.append("/in");
        if (!client.subscribe(intopic.c_str(), 1))
        {
            logwarning("subscription to topic %s failed!\n", intopic.c_str());
            logflush();
            return false;
        }
        loginfo("mqtt subscription OK\n");
        logflush();
        return true;
    }

    bool collectMeasurement(SanDataCollector *dc)
    {
        if (dc && !collectMeasurement_internal(dc))
            return false;
        if (_collectdataCallback)
        {
            if (!_collectdataCallback(dc))
                return false;
        }
        else
            logdebug("no collectdata Callback defined\n");

        return true;
    }

    // publish and subscribe to the respective MQTT chanels (open the connection and close after)
    bool mqttpubsub(SanDataCollector *dc)
    {
        logdebug("enter mqttpubsub\n");

        _mqttpayloadLength = 0;
        uint32_t m0 = millis();
        if (!Setup_wifi())
            return false;
        if (!client.connected())
            if (!mqttConnect())
                return false;
        if (_mqttsubscribe)
            mqttSubscribe(); // todo : action en cas de défaut de souscription??

        // WaitforMqtt(15); // to wait for incoming messages
        if (dc)
        {
            if (_measurementAttenmpts >= _maxMeasurementAttenmpts)
                dc->Add("SensorIssue", true);

            char buffer[_jsonoutbuffersize];
            size_t size = dc->Serialize(buffer, _jsonoutbuffersize);

            std::string outopic(SanSensNodeV2::_mqttTopicBaseName);
            outopic.append(_nodename);
            outopic.append("/out");
            logdebug("publish on %s, taille message: %i\n", outopic.c_str(), (int)size);
            for (uint8_t i = 0; i < _EXPtestduplicatepublish; i++)
            {
                if (!client.publish(outopic.c_str(), buffer, true))
                {
                    logerror("publish failed at %i\n", i);
                    break;
                }
                if (!WaitforMqtt(_waitforMqttSend))
                    return false;
                logdebug("publish pass %i\n", i);
            }
            _Pi++;
            loginfo("message: %s, publication mqtt OK\n", (char *)buffer);
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

        client.setServer(_mqtt_server, SANSENSNODE_MQTTPORT);
        client.setCallback(mqttCallback);

        return true;
    }

    void wifiOff()
    {
        WiFi.disconnect(true);
        WiFi.mode(WIFI_OFF);
    }

    void setupSerialMenu()
    {
        _consolemenu = new Menu<27>(); // menu-entries in this class ( 2 more in the sketch) (todo : to template)
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
        publication_menu->addMenuitemUpdater("duplicate publication", &_EXPtestduplicatepublish);

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

    bool collectMeasurement_internal(SanDataCollector *dc)
    {
        if (!dc)
            return false;
        //collect data and fill _datacollector with it
        dc->Add("device", _nodename);
        dc->Add("Gi", _Gi);
        dc->Add("Pi", _Pi);
        double timeSinceStartup = esp_timer_get_time() / 1000000;
        dc->Add("uptime", timeSinceStartup);
        if (_verboseMode)
        {
            dc->Add("G_span", _G_seconds);
            dc->Add("P_nb", _Pfactor);
            dc->Add("freeram", (int)heap_caps_get_free_size(MALLOC_CAP_8BIT));
            dc->Add("boot count", _bootCount);
            // dc->Add("cpumhz", _cpuFreq);
            // dc->Add("Serial", _serial);
            // dc->Add("wifitrialsmax", _wifitrialsmax);
            // dc->Add("mqttattemps", _EXPmqttattemps);
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
        // String a;
        // char *myvalue;
        // bool asleep;

        // SanCodedStrings pyldic(_mqttpayload);
        // logdebug("SanCodedStrings initialized\n"); //to delete
        // logflush();

        // if (pyldic.TryParseValue_b("sleep", &asleep))
        // {
        //     _awakemode = !asleep;
        //     logdebug("sleep:%i\n", _awakemode);
        // }
        // if (pyldic.TryParseValue_b("serial", &_serial))
        // {
        //     logdebug("serial:%i\n", _serial);
        //     if (_serial)
        //         Serial.begin(115200);
        //     else
        //         Serial.end();
        // }
        // if (pyldic.TryParseValue_i("G", &_G_seconds))
        // {
        //     logdebug("G:%is\n", _G_seconds);
        // }
        // if (pyldic.TryParseValue_i("P", &_Pfactor))
        // {
        //     logdebug("P:%i x G(=%is)\n", _Pfactor, _G_seconds);
        // }

        // pyldic.TryParseValue_b("details", &_verboseMode);
        // pyldic.TryParseValue_i("wifiattemps", &_EXPmqttattemps);
        // logflush();

        // if (_inputmessageCallback)
        //     _inputmessageCallback(pyldic);
        // else
        // {
        //     logwarning("no _inputmessageCallback defined\n");
        //     logflush();
        // }
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
