#pragma once
#include "DeepSleep.hpp"

namespace SANSENSNODE_NAMESPACE
{

typedef bool (*pf_callback_collectdata)(SanDataCollector *);
typedef void (*pf_callback_setupdevices)(void);
typedef bool (*pf_callback_inputmessage)(SanCodedStrings);

static char _menuswitch_buff[SANSENSNODE_STRING_BUFFER_SIZE];
static char _sansensnodeversion[10];
static size_t _lastbuffercapacity;
static bool _breakCurrentLoop = false;

static WiFiClient __espClient;
static RTC_DATA_ATTR int _bootCount = 0;                             // store "boot" (restart after sleep) counting (in the RTC resilient RAM memory)
static RTC_DATA_ATTR bool _firstinit = false;                        //
static RTC_DATA_ATTR bool _awakemode;                                // while in awake mode => don't go to sleep on each G cycle until a sleep order has been received
static RTC_DATA_ATTR int _G_seconds;                                 // measurement cycle time ie: sleep time (in seconds) (in awake mode and sleep mode)
static RTC_DATA_ATTR int _Pfactor;                                   // publication and mqtt connection frequency (in G multiples)
static RTC_DATA_ATTR bool _serial;                                   // false to disable output serial log
static RTC_DATA_ATTR uint8_t _cpuFreq = SANSENSNODE_STARTINGCPUFREQ; // CPU operating frequency
static RTC_DATA_ATTR uint8_t _wifitrialsmax;                         // nb max of attemps to check the wifi network for a wakeup cycle
static RTC_DATA_ATTR uint8_t _waitforMqtt;                           // nb of loop to wait for mqtt server answer
static RTC_DATA_ATTR int _GcycleIx;                                  // store the current cycle index (G or measurement cycle)
static RTC_DATA_ATTR int _EXPwifiwait;                               // EXPERIMENTAL : temps d'attente suite allmuage wifi pour que le MQTT puisse répondre
static RTC_DATA_ATTR int _EXPmqttattemps;                            // EXPERIMENTAL : nb de tentatives pour connexion au server mqtt
static RTC_DATA_ATTR uint8_t _wifiMode = 4;                          // WIFI MODE :    0=WIFI_MODE_NULL (no WIFI),1=WIFI_MODE_STA (WiFi station mode),2=WIFI_MODE_AP (WiFi soft-AP mode)
static RTC_DATA_ATTR uint8_t _loglevel = 4;                          // log level : 0=Off, 1=Critical, 2=Error, 3=Warning, 4=Info, 5=Debug
                                                                     // 3=WIFI_MODE_APSTA (WiFi station + soft-AP mode) 4=WIFI_MODE_MAX
static RTC_DATA_ATTR bool _EXPusesandatacollector = true;
static RTC_DATA_ATTR bool _verboseMode;
static pf_callback_inputmessage _inputmessageCallback;

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
    SanSensNodeV2(const char *nodename, const char *ssid, const char *wifipasswd, const char *mqttserver,
                  bool lowEnergyMode, int G, int Pfactor)
    {

        if (!_firstinit) // première initialisation, ensuite on rentre dans ce constructeur à chaque reveil donc on ne doit pas réinitialiser les variables stockées dnas la RAM de la RTC
        {
            logdebugLn("enter SanSensNodeV2 ctor first init");
            if (SANSENSNODE_STARTSAWAKEN == 1)
                _awakemode = true;
            else
                _awakemode = false;

            _serial = true;
            _waitforMqtt = SANSENSNODE_WAITFORMQTTLOOP;
            _GcycleIx = 0;
            _EXPwifiwait = SANSENSNODE_WIFIWAITTIMEMS;
            _EXPmqttattemps = SANSENSNODE_MQTTATTEMPTSNB;
            _verboseMode = false;
            SanSensNodeV2::_nodename = (char *)nodename;
            SanSensNodeV2::_ssid = (char *)ssid;
            SanSensNodeV2::_password = (char *)wifipasswd;
            SanSensNodeV2::_mqtt_server = (char *)mqttserver;
            SanSensNodeV2::_maxMeasurementAttenmpts = SANSENSNODE_MAX_MEASURES_ATTEMPTS;
            SanSensNodeV2::_measurementAttenmpts = 0;
            _G_seconds = G;
            _Pfactor = Pfactor;
            if (lowEnergyMode)
                _cpuFreq = 80;
            _wifitrialsmax = SANSENSNODE_WIFITRIALSINIT;
            if (lowEnergyMode)
                _wifitrialsmax = _wifitrialsmax / SANSENSNODE_LOWENERGYFACTOR;
            _firstinit = true;
            _bootCount += 13;
        }
        this->client.setClient(__espClient);
        setupSerialMenu();
        ++_bootCount;
        logdebugLn("end SanSensNodeV2 ctor");
    }

