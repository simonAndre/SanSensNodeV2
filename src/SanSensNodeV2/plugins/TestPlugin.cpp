#include "TestPlugin.h"
#include <Arduino.h>

namespace SANSENSNODE_NAMESPACE
{

	TestPlugin::TestPlugin() : SensorPlugin("TestPlugin") {}
	TestPlugin::~TestPlugin() {}

	bool TestPlugin::led1On()
	{
		digitalWrite(LED1PIN, HIGH);
		// getSanSensInstance()->waitListeningIOevents(250);
		delay(250);
		digitalWrite(LED1PIN, LOW);
		return false;
	}
	bool TestPlugin::led2On()
	{
		digitalWrite(LED2PIN, HIGH);
		delay(250);
		// getSanSensInstance()->waitListeningIOevents(250);
		digitalWrite(LED2PIN, LOW);
		return false;
	}

	void TestPlugin::firstSetup()
	{
	}

	void TestPlugin::setMenu(SubMenu &sensor_menu)
	{
		SensorPlugin::setMenu(sensor_menu);
		//hook up additional menu entries
		sensor_menu.addMenuitem()->SetLabel("switch led 1")->addLambda([&]() { led1On(); });
		sensor_menu.addMenuitem()->SetLabel("switch led 2")->addLambda([&]() { led2On(); });
	}
	
	void TestPlugin::setupsensor()
	{
		logdebug("enter setupdevice TestPlugin\n");
		pinMode(LED1PIN, OUTPUT); // Initialize the LED2PIN pin as an output
		pinMode(LED2PIN, OUTPUT); // Initialize the LED2PIN pin as an output

		logdebug("setupdevice TestPlugin done\n");
	}

	bool TestPlugin::collectdata(JsonColl &collector)
	{
		return true;
	}
	void TestPlugin::onInputMessage(flyingCollection::SanCodedStr &data)
	{
		bool ledon;
		uint8_t ledpin{0};

		if (data.tryGetValue("led2", ledon))
		{
			ledpin = LED2PIN;
		}
		if (data.tryGetValue("led1", ledon))
		{
			ledpin = LED1PIN;
		}
		if (ledpin > 0)
		{
			logdebug("led-%i:%i\n", ledpin, ledon);
			if (ledon)
			{
				digitalWrite(ledpin, HIGH);
				// sansens_instance->waitListeningIOevents(500);
				// digitalWrite(ledpin, LOW);
			}
			else
				digitalWrite(ledpin, LOW);
		}
	}
}