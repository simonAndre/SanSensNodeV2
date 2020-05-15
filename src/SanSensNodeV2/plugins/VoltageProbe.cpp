#include "VoltageProbe.h"
namespace SANSENSNODE_NAMESPACE
{
	VoltageProbe::VoltageProbe() : SensorPlugin("VoltageProbe") {}
	VoltageProbe::~VoltageProbe() {}

	double VoltageProbe::ReadVoltage()
	{
		//on fait une moyenne des lectures
		double reading = 0;
		for (size_t i = 0; i < nb_readings; i++)
		{
			reading += analogRead(VOLTAGEPIN);
			delay(10);
		}
		reading = reading / nb_readings;
		// Reference voltage is 3v3 so maximum reading is 3v3 = 4095 in range 0 to 4095
		if (reading < 1 || reading > 4095)
			return 0;
		return ADC_a * reading + ADC_b;
	}

	double VoltageProbe::ReadVoltageOn3_3()
	{
		double reading = analogRead(VOLTAGEPIN); // Reference voltage is 3v3 so maximum reading is 3v3 = 4095 in range 0 to 4095
		if (reading < 1 || reading > 4095)
			return 0;
		return -0.000000000000016 * pow(reading, 4) + 0.000000000118171 * pow(reading, 3) - 0.000000301211691 * pow(reading, 2) + 0.001109019271794 * reading + 0.034143524634089;
	}

	void VoltageProbe::firstSetup()
	{
		logdebug("firstSetup VoltageProbe\n");
	}

	void VoltageProbe::setupdevice(SubMenu &device_menu)
	{
		logdebug("setupdevice VoltageProbe done\n");
		}

	bool VoltageProbe::collectdata(JsonColl &collector)
	{
		double voltage = ReadVoltage();
		loginfo("Vpower=%f\n", voltage);
		collector.add("V", voltage);
		return true;
	}
	void VoltageProbe::onInputMessage(flyingCollection::SanCodedStr &data)
	{
	}
} // namespace SANSENSNODE_NAMESPACE