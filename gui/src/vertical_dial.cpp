#include "vertical_dial.hpp"

float VerticalScaleToMultiplier(VerticalScale vertical_scale)
{
    switch (vertical_scale) {
        case VerticalScale::_5: return 0.2f;
        case VerticalScale::_2: return 0.5f;
        case VerticalScale::_1: return 1.f;
        case VerticalScale::_500m: return 2.f;
        case VerticalScale::_200m: return 5.f;
        case VerticalScale::_100m: return 10.f;
        case VerticalScale::_50m : return 20.f;
        case VerticalScale::_20m : return 50.f;
        case VerticalScale::_10m : return 100.f;
        case VerticalScale::_5m  : return 200.f;
        case VerticalScale::_2m  : return 500.f;
        case VerticalScale::_1m  : return 1000.f;
        case VerticalScale::_500u: return 2000.f;
        default: throw std::runtime_error("");
    }
}

std::string VerticalDial::IndexToString(int index)
{
    VerticalScale vertical_scale = IndexToVerticalScale(index);

    switch (vertical_scale) {
        case VerticalScale::_5: return "5V";
        case VerticalScale::_2: return "2V";
        case VerticalScale::_1: return "1V";
        case VerticalScale::_500m: return "500mV";
        case VerticalScale::_200m: return "200mV";
        case VerticalScale::_100m: return "100mV";
        case VerticalScale::_50m: return "50mV";
        case VerticalScale::_20m: return "20mV";
        case VerticalScale::_10m: return "10mV";
        case VerticalScale::_5m: return "5mV";
        case VerticalScale::_2m: return "2mV";
        case VerticalScale::_1m: return "1mV";
        case VerticalScale::_500u: return "500\u03BC";
        default: throw std::runtime_error("");
    }
}

VerticalScale VerticalDial::IndexToVerticalScale(int index)
{
    switch (index) {
        case 0: return VerticalScale::_5;
        case 1: return VerticalScale::_2;
        case 2: return VerticalScale::_1;
        case 3: return VerticalScale::_500m;
        case 4: return VerticalScale::_200m;
        case 5: return VerticalScale::_100m;
        case 6: return VerticalScale::_50m;
        case 7: return VerticalScale::_20m;
        case 8: return VerticalScale::_10m;
        case 9: return VerticalScale::_5m;
        case 10: return VerticalScale::_2m;
        case 11: return VerticalScale::_1m;
        case 12: return VerticalScale::_500u;
        default: throw std::runtime_error("");
    }
}

VerticalDial::VerticalDial(QWidget* parent)
    : LabeledDial(true)
{
    setRange(0, 12);
    setValue(2); // Default to 1V
}

VerticalScale VerticalDial::GetVerticalScale()
{
    return IndexToVerticalScale(value());
}
