    #pragma once
    #include <string>
    #include "../../Strings/split.h"
    #include "../../Strings/equals.h"
    #include "../data/pipe.h"
    namespace JSL::Plotting
    {
        class Display; //for friend purposes
        enum class Position : unsigned short int
        {
            None = 0,
            // Vertical
            Top    = 1 << 0,
            Bottom = 1 << 1,
            Center = 1 << 2,
            // Horizontal
            Left   = 1 << 3,
            Right  = 1 << 4,
            Middle = 1 << 5,
            // Layout
            Outside = 1 << 6,
            // Auto
            Auto    = 1 << 7
        };

        // Boilerplate to allow the '|' operator
        inline Position operator|(Position lhs, Position rhs) 
        {
            return static_cast<Position>(
                static_cast<unsigned short int>(lhs) | static_cast<unsigned short int>(rhs)
            );
        }

        // Boilerplate to allow the '&' operator (for checking flags)
        inline bool operator&(Position lhs, Position rhs) 
        {
            return static_cast<unsigned short int>(lhs) & static_cast<unsigned short int>(rhs);
        }


        class LegendObject
        {
            friend JSL::Plotting::Display;
            public:
                bool Active = false;
                int MaxColumns = 1;
                Position Location = Position::Auto;

                std::vector<std::pair<std::string,Position>> SearchVector;
                void SetLocation(std::string pos)
                {
                    Active = true;
                    if (SearchVector.size() == 0)
                    {
                        InitialiseSearch();
                    }
                    

                    Position loc = Position::None;
                    auto iterate = JSL::split(pos," ");
                    for (auto el : iterate)
                    {
                        for (size_t j = 0; j < SearchVector.size(); ++j)
                        {
                            if (JSL::insensitiveEquals(el,SearchVector[j].first))
                            {
                                loc = loc | SearchVector[j].second;
                            }
                        }
                    }
                    Location = loc;
                }

                void operator()()
                {
                    Active = true;
                }
                void operator()(Position loc)
                {
                    Active = true;
                    Location = loc;
                }
                void operator()(std::string loc)
                {
                    Active = true;
                    SetLocation(loc);
                }
            private:
                void InitialiseSearch()
                {
                    auto add = [&](std::vector<std::string> keys, Position pos){
                        for (auto key : keys) {SearchVector.push_back({key,pos});}
                    };

                    add({"top","north"},Position::Top);
                    add({"bottom","south"},Position::Bottom);
                    add({"left","west"},Position::Left);
                    add({"right","east"},Position::Right);
                    add({"center","centre","vcenter","vmiddle"},Position::Center);
                    add({"middle","hcenter","hmiddle"},Position::Middle);
                    add({"outside","outer","out"},Position::Outside);
                    add({"*","auto","automatic"},Position::Auto);

                }

                void ToPipe(internal::GnuplotPipe & pipe)
                {
                    if (Active)
                    {
                        
                        std::string vertical, horizontal, region;
                        region = (Location & Position::Outside) ? "outside" : "inside";
                        pipe << "set key " << region;
                        
                        if (!(Location & Position::Auto))
                        {
                            
                            
                            if (Location & Position::Center)        vertical =  "center";
                            else if (Location & Position::Bottom)   vertical =  "bottom";
                            else                                    vertical =  "top";
                            
                            if (Location & Position::Middle)        horizontal = "center";
                            else if (Location & Position::Right)    horizontal = "right";
                            else                                    horizontal = "left";
                            pipe << " " << horizontal << " " << vertical;
                        }
                        pipe <<  " horizontal maxrows " << MaxColumns <<"\n";
                    }
                    else
                    {
                        pipe << "unset key\n";
                    }
                }
        };
    }