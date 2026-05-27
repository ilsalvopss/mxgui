#include "mxgui/entry.h"

#include "misc_inst.h"
#include "level2/application.h"
#include "level2/widgets/button.h"
#include "level2/widgets/label.h"

#include <cstdlib>

using namespace mxgui;

ENTRY() {
    // Create a WindowPreference object to be moved into createWindow()
    WindowPreferences prefs;
    prefs.height = 100;
    prefs.width = 200;
    prefs.background = darkGrey;

    // Ask the WindowManager for a Window @ (20,20) and get a Window::Handle
    auto w = WindowManager::instance().createWindow({20, 20}, std::move(prefs));

    // Since in this demo w is the only window, let's brutally terminate when it is closed!
    w.registerOnClose([]() { std::exit(0); });

    // Create a label widget inside the Window w:
    w.make<widgets::Label>(Point(10, 15), 180, 20, "Hello, world!").setXAlignment(Alignment::CENTER);

    // Create a Button, with a callback!
    auto& b1 = w.make<widgets::Button>(Rect(Point(45, 40), Point(150, 80)), "Cool, move me!");
    b1.setCallback([w] {
        // Move the window to a random position when the button is pressed
        const auto newPos = Point(std::rand() % 25, std::rand() % 100);
        w.move(newPos);
    });

    // Start the WindowManager's event loop.
    WindowManager::instance().loop();

    // Code will never reach here!
}
