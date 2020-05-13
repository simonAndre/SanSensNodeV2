#include "sansensnode/SanSensNodeV2.h"

namespace SANSENSNODE_NAMESPACE
{

	static char _menuswitch_buff[SANSENSNODE_STRING_BUFFER_SIZE];
	static char _sansensnodeversion[10];
	static char _mqttpayload[SANSENSNODE_MQTTRECEIVEMESSAGE_SIZE];
	static uint16_t _mqttpayloadLength = 0;
	static bool _breakCurrentLoop = false;

	static WiFiClient __espClient;
	static RTC_DATA_ATTR int _bootCount = 0;												// store "boot" (restart after sleep) counting (in the RTC resilient RAM memory)
	static RTC_DATA_ATTR bool _firstinit = true;											//
	static RTC_DATA_ATTR bool _awakemode;													// while in awake mode => don't go to sleep on each G cycle until a sleep order has been received
	static RTC_DATA_ATTR bool _mqttsubscribe;												// if not subscribe : doesn't respond to mqtt event (only send to server)s
	static RTC_DATA_ATTR uint16_t _G_seconds;												// measurement cycle time ie: sleep time (in seconds) (in awake mode and sleep mode)
	static RTC_DATA_ATTR uint16_t _Pfactor;													// publication and mqtt connection frequency (in G multiples)
	static RTC_DATA_ATTR bool _serial;														// false to disable output serial log
	static RTC_DATA_ATTR uint8_t _cpuFreq = SANSENSNODE_STARTINGCPUFREQ;					// CPU operating frequency
	static RTC_DATA_ATTR uint8_t _wifitrialsmax;											// nb max of attemps to check the wifi network for a wakeup cycle
	static RTC_DATA_ATTR uint32_t _Gi;														// store the current cycle index (G or measurement cycle)
	static RTC_DATA_ATTR uint32_t _Pi;														// store the current publication index (P)
	static RTC_DATA_ATTR uint16_t _EXPwifiwait;												// EXPERIMENTAL : temps d'attente suite allmuage wifi pour que le MQTT puisse répondre
	static RTC_DATA_ATTR uint16_t _waitforMqttSend = SANSENSNODE_WAITFORMQTTSENDLOOP;		// nb of loop to wait for mqtt send to server
	static RTC_DATA_ATTR uint16_t _waitforMqttReceive = SANSENSNODE_WAITFORMQTTRECEIVELOOP; // nb of loop to wait for mqtt receive from server
	static RTC_DATA_ATTR uint8_t _EXPmqttattemps;											// EXPERIMENTAL : nb de tentatives pour connexion au server mqtt
	static RTC_DATA_ATTR uint8_t _wifiMode = 4;												// WIFI MODE :    0=WIFI_MODE_NULL (no WIFI),1=WIFI_MODE_STA (WiFi station mode),2=WIFI_MODE_AP (WiFi soft-AP mode)
	static RTC_DATA_ATTR uint8_t _loglevel = LOG_LEVEL;										// log level : 0=Off, 1=Critical, 2=Error, 3=Warning, 4=Info, 5=Debug
																							// 3=WIFI_MODE_APSTA (WiFi station + soft-AP mode) 4=WIFI_MODE_MAX
	static RTC_DATA_ATTR const char *_nodename{nullptr};
	static RTC_DATA_ATTR const char *_ssid{nullptr};
	static RTC_DATA_ATTR const char *_password{nullptr};
	static RTC_DATA_ATTR const char *_mqtt_server{nullptr};

	static RTC_DATA_ATTR bool _verboseMode;
	static RTC_DATA_ATTR uint16_t _EXPmodetestmqttmessagestart = 100;

	static RTC_DATA_ATTR uint8_t _maxMeasurementAttenmpts;

	static std::function<void(flyingCollection::SanCodedStr const &)> _inputmessageCallback;
	static std::function<bool(JsonColl &)> _collectdataCallback;
	static std::function<void(SubMenu &)> _setupdevicesCallback;
	
	
	const uint16_t _jsonoutbuffersize = 150;

    SanSensNodeV2::SanSensNodeV2(const char *nodename, const char *ssid, const char *wifipasswd, const char *mqttserver, int G, int Pfactor)
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
}