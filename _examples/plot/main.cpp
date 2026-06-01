
#include <entry.h>
#include <level2/widgets/simple_plot.h>
#include <unistd.h>
#include <cmath>
#include <thread>

#include "level2/application.h"
#include "level2/widgets/button.h"

using namespace std;
using namespace mxgui;

ENTRY()
{
    // Create a WindowPreference object to be moved into createWindow()
    WindowPreferences prefs;
    prefs.height = 300;
    prefs.width = 230;
    prefs.background = rgb565(0, 40, 40);

    // Ask the WindowManager for a Window @ (5,5) and get a Window::Handle
    auto w = WindowManager::instance().createWindow({5, 5}, std::move(prefs));
    auto& exit_btn = w.make<widgets::Button>(Rect{{15,270}, {215,290}}, "Close, please");
    exit_btn.setCallback([&w] { w.close(); });

    int i=0;
    vector<float> data1; 
    vector<float> data2;
    
    vector<widgets::SimplePlot::Dataset> dataset;
    dataset.push_back(widgets::SimplePlot::Dataset(data1,red));
    dataset.push_back(widgets::SimplePlot::Dataset(data2,green));

    auto& plotter = w.make<widgets::SimplePlot>(Rect{{5,5}, {220,260}});
    std::atomic<bool> go = true;
    w.registerOnClose([&go]() { go = false; });

    std::thread t([&] {
        w.whileAlive([&](Window&) {
            for(;;i+=2)
            {
                if (!go)
                    break;

                plotter.plot(dataset);

                data1.push_back(20*(1+0.05*i)*sin(0.1*i));
                data2.push_back(20*(1+0.05*i));
                usleep(100000);
            }
        });
    });
    t.detach();

    WindowManager::instance().loop();
}