    /**
 * @brief called by main sketch
 * 
 */
    void Setup()
    {
        SetEnergyMode();
        _deepsleep = new SANSENSNODE_NAMESPACE::DeepSleep();
        _deepsleep->SetupTouchpadWakeup(40, 13);

        if (_serial)
        {
            delay(500); // pour attendre port serie
            Serial.begin(115200);
            loginfoLn("#boot=%i", _bootCount);
            loginfoLn("#awakemode:%i", _awakemode);
            loginfoLn("G duration = %i", _G_seconds);
            loginfoLn("P factor = %i", _Pfactor);
            loginfoLn("WIFI mode = %i", _wifiMode);
            logflush();
        }
        // esp_sleep_enable_timer_wakeup(_delayloop * uS_TO_S_FACTOR);

        if (_deepsleep)
            _deepsleep->print_wakeup_reason();

        if (_setupdevicesCallback)
            _setupdevicesCallback();
        else
            logdebugLn("no _setupdevicesCallback defined");
        logflush();
    }

    /**
 * @brief called by main sketch
 * 
 */
    void Loop()
    {
        _GcycleIx++;
        _breakCurrentLoop = false;

        loginfoLn("G index:%i", _GcycleIx);

        _measurementAttenmpts = 1;

        SanDataCollector *datacollector{nullptr};
        if (_EXPusesandatacollector)
            datacollector = new SanDataCollector();

        _lastbuffercapacity = datacollector->getbufferSize();

        logdebugLn("measure start");
        while (!SanSensNodeV2::collectMeasurement(datacollector) && _measurementAttenmpts <= _maxMeasurementAttenmpts)
        {
            _measurementAttenmpts++;
        }
        logdebugLn("measure end");
        logflush();

        if (_wifiMode > 0 && datacollector && _GcycleIx % _Pfactor == 0)
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
 * @param waittimems 
 */
    void waitListeningIOevents(unsigned int waittimems)
    {
        if (waittimems > 500)
            loginfo("wait");
        for (size_t i = 0; i < waittimems / SANSENSNODE_WAITLOOPDELAYMS; i++)
        {
            if (i % (1000 / SANSENSNODE_WAITLOOPDELAYMS) == 0)
            {
                printf("."); //display 1 . every 500ms
            }
            _consolemenu->LoopCheckSerial();
            delay(SANSENSNODE_WAITLOOPDELAYMS);
            if (_breakCurrentLoop)
                break;
        }
    }

    /**
 * @brief wait for the next G time. If awakemode is false, go to deep sleep.
 * 
 */
    void waitNextG()
    {

        if (_awakemode)
        {
            loginfo("Wait for %i s", _G_seconds);
            logflush();
            waitListeningIOevents(1000 * _G_seconds);
        }
        else
        {
            loginfoLn("Sleep for %i s", _G_seconds);
            logflush();
            SanSensNodeV2::DeepSleep();
        }
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
        loginfoLn("Going to sleep,bye.");
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
    char *_nodename{nullptr}, *_ssid{nullptr}, *_password{nullptr}, *_mqtt_server{nullptr};
    uint8_t _maxMeasurementAttenmpts, _measurementAttenmpts;
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
            loginfoLn("OK");
            logflush();
        }
        return true;
    }

    //allow the client to process incoming messages and maintain its connection to the server.
    bool WaitforMqtt(int nb)
    {
        int l;
        // if (_awakemode)
        //   nb = 1;
        for (int l = 0; l <= nb; l++)
        {
            if (!client.loop()) //This should be called regularly to allow the client to process incoming messages and maintain its connection to the server.
                return false;   // the client is no longer connected
            waitListeningIOevents(200);
        }
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
        loginfoLn("mqtt subscription OK");
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
            logdebugLn("no collectdata Callback defined");

        return true;
    }

