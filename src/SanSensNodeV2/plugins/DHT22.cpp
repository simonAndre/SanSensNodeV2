#include "DHT22.h"
#include <Arduino.h>
namespace SANSENSNODE_NAMESPACE
{
	DHT22::DHT22(uint8_t dhtpin) : SensorPlugin("DHT22")
	{
		this->_dhtpin = dhtpin;
	}

	DHT22::~DHT22()
	{
	}

	void DHT22::firstSetup()
	{
		_dhtWarmupTime = DHT_WAITTIMEMS;
	}
	void DHT22::setMenu(SubMenu &sensor_menu)
	{
		SensorPlugin::setMenu(sensor_menu);
		//hook up additional menu entries
		sensor_menu.addMenuitemUpdater("DHT warmup time (ms)", &_dhtWarmupTime);
		sensor_menu.addMenuitem()->SetLabel("DHT reset")->addLambda([&, this]() { this->dht_setup(); });
	}

	void DHT22::setupsensor()
	{
		logdebug("enter setupdevice DHT22\n");
		dht_setup();
		logdebug("setupdevice DHT22 done\n");
	}
	bool DHT22::collectdata(JsonColl &collector)
	{
		// _sensorNode->waitListeningIOevents(_dhtWarmupTime);
		delay(_dhtWarmupTime);

		TempAndHumidity th = dht.getTempAndHumidity();

		// Check if any reads failed and exit early (to try again).
		if (isnan(th.temperature) || isnan(th.humidity))
		{
			logerror("Failed to read from DHT sensor!\n");
			return false;
		}
		loginfo("DHT data t=%f°c ,H=%f\n", th.temperature, th.humidity);

		collector.add("temp", th.temperature);
		collector.add("humi", th.humidity);
		return true;
	}
	void DHT22::onInputMessage(flyingCollection::SanCodedStr &data)
	{
	}
	void DHT22::dht_setup()
	{
		dht.setup(_dhtpin, DHTesp::DHT_MODEL_t::DHT22);
	}
} // namespace SANSENSNODE_NAMESPACE