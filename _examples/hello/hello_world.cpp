
#include "mxgui/entry.h"
#include <thread>
#include "mxgui/display.h"

using namespace mxgui;

ENTRY()
{
    {
        DrawingContext dc(DisplayManager::instance().getDisplay());
        dc.write(Point(0,1),"Hello World!");
    }
    for(;;)
        std::this_thread::sleep_for(std::chrono::seconds(1));  // otherwise compilers assume unreachable and emit ud2
}
