#include <entry.h>
#include <memory>
#include <level2/application.h>
#include <level2/widgets/checkbox.h>
#include <level2/widgets/button.h>
#include <level2/widgets/radio_button.h>
#include <level2/widgets/label.h>
//#include <iostream>
using namespace std;
using namespace mxgui;


ENTRY()
{
    int counter=0;
    auto rg=widgets::RadioGroup();
    auto rg2=widgets::RadioGroup();
    shared_ptr<Window> w= make_shared<Window>();
    auto b1=widgets::Button(w.get(),Rect(Point(10,10),Point(70,40)),"Button 1");
    auto l1=widgets::Label(w.get(),Point(110,10),5,20,"0");

    auto c1=widgets::CheckBox(w.get(),Point(10,60),15,"Check 1");
    auto l2=widgets::Label(w.get(),Point(110,60),80,20,"false");
    auto c2=widgets::CheckBox(w.get(),Point(10,80),15,"Check 2",true);
    auto l5=widgets::Label(w.get(),Point(110,80),80,20,"true");

    c1.setCallback([&l2,&c1](){
        string s="false";
        if(c1.isChecked())
            s="true";
        l2.setText(s);
    });

    c2.setCallback([&l5,&c2](){
        string s="false";
        if(c2.isChecked())
            s="true";
        l5.setText(s);
    });

    auto r00=widgets::RadioButton(w.get(),&rg2,Point(10,110),15,"Mario");
    auto r01=widgets::RadioButton(w.get(),&rg2,Point(10,130),15,"Luigi");
    auto l4=widgets::Label(w.get(),Point(110,110),70,20,"None");
    auto cb2 = [&rg2,&l4]() { l4.setText(rg2.getChecked()->getLabel()); };

    r00.setCallback(cb2);
    r01.setCallback(cb2);
    
    auto r1=widgets::RadioButton(w.get(),&rg,Point(10,160),15,"Radio 1");
    auto r2=widgets::RadioButton(w.get(),&rg,Point(10,200),15,"Radio 2");
    auto r3=widgets::RadioButton(w.get(),&rg,Point(10,240),15,"Radio 3");
    
    auto l3=widgets::Label(w.get(),Point(110,160),70,20,"Radio 1");
    auto cb = [&rg,&l3]() { l3.setText(rg.getChecked()->getLabel()); };

    r1.setCallback(cb);
    r2.setCallback(cb);
    r3.setCallback(cb);
    rg.setChecked(&r1);
    b1.setCallback([&l1,&counter](){
        counter=(counter+1)%10;
        l1.setText(to_string(counter));
    });

    auto b2=widgets::Button(w.get(),Point(10,280),25,35,"Exit");
    b2.setCallback([w](){
        //brutal way to do it, but it's just an example
        w.get()->~Window();
    });

    // This must be wrong. No user code should register a callback to the input handler, as this will break the window manager.
    // The window manager should be the only one registering a callback to the input handler, and it should do it in its constructor.
    // If user code needs to be notified of an event, it should do it through the event loop of the window, which is what it's for.
    /*InputHandler::instance().registerEventCallback([w](){
        Event e = InputHandler::instance().popEvent();
        //cout<<"Event: "<<e.getEvent()<<endl;
        w.get()->postEvent(e);
    });*/
    WindowManager::instance().start(w);
    w.get()->eventLoop();

    return 0;
}