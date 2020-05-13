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
        void SetupTouchpadWakeup(uint8_t touchepadThreshold = 40, uint8_t touchPadGPIO = 13);

        /**
     * @brief configure a wekup source from a GPIO 
     *
     * 
     * @param gpioButton : Only RTC IO can be used as a source for external wake source. They are pins: 0,2,4,12-15,25-27,32-39.
     */
        void SetupExt1InterruptWakup(uint8_t gpioButton = 33);

        void SetupTimerWakeup(uint16_t sleeptime_sec);

        void HookWakupCallback(void (*wakupcallback)());

        /**
     * @brief go to deep sleep
     * 
     */
        void GotoSleep();

        /*
    Method to print the reason by which ESP32 has been awaken from sleep
    */
        uint8_t getWakeup_reason();

    private:
        touch_pad_t touchPin;
        uint8_t _touchepadThreshold = 0; //0 means : no wake-up by touchepad
        uint8_t _touchPadGPIO;
        uint64_t _extPinsBitmask = 0x0; //0 means : no wake-up by external
        void (*_wakupcallback)(){wakeupCallback};

        void internalsetup();
        static void wakeupCallback();
    };
} // namespace SANSENSNODE_NAMESPACE