    // publish and subscribe to the respective MQTT chanels (open the connection and close after)
    bool mqttpubsub(SanDataCollector *dc)
    {
        logdebugLn("enter mqttpubsub");
        if (bootAfterSleep())
        {
            logflush();
            waitListeningIOevents(250 * _waitforMqtt);
        }

        uint32_t m0 = millis();
        Setup_wifi();
        if (!client.connected())
            if (!mqttConnect())
                return false;

        mqttSubscribe(); // todo : action en cas de défaut de souscription??

        // WaitforMqtt(15); // to wait for incoming messages
        if (dc)
        {
            if (_measurementAttenmpts >= _maxMeasurementAttenmpts)
                dc->Add_b("SensorIssue", true);

            char buffer[dc->getbufferSize()];
            size_t size = dc->Serialize(buffer);

            std::string outopic(SanSensNodeV2::_mqttTopicBaseName);
            outopic.append(_nodename);
            outopic.append("/out");
            loginfoLn("publish on %s, taille message: %i ", outopic.c_str(), (int)size);
            client.publish(outopic.c_str(), buffer, true);
            loginfoLn("message: %s, publication mqtt OK", (char *)buffer);
            logflush();
        }
        bool r = WaitforMqtt(_waitforMqtt);
        wifiOff();

        loginfoLn("wifi was on for %ims,  WaitforMqtt=%i", millis() - m0, r);
        logdebugLn("exit mqttpubsub");
        logflush();
        return r;
    }

    void Setup_wifi()
    {
        delay(10);

        logdebugLn("Connecting to %s, wifi mode=%i", SanSensNodeV2::_ssid, _wifiMode);
        logflush();

        WiFi.begin(SanSensNodeV2::_ssid, SanSensNodeV2::_password);
        WiFi.mode((wifi_mode_t)_wifiMode);
        int nb = 0;
        while (WiFi.status() != WL_CONNECTED && nb++ < _wifitrialsmax)
        {
            waitListeningIOevents(_EXPwifiwait);
        }
        if (WiFi.status() != WL_CONNECTED)
        {
            logwarning("Wifi not connected, expired wifitrialsmax\n");
            logflush();
            waitNextG();
        }

        loginfo("WiFi connected, IP address: ");
        Serial.print(WiFi.localIP());
        Serial.print(",\n");
        logflush();

        client.setServer(SanSensNodeV2::_mqtt_server, SANSENSNODE_MQTTPORT);
        client.setCallback(mqttCallback);
        waitListeningIOevents(_EXPwifiwait);
    }

    void wifiOff()
    {
        WiFi.disconnect(true);
        WiFi.mode(WIFI_OFF);
    }

    void setupSerialMenu()
    {
        _consolemenu = new Menu<26>(); //21 menu-entries in this class and 4 more in the sketch (todo : to template)
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
        SubMenu *measure_menu = root->addSubMenu("measure");
        SubMenu *publication_menu = root->addSubMenu("publication");
        SubMenu *infos_menu = root->addSubMenu("infos");

        publication_menu->addMenuitemUpdater("awake mode", &_awakemode)->addCallback(breakLoop);
        publication_menu->addMenuitemUpdater("verbose mode", &_verboseMode);

        measure_menu->addMenuitemUpdater("set G", &_G_seconds)->addCallback(breakLoop);
        publication_menu->addMenuitemUpdater("set P", &_Pfactor);
        publication_menu->addMenuitemUpdater("wait for mqtt loops", &_waitforMqtt);
        publication_menu->addMenuitemUpdater("wifiwait", &_EXPwifiwait);
        publication_menu->addMenuitemUpdater("wifiattemps", &_EXPmqttattemps);
        publication_menu->addMenuitemUpdater("wifi mode (1 to 4)", &_wifiMode);
        publication_menu->addMenuitemUpdater("use sandatacollector", &_EXPusesandatacollector);

        SubMenu *smfreq = _device_menu->addSubMenu("set CPU freq")->addCallbackToChilds(SetEnergyMode)->addCallbackToChilds(breakLoop);
        smfreq->addMenuitem()->SetLabel("80 Mhz")->addLambda([]() { _cpuFreq = 80; });
        smfreq->addMenuitem()->SetLabel("160 Mhz")->addLambda([]() { _cpuFreq = 160; });
        smfreq->addMenuitem()->SetLabel("240 Mhz")->addLambda([]() { _cpuFreq = 240; });

        _device_menu->addMenuitemCallback("break loop", breakLoop);

        infos_menu->addMenuitemCallback("build infos", buildInfos);
        infos_menu->addMenuitemCallback("RT infos", rtInfos);
        infos_menu->addMenuitemUpdater("Log level (0=off->5=debug)", &_loglevel)->addLambda([]() { loglevel((log_level_e)_loglevel); });
    }

