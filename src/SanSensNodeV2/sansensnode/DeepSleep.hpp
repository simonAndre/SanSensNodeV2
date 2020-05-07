/*
SImon ANDRE, wake-up methods
implementation specific to ESP32
*/

#pragma once
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
    uint8_t getWakeup_reason()
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

private:
    touch_pad_t touchPin;
    uint8_t _touchepadThreshold = 0; //0 means : no wake-up by touchepad
    uint8_t _touchPadGPIO;
    uint64_t _extPinsBitmask = 0x0; //0 means : no wake-up by external
    void (*_wakupcallback)(){wakeupCallback};

    void internalsetup()
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
    static void wakeupCallback()
    {
        logdebug("no wakeupCallback configured\n");
    }

    //     /*
    // Method to print the touchpad by which ESP32
    // has been awaken from sleep
    // */
    //     void print_wakeup_touchpad()
    //     {
    //         touchPin = esp_sleep_get_touchpad_wakeup_status();

    //         switch (touchPin)
    //         {
    //         case 0:
    //             logdebug("Touch detected on GPIO 4");
    //             break;
    //         case 1:
    //             logdebug("Touch detected on GPIO 0");
    //             break;
    //         case 2:
    //             logdebug("Touch detected on GPIO 2");
    //             break;
    //         case 3:
    //             logdebug("Touch detected on GPIO 15");
    //             break;
    //         case 4:
    //             logdebug("Touch detected on GPIO 13");
    //             break;
    //         case 5:
    //             logdebug("Touch detected on GPIO 12");
    //             break;
    //         case 6:
    //             logdebug("Touch detected on GPIO 14");
    //             break;
    //         case 7:
    //             logdebug("Touch detected on GPIO 27");
    //             break;
    //         case 8:
    //             logdebug("Touch detected on GPIO 33");
    //             break;
    //         case 9:
    //             logdebug("Touch detected on GPIO 32");
    //             break;
    //         default:
    //             logdebug("Wakeup not by touchpad");
    //             break;
    //         }
    //     }
};
} // namespace SANSENSNODE_NAMESPACE
