//
// Created by Salvo Passaro on 25/05/26.
//

#include "mxgui/entry.h"
#include <thread>
#include <cmath>

#include "misc_inst.h"
#include "level2/application.h"
#include "level2/widgets/button.h"
#include "level2/widgets/label.h"
#include "level2/widgets/simple_plot.h"
#include "level2/widgets/image.h"

#include "miosixlogohomepage.h"

using namespace mxgui;

void demo0(std::atomic<bool>& run) {
    auto prefs = WindowPreferences(160, 25, black, rgb565(242,221,227), defaultFont);
    auto w0 = WindowManager::instance().createWindow({30,140},std::move(prefs));
    w0.make<widgets::Label>(Point(0,0), 160, 25, "I am below the bouncing window").setXAlignment(Alignment::CENTER);

    prefs = WindowPreferences(100, 44, white, 2047);
    auto w1 = WindowManager::instance().createWindow({0,110}, std::move(prefs));

    prefs = WindowPreferences(160, 25, black, rgb565(242,221,227), defaultFont);
    auto w2 = WindowManager::instance().createWindow({30,240},std::move(prefs));
    w2.make<widgets::Label>(Point(0,0), 160, 25, "I am above the bouncing window").setXAlignment(Alignment::CENTER);

    constexpr int screenW = 240;
    constexpr int screenH = 200;
    constexpr int winW = 100;
    constexpr int winH = 44;

    int x = 0;
    int y = 0;
    int vx = 2;
    int vy = 1;

    while (run) {
        x += vx;
        y += vy;

        if (x <= 0) {
            x = 0;
            vx = -vx;
        } else if (x >= screenW - winW) {
            x = screenW - winW;
            vx = -vx;
        }

        if (y <= 0) {
            y = 0;
            vy = -vy;
        } else if (y >= screenH - winH) {
            y = screenH - winH;
            vy = -vy;
        }

        w1.move(Point(x, 120+y));

        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    w0.close();
    w1.close();
    w2.close();
}

void demo1(std::atomic<bool>& run) {
    std::array colors = {red, green, blue, black, darkGrey};

    std::vector<Window::Handle> windows;
    while (run) {
        // let's create a bunch of windows in the space defined by the rectangle with tl @ {0, 65}
        for (int i = 0; i < 5; ++i) {
            auto prefs = WindowPreferences(50, 50, white, colors[i]);
            auto w = WindowManager::instance().createWindow(
                {static_cast<short int>(i*20), static_cast<short int>(65 + i*40)},
                std::move(prefs));

            auto& label = w.make<widgets::Label>(Point(10,10), 30, 30, "#" + std::to_string(i));
            label.setXAlignment(Alignment::CENTER);

            windows.push_back(std::move(w));
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
        }

        std::this_thread::sleep_for(std::chrono::seconds(4));

        for (auto& w : windows) {
            w.close();
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
        }
        windows.clear();
    }
}

void demo2(std::atomic<bool>& run) {
    auto prefs = WindowPreferences(140, 80, white, red, defaultFont);
    auto w1 = WindowManager::instance().createWindow({0,130},std::move(prefs));
    w1.make<widgets::Label>(Point(10,30), 120, 20, "This is a window!").setXAlignment(Alignment::CENTER);
    w1.make<widgets::Button>(Point(110,5), 20, 20, "X").setCallback([w1] { w1.close(); });

    prefs = WindowPreferences(180, 80, black, green, defaultFont);
    auto w2 = WindowManager::instance().createWindow({20,150 },std::move(prefs));
    w2.make<widgets::Label>(Point(10,10), 160, 20, "This is another window!").setXAlignment(Alignment::CENTER);
    w2.make<widgets::Button>(Point(10,40), 160, 20, "Close me?").setCallback([w2] { w2.close(); });

    prefs = WindowPreferences(120, 110, white, blue, defaultFont);
    auto w3 = WindowManager::instance().createWindow({100,180 },std::move(prefs));

    prefs = WindowPreferences(80, 60, green, black, defaultFont);
    auto w4 = WindowManager::instance().createWindow({40,240 },std::move(prefs));
    auto long_str= "This label shouldn't really fit inside this window... right?";
    w4.make<widgets::Label>(Point(10,10), defaultFont.calculateLength(long_str), 20, long_str);

    prefs = WindowPreferences(30, 180, black, lightGrey, defaultFont);
    auto w5 = WindowManager::instance().createWindow({150,130 },std::move(prefs));

    while (run) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    w1.close();
    w2.close();
    w3.close();
    w4.close();
    w5.close();
}

void demo3(std::atomic<bool>& run) {
    auto prefs = WindowPreferences(240, 175, white, black);
    auto w = WindowManager::instance().createWindow({0,140},std::move(prefs));

    auto& plotter = w.make<widgets::SimplePlot>(Rect{{0,0}, {220,165}});

    int i=0;
    std::vector<float> data1;
    std::vector<float> data2;
    while (run) {
        data1.push_back(20*(1+0.05*i)*std::sin(0.1*i));
        data2.push_back(20*(1+0.05*i));

        plotter.plot({{data1, red}, {data2, green}});
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
        i += 2;
    }

    w.close();
}

ENTRY() {
    std::array demos = { &demo0, &demo1, &demo2, &demo3 };
    int demo_index = 0;
    std::atomic<bool> run_demo{true};

    auto prefs = WindowPreferences(240, 40, white, grey);
    auto status_bar = WindowManager::instance().createWindow({0,0},std::move(prefs));

    auto& next_demo_btn = status_bar.make<widgets::Button>(Point(10,9), 30, 20, ">>>");
    auto& demo_idx_label = status_bar.make<widgets::Label>(Point(60,10), 50, 20, "Demo #");

    next_demo_btn.setCallback([&run_demo, &demo_idx_label] {
        run_demo = false;
        demo_idx_label.setColors({red, grey});
    });

    std::thread demo_runner([&run_demo, &demo_index, &demos, &demo_idx_label] {
        for (;;) {
            run_demo = true;
            demo_idx_label.setText("Demo #" + std::to_string(demo_index % demos.size()));
            demo_idx_label.setColors({white, grey});
            demos[demo_index++ % demos.size()](run_demo);
        }
    });

    prefs = WindowPreferences(240, 30, black, rgb565(242,221,227));
    auto info_banner = WindowManager::instance().createWindow({0,50},std::move(prefs));

    info_banner.make<widgets::Label>(Point(10,5), 220, 20, "This is a demo of the windowing system!")
    .setXAlignment(Alignment::CENTER);

    prefs = WindowPreferences(220, 46, black, white);
    auto logo_banner = WindowManager::instance().createWindow({10,80},std::move(prefs));

    {
        auto logo = miosixlogohomepage;
        logo_banner.make<widgets::Image>(Point(0,0), std::move(logo));
    }

    std::thread ticker([status_bar] {
        status_bar.make<widgets::Label>(Point(160,13), defaultFont.calculateLength("Uptime: "), 15, "Uptime: ");
        auto& uptime = status_bar.make<widgets::Label>(Point(160 + defaultFont.calculateLength("Uptime: "),13), 40, 15, "0s");

        int fake_uptime = 0;
        status_bar.whileAlive([&](Window&) {
            for (;;) {
                auto str_time = std::to_string(fake_uptime++) + "s";
                uptime.setText(str_time);
                std::this_thread::sleep_for(std::chrono::seconds(1));
            }
        });
    });

    WindowManager::instance().loop();

    ticker.join();
    demo_runner.join();
}
