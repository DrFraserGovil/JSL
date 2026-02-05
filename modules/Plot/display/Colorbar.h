#pragma once
#include <string>
#include <format>
#include "../data/pipe.h"
#include "PerAxis.h"
namespace JSL::Plotting::Properties
{
    class Colorbar : public PerAxis
    {
        friend class Display;
        public:
            Colorbar() : PerAxis(false,"Colour") {};
            bool Visible = false;
            bool Inverted = false;
            std::string Palette = "viridis";
            void operator()()
            {
                Visible = true;
            }
            void Show()
            {
                Visible = true;
            }
            void Invert()
            {
                Inverted = true;
            }

            void Scheme(std::string gnuplt_cmd)
            {
                Palette = gnuplt_cmd;
            }
            void Gradient(std::string color1, std::string color2)
            {
                Palette = std::format("defined (0,\"{}\",1 \"{}\")",color1,color2);
            }
        private:
            void ToPipe(internal::GnuplotPipe pipe)
            {
                // pipe << "set palette " << Palette;
                // if (Inverted) pipe << " negative";

                if (Visible)
                {
                    pipe << "set colorbox\n";
                }
            }

    };
}