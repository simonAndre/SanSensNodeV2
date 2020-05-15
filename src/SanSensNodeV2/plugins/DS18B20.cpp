#include "DS18B20.h"
#include <Arduino.h>
namespace SANSENSNODE_NAMESPACE
{
	DS18B20::DS18B20(uint8_t oneWireBus) : SensorPlugin("DS18B20"), _oneWireBus(oneWireBus)
	{
	}

	DS18B20::~DS18B20()
	{
	}

	void DS18B20::firstSetup()
	{
	}
	void DS18B20::setMenu(SubMenu &sensor_menu)
	{
		SensorPlugin::setMenu(sensor_menu);
		//hook up additional menu entries
	}

	void DS18B20::setupsensor()
	{
		logdebug("enter setup DS18B20, bus on PIN%i\n", _oneWireBus);
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
				loginfo("Found device %i with address:\n",i);
				printAddress(tempDeviceAddress);
				printf("\n");
			}
			else
			{
				logdebug("Found ghost device at %i but could not detect address. Check power and cabling\n", i);
			}
		}
		logdebug("setupdevice DS18B20 done\n");
	}

	bool DS18B20::collectdata(JsonColl &collector)
	{
		if (numberOfDevices > 0)
		{
			sensors->requestTemperatures(); // Send the command to get temperatures

			float T2 = sensors->getTempCByIndex(0);
			float T3 = sensors->getTempCByIndex(1);
			loginfo("one wire temp1=%f°c, temp2=%f°c\n", T2, T3);

			// Loop through each device, print out temperature data
			for (int i = 0; i < numberOfDevices; i++)
			{
				// Search the wire for address
				if (sensors->getAddress(tempDeviceAddress, i))
				{
					float tempC = sensors->getTempC(tempDeviceAddress);
					loginfo("Temperature for device [%i] : %fc\n", i, tempC);
					char buf[3];
					snprintf(buf, 3, "T%i", i);
					collector.add(buf, tempC);
				}
			}
		}
	}

	void DS18B20::onInputMessage(flyingCollection::SanCodedStr &data)
	{
	}

	// function to print a device address
	void DS18B20::printAddress(DeviceAddress deviceAddress)
	{
		for (uint8_t i = 0; i < 8; i++)
		{
			if (deviceAddress[i] < 16)
				Serial.print("0");
			Serial.print(deviceAddress[i], HEX);
		}
	}
} // namespace SANSENSNODE_NAMESPACE