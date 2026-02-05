#pragma once

#include <string>
#include <format>
namespace JSL::Plotting
{
    enum class Bound
    {
        Upper,
        Lower
    };

    class RangeObject
    {
        private:
            bool _LowerAuto;
            bool _UpperAuto;
            double _Lower;
            double _Upper;
        public:
            RangeObject()
            {
                _LowerAuto=true;
                _UpperAuto=true;
            };
            RangeObject(double low, double high) : _Lower(low), _Upper(high)
            {
                _LowerAuto=false;
                _UpperAuto=false;
            }
            RangeObject(Bound bound, double value)
            {
                Auto();
                Set(bound,value);
            }
            void Auto()
            {
                _LowerAuto=true;
                _UpperAuto=true;
            }
            void Auto(Bound bound)
            {
                if (bound == Bound::Lower)
                {
                    _LowerAuto = true;
                }
                else
                {
                    _UpperAuto = true;
                }
            }
            std::pair<bool,bool> IsAuto()
            {
                return {_LowerAuto,_UpperAuto};
            }
            void Set()
            {
                Auto();
            }
            void Set(double low, double high)
            {
                SetLower(low);
                SetUpper(high);
            }
            void Set(Bound bound)
            {
                Auto(bound);
            }
            void Set(Bound bound, double val)
            {
                if (bound == Bound::Lower)
                {
                    SetLower(val);
                }
                else
                {
                    SetUpper(val);
                }
            }

            void operator()(double low, double high)
            {
                Set(low,high);
            }
            void operator()(Bound bound, double val)
            {
                Set(bound,val);
            }
            double Upper(){return _Upper;}
            void SetUpper(double value)
            {
                _UpperAuto = false;
                _Upper = value;
            }
            double Lower(){return _Lower;}
            void SetLower(double value)
            {
                _LowerAuto = false;
                _Lower = value;
            }
            std::string ToString()
            {
                auto cvt = [](bool check, double val){return check ? "*" : std::to_string(val);};
                return std::format("[{}:{}]",cvt(_LowerAuto,_Lower),cvt(_UpperAuto,_Upper));
            }
    };
}