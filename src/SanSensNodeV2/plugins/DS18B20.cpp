#include "DS18B20.h"
#include <Arduino.h>
namespace SANSENSNODE_NAMESPACE
{
	RTC_DATA_ATTR int _WarmupTime;
	char addstr[16]{'0'};
	const char *getAddress(DeviceAddress deviceAddress); //forward decl

	DS18B20::DS18B20(uint8_t oneWireBus) : SensorPlugin("DS18B20"), _oneWireBus(oneWireBus)
	{
	}

	DS18B20::~DS18B20()
	{
	}

	void DS18B20::firstSetup()
	{
		_WarmupTime = DS18B20_WAITTIMEMS;
	}
	void DS18B20::setMenu(SubMenu &sensor_menu)
	{
		SensorPlugin::setMenu(sensor_menu);
		//hook up additional menu entries
		sensor_menu.addMenuitemUpdater("warmup time (ms)", &_WarmupTime);
	}

	void DS18B20::setupsensor()
	{
		logdebug("enter setup DS18B20, bus on PIN%i\n", _oneWireBus);

		waitWarmup();

		// Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
		OneWire oneWire(_oneWireBus);
		// Pass our oneWire reference to Dallas Temperature.
		sensors = new DallasTemperature(&oneWire);

		sensors->begin();

		// Grab a count of devices on the wire
		numberOfDevices = sensors->getDeviceCount();

		// locate devices on the bus
		logdebug("Locating devices, found %i devices\n", numberOfDevices);

		// Loop through each device, print out address
		for (int i = 0; i < numberOfDevices; i++)
		{
			// Search the wire for address
			if (sensors->getAddress(tempDeviceAddress, i))
			{
				loginfo("Found device %i with address: 0x%s\n", i, getAddress(tempDeviceAddress));
			}
			else
			{
				loginfo("Found ghost device at %i but could not detect address. Check power and cabling\n", i);
			}
		}
		logdebug("setupdevice DS18B20 done\n");
	}

	bool DS18B20::collectdata(JsonColl &collector)
	{
		if (numberOfDevices > 0)
		{
			logdebug("collect DS18B20 data on bus-pin:%i\n", _oneWireBus);
			waitWarmup();

			sensors->requestTemperatures(); // Send the command to get temperatures

			// float T2 = sensors->getTempCByIndex(0);
			// float T3 = sensors->getTempCByIndex(1);
			// loginfo("one wire temp1=%f°c, temp2=%f°c\n", T2, T3);

			// Loop through each device, print out temperature data
			for (int i = 0; i < numberOfDevices; i++)
			{
				// Search the wire for address
				logdebug("requesting getAddress for %i\n", i);
				try
				{
					if (sensors->getAddress(tempDeviceAddress, i))
					{
						logdebug("requesting getTempC for %i\n", i);
						float tempC = sensors->getTempC(tempDeviceAddress);
						loginfo("Temperature for sub-sensor [%i] : %fc\n", i, tempC);
						char buf[4];
						snprintf(buf, 4, "T%i", i);
						collector.add(buf, tempC);
					}
				}
				catch (const std::exception &e)
				{
					logerror("error requesting data from DS18b20 : %s\n", e.what());
				}
				catch (...)
				{
					logerror("unknown error requesting data from DS18b20\n");
				}
			}
		}
	}

	void DS18B20::onInputMessage(flyingCollection::SanCodedStr &data)
	{
	}

	void DS18B20::waitWarmup()
	{
		if (_WarmupTime > 0)
		{
			logdebug("wait for DS18B20 warmup %ims\n", _WarmupTime);
			_sansens_instance->waitListeningIOevents(_WarmupTime);
		}
	}

	// function to convert a device address as string
	const char *getAddress(DeviceAddress deviceAddress)
	{
		uint8_t vi;
		for (size_t i = 0; i < 8; i++)
		{
			if (deviceAddress[i] < 16)
				vi = 0;
			else
				vi = deviceAddress[i];
			snprintf(&(addstr[2 * i]), 3, "%02X", vi);
		}
		return (const char *)addstr;
	}

} // namespace SANSENSNODE_NAMESPACE