    bool collectMeasurement_internal(SanDataCollector *dc)
    {
        if (!dc)
            return false;
        //collect data and fill _datacollector with it
        dc->Add_s("device", (const char *)_nodename);
        dc->Add_i("boot", _bootCount);
        double timeSinceStartup = esp_timer_get_time() / 1000000;
        dc->Add_d("uptime", timeSinceStartup);
        if (_verboseMode)
        {
            dc->Add_i("G", _G_seconds);
            dc->Add_i("P", _Pfactor);
            dc->Add_i("freeram", (int)heap_caps_get_free_size(MALLOC_CAP_8BIT));
            dc->Add_i("Gcylcle", _GcycleIx);
            // dc->Add_b("cpumhz", _cpuFreq);
            // dc->Add_b("Serial", _serial);
            // dc->Add_i("wifitrialsmax", _wifitrialsmax);
            // dc->Add_i("waitforMqtt", _waitforMqtt);
            // dc->Add_i("mqttattemps", _EXPmqttattemps);
            // dc->Add_i("wifiwait", _EXPwifiwait);
        }
        return true;
    }


    static bool mqttCallback(char *topic, uint8_t *payload, unsigned int length)
    {
        loginfoLn("Message arrived on [%s]:", topic);
        for (int i = 0; i < length; i++)
        {
            IoHelpers::IOdisplay((char)payload[i]);
        }
        IoHelpers::IOdisplayLn("");
        String a;
        char *myvalue;
        bool asleep;

        SanCodedStrings pyldic((char *)payload);

        if (pyldic.TryParseValue_b("sleep", &asleep))
        {
            _awakemode = !asleep;
            loginfoLn("sleep:%i", _awakemode);
        }
        if (pyldic.TryParseValue_b("serial", &_serial))
        {
            loginfoLn("serial:%i", _serial);
            if (_serial)
                Serial.begin(115200);
            else
                Serial.end();
        }
        if (pyldic.TryParseValue_i("G", &_G_seconds))
        {
            loginfoLn("G:%is", _G_seconds);
        }
        if (pyldic.TryParseValue_i("P", &_Pfactor))
        {
            loginfoLn("P:%i x G(=%is)", _Pfactor, _G_seconds);
        }
        if (pyldic.TryParseValue_i("wfmqtt", &_waitforMqtt))
        {
            loginfoLn("WaitforMqtt:%i", _waitforMqtt);
        }
        pyldic.TryParseValue_b("details", &_verboseMode);
        pyldic.TryParseValue_i("wifiwait", &_EXPwifiwait);
        pyldic.TryParseValue_i("wifiattemps", &_EXPmqttattemps);
        logflush();

        if (_inputmessageCallback)
            return _inputmessageCallback(pyldic);
        else
        {
            logwarning("no _inputmessageCallback defined\n");
            logflush();
        }

        return true;
    }

    static bool buildInfos()
    {
#ifdef SANSENSNODE_SKETCHVERSION
        loginfoLn("sketch V:%s", SANSENSNODE_SKETCHVERSION);
#endif
        printf("SanSensNodeV2 V:%s", getVersion());
        printf("consoleMenu V:%s", Menubase::getVersion());
        printf("__GNUG__:%i", __GNUG__);
        printf("__cplusplus:%i", __cplusplus);
        printf("__TIMESTAMP__:%i", __TIMESTAMP__);
        logflush();
        return false;
    }

    static bool rtInfos()
    {
        printf("cpu freq:%i\n", getCpuFrequencyMhz());
        printf("buffer capacity:%i\n", _lastbuffercapacity);
        logflush();
        return false;
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
