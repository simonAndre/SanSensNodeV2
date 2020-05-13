#include <Arduino.h>
#include "../Configuration.h"
#include "../Namespace.h"
#include "DeepSleep.h"
#include "../platform_logger.h"

namespace SANSENSNODE_NAMESPACE
{
	void DeepSleep::SetupTouchpadWakeup(uint8_t touchepadThreshold , uint8_t touchPadGPIO )
	{
		_touchPadGPIO = touchPadGPIO;
		_touchepadThreshold = touchepadThreshold;
		internalsetup();
	}

	void DeepSleep::SetupExt1InterruptWakup(uint8_t gpioButton )
	{
		_extPinsBitmask = 2 ^ gpioButton;
		internalsetup();
	}

	void DeepSleep::SetupTimerWakeup(uint16_t sleeptime_sec)
	{
		esp_sleep_enable_timer_wakeup(sleeptime_sec * 1000000L);
	}

	void DeepSleep::HookWakupCallback(void (*wakupcallback)())
	{
		_wakupcallback = wakupcallback;
	}

	void DeepSleep::GotoSleep()
	{
		esp_deep_sleep_start();
	}

	uint8_t DeepSleep::getWakeup_reason()
	{
		esp_sleep_wakeup_cause_t wakeup_reason;

		wakeup_reason = esp_sleep_get_wakeup_cause();

		switch (wakeup_reason)
		{
		case ESP_SLEEP_WAKEUP_EXT0:
			logdebug("Wakeup caused by external signal using RTC_IO\n");
			break;
		case ESP_SLEEP_WAKEUP_EXT1:
			logdebug("Wakeup caused by external signal using RTC_CNTL\n");
			break;
		case ESP_SLEEP_WAKEUP_TIMER:
			logdebug("Wakeup caused by timer\n");
			break;
		case ESP_SLEEP_WAKEUP_TOUCHPAD:
			logdebug("Wakeup caused by touchpad\n");
			break;
		case ESP_SLEEP_WAKEUP_ULP:
			logdebug("Wakeup caused by ULP program\n");
			break;
		default:
			logdebug("Wakeup was not caused by deep sleep: %i\n", (uint8_t)wakeup_reason);
			break;
		}

		// if (_touchepadThreshold > 0)
		//     print_wakeup_touchpad();

		logflush();
		return (uint8_t)wakeup_reason;
	}

	void DeepSleep::internalsetup()
	{

		if (_touchepadThreshold > 0)
		{
			touchAttachInterrupt(_touchPadGPIO, _wakupcallback, _touchepadThreshold);
			//Configure Touchpad as wakeup source
			esp_sleep_enable_touchpad_wakeup();
			logdebug("wakeup on touchepad configured\n");
		}
		if (_extPinsBitmask > 0)
		{
			esp_sleep_enable_ext1_wakeup(_extPinsBitmask, ESP_EXT1_WAKEUP_ANY_HIGH);
			logdebug("wakeup on ext1 configured\n");
		}
	}

    void DeepSleep::wakeupCallback()
	{
		logdebug("no wakeupCallback configured\n");
	}

} // namespace SANSENSNODE_NAMESPACE