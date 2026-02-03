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
        P.Set().Range.X(-1,4);
        if (s == 0)
            {
            P.MoveTo(1);
            P.Add().Plot(x,x);
            P.Set().Range.Y(JSL::Plotting::Bound::Upper,25);
            }
        using namespace std::chrono_literals;
        std::this_thread::sleep_for(50ms);
        
        P.Show();
    }
    

}