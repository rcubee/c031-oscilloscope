#include "horizontal_dial.hpp"

std::string HorizontalDial::IndexToString(int index)
{
    osc_horizontal_scale horizontal_scale = IndexToHorizontalScale(index);

    switch (horizontal_scale) {
        case OSC_HORIZONTAL_SCALE_1s: return "1s";
        case OSC_HORIZONTAL_SCALE_500ms: return "500ms";
        case OSC_HORIZONTAL_SCALE_200ms: return "200ms";
        case OSC_HORIZONTAL_SCALE_100ms: return "100ms";
        case OSC_HORIZONTAL_SCALE_50ms: return "50ms";
        case OSC_HORIZONTAL_SCALE_20ms: return "20ms";
        case OSC_HORIZONTAL_SCALE_10ms: return "10ms";
        case OSC_HORIZONTAL_SCALE_5ms: return "5ms";
        case OSC_HORIZONTAL_SCALE_2ms: return "2ms";
        case OSC_HORIZONTAL_SCALE_1ms: return "1ms";
        case OSC_HORIZONTAL_SCALE_500us: return "500\u03BCs";
        case OSC_HORIZONTAL_SCALE_200us: return "200\u03BCs";
        case OSC_HORIZONTAL_SCALE_100us: return "100\u03BCs";
        case OSC_HORIZONTAL_SCALE_50us: return "50\u03BCs";
        case OSC_HORIZONTAL_SCALE_20us: return "20\u03BCs";
        case OSC_HORIZONTAL_SCALE_10us: return "10\u03BCs";
        default: throw std::runtime_error("");
    }
}

osc_horizontal_scale HorizontalDial::IndexToHorizontalScale(int index)
{
    switch (index)
    {
        case 0: return OSC_HORIZONTAL_SCALE_1s;
        case 1: return OSC_HORIZONTAL_SCALE_500ms;
        case 2: return OSC_HORIZONTAL_SCALE_200ms;
        case 3: return OSC_HORIZONTAL_SCALE_100ms;
        case 4: return OSC_HORIZONTAL_SCALE_50ms;
        case 5: return OSC_HORIZONTAL_SCALE_20ms;
        case 6: return OSC_HORIZONTAL_SCALE_10ms;
        case 7: return OSC_HORIZONTAL_SCALE_5ms;
        case 8: return OSC_HORIZONTAL_SCALE_2ms;
        case 9: return OSC_HORIZONTAL_SCALE_1ms;
        case 10: return OSC_HORIZONTAL_SCALE_500us;
        case 11: return OSC_HORIZONTAL_SCALE_200us;
        case 12: return OSC_HORIZONTAL_SCALE_100us;
        case 13: return OSC_HORIZONTAL_SCALE_50us;
        case 14: return OSC_HORIZONTAL_SCALE_20us;
        case 15: return OSC_HORIZONTAL_SCALE_10us;
        default: throw std::runtime_error("");
    }
}

int HorizontalDial::HorizontalScaleToIndex(osc_horizontal_scale horizontal_scale)
{
    switch (horizontal_scale) {
        case OSC_HORIZONTAL_SCALE_1s: return 0;
        case OSC_HORIZONTAL_SCALE_500ms: return 1;
        case OSC_HORIZONTAL_SCALE_200ms: return 2;
        case OSC_HORIZONTAL_SCALE_100ms: return 3;
        case OSC_HORIZONTAL_SCALE_50ms: return 4;
        case OSC_HORIZONTAL_SCALE_20ms: return 5;
        case OSC_HORIZONTAL_SCALE_10ms: return 6;
        case OSC_HORIZONTAL_SCALE_5ms: return 7;
        case OSC_HORIZONTAL_SCALE_2ms: return 8;
        case OSC_HORIZONTAL_SCALE_1ms: return 9;
        case OSC_HORIZONTAL_SCALE_500us: return 10;
        case OSC_HORIZONTAL_SCALE_200us: return 11;
        case OSC_HORIZONTAL_SCALE_100us: return 12;
        case OSC_HORIZONTAL_SCALE_50us: return 13;
        case OSC_HORIZONTAL_SCALE_20us: return 14;
        case OSC_HORIZONTAL_SCALE_10us: return 15;
        default: throw std::runtime_error("");
    }
}

HorizontalDial::HorizontalDial(QWidget* parent)
    : LabeledDial(true)
{
    setRange(0, OSC_HORIZONTAL_SCALE_COUNT - 1);
    setValue(HorizontalScaleToIndex(OSC_HORIZONTAL_SCALE_1ms)); // Default to 1ms
}

osc_horizontal_scale HorizontalDial::GetHorizontalScale()
{
    return IndexToHorizontalScale(value());
}

void HorizontalDial::SetHorizontalScale(osc_horizontal_scale horizontal_scale)
{
    setValue(HorizontalScaleToIndex(horizontal_scale));
}
