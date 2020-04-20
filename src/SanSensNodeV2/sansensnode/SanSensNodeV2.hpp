#pragma once

namespace SANSENSNODE_NAMESPACE
{

typedef bool (*pf_callback_collectdata)(SanDataCollector);
typedef void (*pf_callback_setupdevices)(void);
typedef bool (*pf_callback_inputmessage)(SanCodedStrings);

static char _menuswitch_buff[SANSENSNODE_STRING_BUFFER_SIZE];
static char _sansensnodeversion[10];
static size_t _lastbuffercapacity;

static WiFiClient __espClient;
static RTC_DATA_ATTR int _bootCount = 0;      // store "boot" (restart after sleep) counting (in the RTC resilient RAM memory)
static RTC_DATA_ATTR bool _firstinit = false; //
static RTC_DATA_ATTR bool _awakemode;         // while in awake mode => don't go to sleep on each G cycle until a sleep order has been received
static RTC_DATA_ATTR int _G_seconds;          // measurement cycle time ie: sleep time (in seconds) (in awake mode and sleep mode)
static RTC_DATA_ATTR int _Pfactor;            // publication and mqtt connection frequency (in G multiples)
static RTC_DATA_ATTR bool _serial;            // false to disable output serial log
static RTC_DATA_ATTR bool _lowEnergyMode;     // false to disable output serial log
static RTC_DATA_ATTR uint8_t _wifitrialsmax;  // nb max of attemps to check the wifi network for a wakeup cycle
static RTC_DATA_ATTR uint8_t _waitforMqtt;    // nb of loop to wait for mqtt server answer
static RTC_DATA_ATTR int _GcycleIx;           // store the current cycle index (G or measurement cycle)
static RTC_DATA_ATTR int _EXPwifiwait;        // EXPERIMENTAL : temps d'attente suite allmuage wifi pour que le MQTT puisse répondre
static RTC_DATA_ATTR int _EXPmqttattemps;     // EXPERIMENTAL : nb de tentatives pour connexion au server mqtt
static RTC_DATA_ATTR uint8_t _wifiMode=4;       // WIFI MODE :    0=WIFI_MODE_NULL (no WIFI),1=WIFI_MODE_STA (WiFi station mode),2=WIFI_MODE_AP (WiFi soft-AP mode)
                                            // 3=WIFI_MODE_APSTA (WiFi station + soft-AP mode) 4=WIFI_MODE_MAX
static RTC_DATA_ATTR bool _verboseMode;
static pf_callback_inputmessage _inputmessageCallback;

enum MenusKeys
{
    device,
    measure,
    publi,
    infos
};

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
            _lowEnergyMode = lowEnergyMode;
            _wifitrialsmax = SANSENSNODE_WIFITRIALSINIT;
            if (lowEnergyMode)
                _wifitrialsmax = _wifitrialsmax / SANSENSNODE_LOWENERGYFACTOR;
            this->client.setClient(__espClient);
            _firstinit = true;
            _bootCount += 13;
        }
        setupSerialMenu();
        ++_bootCount;
    }

    void Setup()
    {
        if (_lowEnergyMode)
            setCpuFrequencyMhz(80); // set the lowest frequency possible to save energy (we)
        if (_serial)
        {
            delay(500); // pour attendre port serie
            Serial.begin(115200);
            IoHelpers::IOdisplay("#boot=");
            IoHelpers::IOdisplayLn(_bootCount);
            IoHelpers::IOdisplay("awakemode:");
            IoHelpers::IOdisplayLn(_awakemode);
            IoHelpers::IOdisplay("G duration is ");
            IoHelpers::IOdisplayLn(_G_seconds);
            IoHelpers::IOdisplay("P factor is ");
            IoHelpers::IOdisplayLn(_Pfactor);
        }
        // esp_sleep_enable_timer_wakeup(_delayloop * uS_TO_S_FACTOR);

        if (_setupdevicesCallback)
            _setupdevicesCallback();
        else
            IoHelpers::IOdisplayLn("no _setupdevicesCallback defined");
    }

    void Loop()
    {
        _GcycleIx++;

        IoHelpers::IOdisplay("G index:");
        IoHelpers::IOdisplayLn(_GcycleIx);

        _measurementAttenmpts = 1;

        SanDataCollector datacollector;
        _lastbuffercapacity = datacollector.getbufferSize();

        IoHelpers::IOdisplayLn("measure");
        while (!SanSensNodeV2::collectMeasurement(datacollector) && _measurementAttenmpts <= _maxMeasurementAttenmpts)
        {
            _measurementAttenmpts++;
        }
        IoHelpers::IOdisplayLn("measure end");

        if (_GcycleIx % _Pfactor == 0)
            SanSensNodeV2::mqttpubsub(datacollector);

        waitNextG();
    }

    /**
 * @brief wait a given time still listening for serial event to possibly stop the execution flow to display the interractive menu
 * the atomic wait time between serial checks will be defined by SANSENSNODE_WAITLOOPDELAYMS
 * @param waittimems 
 */
    void waitListeningIOevents(unsigned int waittimems)
    {
        if (waittimems > 500)
            IoHelpers::IOdisplay("wait");
        for (size_t i = 0; i < waittimems / SANSENSNODE_WAITLOOPDELAYMS; i++)
        {
            if (i % (1000 / SANSENSNODE_WAITLOOPDELAYMS) == 0)
                IoHelpers::IOdisplay("."); //display 1 . every 500ms
            _consolemenu->LoopCheckSerial();
            delay(SANSENSNODE_WAITLOOPDELAYMS);
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
            IoHelpers::IOdisplay("Wait for s");
            IoHelpers::IOdisplayLn(_G_seconds);
            waitListeningIOevents(1000 * _G_seconds);
        }
        else
        {
            IoHelpers::IOdisplay("Sleep for s");
            IoHelpers::IOdisplayLn(_G_seconds);
            Serial.flush();
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
        IoHelpers::IOdisplayLn("Going to sleep,bye.");
        // WiFi.disconnect(true);
        // WiFi.mode(WIFI_OFF);

        // btStop();
        // adc_power_off();
        // esp_wifi_stop();
        // esp_bt_controller_disable();

        // Configure the timer to wake up
        esp_sleep_enable_timer_wakeup(_G_seconds * 1000000L);

        // Go to sleep
        esp_deep_sleep_start();
    }

    static char *getVersion()
    {
        sprintf(_sansensnodeversion, "%i.%i.%i", CONSOLEMENU_VERSION_MAJOR, CONSOLEMENU_VERSION_MINOR, CONSOLEMENU_VERSION_REVISION);
        return _sansensnodeversion;
    }

    MenuitemHierarchy *getDeviceMenu()
    {
        return this->_device_menu;
    }

    // Menubase *getDeviceSubMenu()
    // {
    //     return this->consolemenu;
    // }

private:
    PubSubClient client;

    bool initStringValue(const char *menuname);
    bool DisplayStringValue(const char *menuname);
    Menubase *_consolemenu;
    MenuitemHierarchy *_device_menu;
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
            IoHelpers::IOdisplay("MQTT connection: ");
            // Attempt to connect
            if (!client.connect(_nodename))
            {
                IoHelpers::IOdisplay("failed, rc=");
                IoHelpers::IOdisplayLn(client.state());
                if (++i >= _EXPmqttattemps)
                    return false;
            }
            IoHelpers::IOdisplayLn("OK");
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
            waitListeningIOevents(20);
        }
    }

    bool mqttSubscribe()
    {
        if (!client.connected())
        {
            IoHelpers::IOdisplayLn("can't subscribe: no mqtt connection");
            return false;
        }
        std::string intopic(SanSensNodeV2::_mqttTopicBaseName);
        intopic.append(_nodename);
        intopic.append("/in");
        if (!client.subscribe(intopic.c_str(), 1))
        {
            IoHelpers::IOdisplay("subscription to topic ");
            IoHelpers::IOdisplay(intopic.c_str());
            IoHelpers::IOdisplayLn(" failed!");
            return false;
        }
        IoHelpers::IOdisplayLn("mqtt subscription OK");
        return true;
    }

    bool collectMeasurement(SanDataCollector dc)
    {
        if (!collectMeasurement_internal(dc))
            return false;
        if (_collectdataCallback)
        {
            if (!_collectdataCallback(dc))
                return false;
        }
        else
            IoHelpers::IOdisplayLn("no collectdata Callback defined");

        return true;
    }

    // publish and subscribe to the respective MQTT chanels (open the connection and close after)
    bool mqttpubsub(SanDataCollector dc)
    {
        uint32_t m0 = millis();
        Setup_wifi();
        if (!client.connected())
            if (!mqttConnect())
                return false;

        mqttSubscribe(); // todo : action en cas de défaut de souscription??

        // WaitforMqtt(15); // to wait for incoming messages

        if (_measurementAttenmpts >= _maxMeasurementAttenmpts)
            dc.Add_b("SensorIssue", true);

        char buffer[dc.getbufferSize()];
        size_t size = dc.Serialize(buffer);

        std::string outopic(SanSensNodeV2::_mqttTopicBaseName);
        outopic.append(_nodename);
        outopic.append("/out");
        IoHelpers::IOdisplay("publish on ");
        IoHelpers::IOdisplay(outopic.c_str());
        IoHelpers::IOdisplay(", taille message: ");
        IoHelpers::IOdisplayLn((int)size);
        client.publish(outopic.c_str(), buffer, true);
        IoHelpers::IOdisplay("message: ");
        IoHelpers::IOdisplayLn((char *)buffer);
        IoHelpers::IOdisplayLn("publication mqtt OK");

        bool r = WaitforMqtt(_waitforMqtt);
        wifiOff();

        IoHelpers::IOdisplay("wifi was on for ");
        IoHelpers::IOdisplay(millis() - m0);
        IoHelpers::IOdisplay("ms, WaitforMqtt = ");
        IoHelpers::IOdisplayLn(r);
        return r;
    }

    void Setup_wifi()
    {

        delay(10);
        // We start by connecting to a WiFi network
        IoHelpers::IOdisplayLn("");
        IoHelpers::IOdisplay("Connecting to ");
        IoHelpers::IOdisplay(SanSensNodeV2::_ssid);

        WiFi.begin(SanSensNodeV2::_ssid, SanSensNodeV2::_password);
        WiFi.mode((wifi_mode_t)_wifiMode);
        int nb = 0;
        while (WiFi.status() != WL_CONNECTED && nb++ < _wifitrialsmax)
        {
            waitListeningIOevents(_EXPwifiwait);
        }
        if (WiFi.status() != WL_CONNECTED)
        {
            IoHelpers::IOdisplayLn("Wifi not connected, expired wifitrialsmax");
            waitNextG();
        }
        // randomSeed(micros());

        IoHelpers::IOdisplayLn("");
        IoHelpers::IOdisplay("WiFi connected, IP address:");
        Serial.print(WiFi.localIP());
        IoHelpers::IOdisplayLn(", ");

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
        _consolemenu = new Menu<20>();
        //define options
        MenuOptions menuoptions;
        menuoptions.addBack = true;
        menuoptions.addExitForEachLevel = true;
        _consolemenu->setOptions(menuoptions);
        // menus & submenus definition
        // root menus

        MenuitemHierarchy *root = _consolemenu->getRootMenu();
        _device_menu = root->addMenuitemHierarchy("device");
        MenuitemHierarchy *measure_menu = root->addMenuitemHierarchy("measure");
        MenuitemHierarchy *publication_menu = root->addMenuitemHierarchy("publication");
        MenuitemHierarchy *infos_menu = root->addMenuitemHierarchy("infos");

        publication_menu->addMenuitemUpdater("awake mode", &_awakemode);
        publication_menu->addMenuitemUpdater("verbose mode", &_verboseMode);

        measure_menu->addMenuitemUpdater("set G", &_G_seconds);
        publication_menu->addMenuitemUpdater("set P", &_Pfactor);
        publication_menu->addMenuitemUpdater("wait for mqtt loops", &_waitforMqtt);
        publication_menu->addMenuitemUpdater("wifiwait", &_EXPwifiwait);
        publication_menu->addMenuitemUpdater("wifiattemps", &_EXPmqttattemps);
        publication_menu->addMenuitemUpdater("wifi mode (1 to 4)", &_wifiMode);

        infos_menu->addMenuitemCallback("build infos", buildInfos);
        infos_menu->addMenuitemCallback("lib version", libVersion);
        infos_menu->addMenuitemCallback("RT infos", rtInfos);
    }

    bool collectMeasurement_internal(SanDataCollector dc)
    {
        //collect data and fill _datacollector with it
        dc.Add_s("device", (const char *)_nodename);
        dc.Add_i("boot", _bootCount);
        double timeSinceStartup = esp_timer_get_time() / 1000000;
        dc.Add_d("uptime", timeSinceStartup);
        dc.Add_i("measurements", _measurementAttenmpts);
        if (_verboseMode)
        {
            dc.Add_i("G", _G_seconds);
            dc.Add_i("P", _Pfactor);
            dc.Add_i("freeram", (int)heap_caps_get_free_size(MALLOC_CAP_8BIT));
            dc.Add_i("Gcylcle", _GcycleIx);
            dc.Add_b("LowEnergyMode", _lowEnergyMode);
            dc.Add_b("Serial", _serial);
            dc.Add_i("wifitrialsmax", _wifitrialsmax);
            dc.Add_i("waitforMqtt", _waitforMqtt);
            dc.Add_i("mqttattemps", _EXPmqttattemps);
            dc.Add_i("wifiwait", _EXPwifiwait);
        }
        return true;
    }

    static bool mqttCallback(char *topic, uint8_t *payload, unsigned int length)
    {
        IoHelpers::IOdisplay("Message arrived [");
        IoHelpers::IOdisplay(topic);
        IoHelpers::IOdisplay("] ");
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
            IoHelpers::IOdisplay("sleep:");
            IoHelpers::IOdisplayLn(_awakemode);
        }
        if (pyldic.TryParseValue_b("serial", &_serial))
        {
            IoHelpers::IOdisplay("serial:");
            IoHelpers::IOdisplayLn(_serial);
            if (_serial)
                Serial.begin(115200);
            else
                Serial.end();
        }
        if (pyldic.TryParseValue_i("G", &_G_seconds))
        {
            IoHelpers::IOdisplay("G(seconds):");
            IoHelpers::IOdisplayLn(_G_seconds);
        }
        if (pyldic.TryParseValue_i("P", &_Pfactor))
        {
            IoHelpers::IOdisplay("P(G multiple):");
            IoHelpers::IOdisplayLn(_Pfactor);
        }
        if (pyldic.TryParseValue_i("wfmqtt", &_waitforMqtt))
        {
            IoHelpers::IOdisplay("WaitforMqtt:");
            IoHelpers::IOdisplayLn(_waitforMqtt);
        }
        pyldic.TryParseValue_b("details", &_verboseMode);
        pyldic.TryParseValue_i("wifiwait", &_EXPwifiwait);
        pyldic.TryParseValue_i("wifiattemps", &_EXPmqttattemps);
        if (_inputmessageCallback)
            return _inputmessageCallback(pyldic);
        else
            IoHelpers::IOdisplayLn("no _inputmessageCallback defined");

        return true;
    }

    static bool buildInfos()
    {
        IoHelpers::IOdisplay("__GNUG__:");
        IoHelpers::IOdisplayLn(__GNUG__);
        IoHelpers::IOdisplay("__cplusplus:");
        IoHelpers::IOdisplayLn(__cplusplus);
        IoHelpers::IOdisplay("build time :");
        IoHelpers::IOdisplayLn(__TIMESTAMP__);
        IoHelpers::IOdisplay("SanSensNodeV2 V:");
        IoHelpers::IOdisplayLn(getVersion());
        IoHelpers::IOdisplay("consoleMenu V:");
        IoHelpers::IOdisplayLn(Menubase::getVersion());
        return false;
    }

    static bool rtInfos()
    {
        IoHelpers::IOdisplay("cpu freq:");
        IoHelpers::IOdisplayLn(getCpuFrequencyMhz());
        IoHelpers::IOdisplay("buffer capacity:");
        IoHelpers::IOdisplayLn(_lastbuffercapacity);
};
} // namespace SANSENSNODE_NAMESPACE
