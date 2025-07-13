#ifndef SBV300_ADC_H_ONCE
#define SBV300_ADC_H_ONCE

#include "MotatePins.h"

namespace Motate {

// ADCPinParent specializations for SBV300
template<>
struct ADCPinParent<kADC1_PinNumber> {
    static constexpr bool is_real = true;
    static constexpr uint32_t adcNumber = 1;
    static constexpr uint32_t adcMask = (1 << 1);
    
    // You'll need to implement these based on your microcontroller's ADC:
    void initPin(const uint32_t adcNumber, const PinOptions_t options = kNormal);
    int32_t getRawPin(const uint32_t adcNumber);
    int32_t getValuePin(const uint32_t adcNumber);
    int32_t getBottomPin(const uint32_t adcNumber);
    float getBottomVoltagePin(const uint32_t adcNumber);
    int32_t getTopPin(const uint32_t adcNumber);
    float getTopVoltagePin(const uint32_t adcNumber);
    void setVoltageRangePin(const uint32_t adcNumber, const float vref, const float min_expected, const float max_expected, const float ideal_steps);
    void setInterrupts(const uint32_t interrupts, const uint32_t adcInterruptMask);
    void startSampling();
};

template<>
struct ADCPinParent<kADC2_PinNumber> {
    static constexpr bool is_real = true;
    static constexpr uint32_t adcNumber = 2;
    static constexpr uint32_t adcMask = (1 << 2);

    // You'll need to implement these based on your microcontroller's ADC:
    void initPin(const uint32_t adcNumber, const PinOptions_t options = kNormal);
    int32_t getRawPin(const uint32_t adcNumber);
    int32_t getValuePin(const uint32_t adcNumber);
    int32_t getBottomPin(const uint32_t adcNumber);
    float getBottomVoltagePin(const uint32_t adcNumber);
    int32_t getTopPin(const uint32_t adcNumber);
    float getTopVoltagePin(const uint32_t adcNumber);
    void setVoltageRangePin(const uint32_t adcNumber, const float vref, const float min_expected, const float max_expected, const float ideal_steps);
    void setInterrupts(const uint32_t interrupts, const uint32_t adcInterruptMask);
    void startSampling();
};

// template<>
// struct ADCPinParent<kADC3_PinNumber> {
//     static constexpr bool is_real = true;
//     static constexpr uint32_t adcNumber = 3;
//     static constexpr uint32_t adcMask = (1 << 3);

//     // You'll need to implement these based on your microcontroller's ADC:
//     void initPin(const uint32_t adcNumber, const PinOptions_t options = kNormal);
//     int32_t getRawPin(const uint32_t adcNumber);
//     int32_t getValuePin(const uint32_t adcNumber);
//     int32_t getBottomPin(const uint32_t adcNumber);
//     float getBottomVoltagePin(const uint32_t adcNumber);
//     int32_t getTopPin(const uint32_t adcNumber);
//     float getTopVoltagePin(const uint32_t adcNumber);
//     void setVoltageRangePin(const uint32_t adcNumber, const float vref, const float min_expected, const float max_expected, const float ideal_steps);
//     void setInterrupts(const uint32_t interrupts, const uint32_t adcInterruptMask);
//     void startSampling();
// };

// template<>
// struct ADCPinParent<kADC4_PinNumber> {
//     static constexpr bool is_real = true;
//     static constexpr uint32_t adcNumber = 4;
//     static constexpr uint32_t adcMask = (1 << 4);

//     // You'll need to implement these based on your microcontroller's ADC:
//     void initPin(const uint32_t adcNumber, const PinOptions_t options = kNormal);
//     int32_t getRawPin(const uint32_t adcNumber);
//     int32_t getValuePin(const uint32_t adcNumber);
//     int32_t getBottomPin(const uint32_t adcNumber);
//     float getBottomVoltagePin(const uint32_t adcNumber);
//     int32_t getTopPin(const uint32_t adcNumber);
//     float getTopVoltagePin(const uint32_t adcNumber);
//     void setVoltageRangePin(const uint32_t adcNumber, const float vref, const float min_expected, const float max_expected, const float ideal_steps);
//     void setInterrupts(const uint32_t interrupts, const uint32_t adcInterruptMask);
//     void startSampling();
// };


} // namespace Motate

#endif