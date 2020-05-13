#include "DHT22.h"
#include <Arduino.h>
namespace SANSENSNODE_NAMESPACE
{
	DHT22::DHT22(uint8_t dhtpin) : DevicePlugin()
	{
		this->_dhtpin = dhtpin;
	}

	DHT22::~DHT22()
	{
	}

	void DHT22::setupdevice(SubMenu &device_menu)
	{
		//hook up additional menu entries
		// SubMenu *device_menu = _sensorNode->getDeviceMenu();
		device_menu.addMenuitemUpdater("DHT warmup time (ms)", &_dhtWarmupTime);
		device_menu.SetLabel("DHT reset")->addLambda([&, this]() { this->dht_setup(); });
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