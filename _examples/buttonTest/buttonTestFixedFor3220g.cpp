#include <entry.h>
#include <level2/widgets/checkbox.h>
#include <level2/widgets/button.h>
#include <level2/widgets/radio_button.h>
#include <level2/widgets/label.h>

using namespace std;
using namespace mxgui;
using namespace mxgui::widgets;

ENTRY()
{
    int counter = 0;
    auto rg = widgets::RadioGroup();
    auto rg2 = widgets::RadioGroup();

    // Create a WindowPreference object to be moved into createWindow()
    WindowPreferences prefs;
    prefs.height = 315;
    prefs.width = 235;
    prefs.background = darkGrey;

    // Ask the WindowManager for a Window @ (5,5) and get a Window::Handle
    auto w = WindowManager::instance().createWindow({5, 5}, std::move(prefs));

    // Since in this demo w is the only window, let's terminate when it is closed!
    w.registerOnClose([]() { std::exit(0); });

    // Create a bunch of widgets inside the Window w:
    auto& b1 = w.make<widgets::Button>(Rect(Point(10, 10), Point(70, 40)), "Button 1");
    auto& l1 = w.make<widgets::Label>(Point(110, 10), 5, 20, "0");

    auto& c1 = w.make<widgets::CheckBox>(Point(10, 60), 15, "Check 1");
    auto& l2 = w.make<widgets::Label>(Point(110, 60), 80, 20, "false");
    auto& c2 = w.make<widgets::CheckBox>(Point(10, 80), 15, "Check 2", true);
    auto& l5 = w.make<widgets::Label>(Point(110, 80), 80, 20, "true");

    // Use references to Widgets inside a callback of a Widget belonging to the same Window!
    c1.setCallback([&l2,&c1]() {
        string s = "false";
        if (c1.isChecked())
            s = "true";
        l2.setText(s);
    });

    c2.setCallback([&l5,&c2]() {
        string s = "false";
        if (c2.isChecked())
            s = "true";
        l5.setText(s);
    });

    // Create even more widgets
    auto& r00 = w.make<widgets::RadioButton>(&rg2, Point(10, 110), 15, "Mario");
    auto& r01 = w.make<widgets::RadioButton>(&rg2, Point(10, 130), 15, "Luigi");
    auto& l4 = w.make<widgets::Label>(Point(110, 110), 70, 20, "None");
    auto cb2 = [&rg2,&l4]() { l4.setText(rg2.getChecked()->getLabel()); };

    r00.setCallback(cb2);
    r01.setCallback(cb2);

    auto& r1 = w.make<widgets::RadioButton>(&rg, Point(10, 160), 15, "Radio 1");
    auto& r2 = w.make<widgets::RadioButton>(&rg, Point(10, 200), 15, "Radio 2");
    auto& r3 = w.make<widgets::RadioButton>(&rg, Point(10, 240), 15, "Radio 3");

    auto& l3 = w.make<widgets::Label>(Point(110, 160), 70, 20, "Radio 1");
    auto cb = [&rg,&l3]() { l3.setText(rg.getChecked()->getLabel()); };

    r1.setCallback(cb);
    r2.setCallback(cb);
    r3.setCallback(cb);
    rg.setChecked(&r1);
    b1.setCallback([&l1,&counter]() {
        counter = ++counter % 10;
        l1.setText(to_string(counter));
    });
    
    Point pOld=Point(-1,-1);
    // At this point the WindowManager has already registered its internal callback. Let's overwrite it
    InputHandler::instance().registerEventCallback([&pOld](){
        Event e = InputHandler::instance().popEvent();
        //cout<<"Event: "<<e.getEvent()<<endl;
        Point p = e.getPoint();
        if(p==Point(0,319))
            p=pOld;
        e=Event(e.getEvent(),p);
        pOld=p;
        WindowManager::instance().pushMessage(WindowManager::WMMessage(std::move(e)));
    });

    // Start the WindowManager's event loop.
    WindowManager::instance().loop();

    // Code will never reach here!
}