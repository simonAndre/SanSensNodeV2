/*
SImon ANDRE, wake-up methods
implementation specific to ESP32
*/

#pragma once
#include "../platform_logger.h"
namespace SANSENSNODE_NAMESPACE
{

class DeepSleep
{

public:
    DeepSleep()
    {
    }

    /**
 * @brief configure a wekup source from the touchepad 
 * 
 * @param touchepadThreshold : greater is more sensitive
 * @param touchPadGPIO (GPIO 4,0,2,15,13,12,14,27,33,32 only)
 */
    void SetupTouchpadWakeup(uint8_t touchepadThreshold = 40, uint8_t touchPadGPIO = 13)
    {
        _touchPadGPIO = touchPadGPIO;
        _touchepadThreshold = touchepadThreshold;
        internalsetup();
    }

    /**
     * @brief configure a wekup source from a GPIO 
     *
     * 
     * @param gpioButton : Only RTC IO can be used as a source for external wake source. They are pins: 0,2,4,12-15,25-27,32-39.
     */
    void SetupExt1InterruptWakup(uint8_t gpioButton = 33)
    {
        _extPinsBitmask = 2 ^ gpioButton;
        internalsetup();
    }

    void SetupTimerWakeup(uint16_t sleeptime_sec)
    {
        esp_sleep_enable_timer_wakeup(sleeptime_sec * 1000000L);
    }

    void HookWakupCallback(void (*wakupcallback)())
    {
        _wakupcallback = wakupcallback;
    }

    /**
 * @brief go to deep sleep
 * 
 */
    void GotoSleep()
    {
        esp_deep_sleep_start();
    }

    /*
Method to print the reason by which ESP32 has been awaken from sleep
*/
    void print_wakeup_reason()
    {
        esp_sleep_wakeup_cause_t wakeup_reason;

        wakeup_reason = esp_sleep_get_wakeup_cause();

        switch (wakeup_reason)
        {
        case ESP_SLEEP_WAKEUP_EXT0:
            loginfoLn("Wakeup caused by external signal using RTC_IO");
            break;
        case ESP_SLEEP_WAKEUP_EXT1:
            loginfoLn("Wakeup caused by external signal using RTC_CNTL");
            break;
        case ESP_SLEEP_WAKEUP_TIMER:
            loginfoLn("Wakeup caused by timer");
            break;
        case ESP_SLEEP_WAKEUP_TOUCHPAD:
            loginfoLn("Wakeup caused by touchpad");
            break;
        case ESP_SLEEP_WAKEUP_ULP:
            loginfoLn("Wakeup caused by ULP program");
            break;
        default:
            loginfoLn("Wakeup was not caused by deep sleep: %d\n", wakeup_reason);
            break;
        }

        if (_touchepadThreshold > 0)
            print_wakeup_touchpad();

        logflush();
    }

private:
    touch_pad_t touchPin;
    uint8_t _touchepadThreshold = 0; //0 means : no wake-up by touchepad
    uint8_t _touchPadGPIO;
    uint64_t _extPinsBitmask = 0x0; //0 means : no wake-up by external
    void (*_wakupcallback)();

    void internalsetup()
    {

        if (_touchepadThreshold > 0)
        {
            touchAttachInterrupt(_touchPadGPIO, _wakupcallback, _touchepadThreshold);
            //Configure Touchpad as wakeup source
            esp_sleep_enable_touchpad_wakeup();
        }
        if (_extPinsBitmask > 0)
            esp_sleep_enable_ext1_wakeup(_extPinsBitmask, ESP_EXT1_WAKEUP_ANY_HIGH);
    }
    static void wakeupCallback()
    {
        //placeholder wakeupCallback function
    }

    /*
Method to print the touchpad by which ESP32
has been awaken from sleep
*/
    void print_wakeup_touchpad()
    {
        touchPin = esp_sleep_get_touchpad_wakeup_status();

        switch (touchPin)
        {
        case 0:
            loginfoLn("Touch detected on GPIO 4");
            break;
        case 1:
            loginfoLn("Touch detected on GPIO 0");
            break;
        case 2:
            loginfoLn("Touch detected on GPIO 2");
            break;
        case 3:
            loginfoLn("Touch detected on GPIO 15");
            break;
        case 4:
            loginfoLn("Touch detected on GPIO 13");
            break;
        case 5:
            loginfoLn("Touch detected on GPIO 12");
            break;
        case 6:
            loginfoLn("Touch detected on GPIO 14");
            break;
        case 7:
            loginfoLn("Touch detected on GPIO 27");
            break;
        case 8:
            loginfoLn("Touch detected on GPIO 33");
            break;
        case 9:
            loginfoLn("Touch detected on GPIO 32");
            break;
        default:
            loginfoLn("Wakeup not by touchpad");
            break;
        }
    }
};
} // namespace SANSENSNODE_NAMESPACE
