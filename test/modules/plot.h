#include "../../modules/Plot/plot.h"

#include <thread>
#include "../../modules/Time/Timer.h"
TEST_CASE("Plotting","[plot]")
{
    JSL::Plotting::Figure P;
    
    P.SetMultiplot(2,1);
    P.ScriptPath("tst.tst");
    // P.Persist(2);
    
    for (double s = 0; s < 0.1; s+= 0.1)
    {
        std::vector<double> x;
        std::vector<double> y;
        for (int i = 0; i < 20; ++i)
        {
            double xx = i*4.0/10-2;
            x.push_back(xx);
            y.push_back(xx*(xx-s)/10);
        }
        P.MoveTo(0);
        P.Clear();
        P.Add().Plot(x,y);
        // P.Set().X.Range = {JSL::Plotting::Bound::Upper,10};
        P.Set().X.Tics.FormatAsTime();
        P.Set().X.TimeParsingFormat = "%m.%d";
        P.Set().X.Tics.TimeFormat = "%m/%d";
        P.Set().X.Tics.Rotate= 45;
        P.Set().Y.Tics.Interval = 0.8;
        P.Set().X.LogScale();
        P.Set().Legend.Active = true;
        if (s == 0)
            {
            P.MoveTo(1);
            P.Add().Plot(x,x);
            P.Set().Y.Range(JSL::Plotting::Bound::Upper,25);
            P.Set().Title = "Testing!";
            P.Set().X.Tics.Labels = {{"hi",0},{7},{2.2}};
            }
        P.Set().Legend("left center");
        P.Add().CustomCommand("unset grid");
        P.Set().Y.Mirror();
        using namespace std::chrono_literals;
        std::this_thread::sleep_for(50ms);
        
        P.Show();
    }
    

}