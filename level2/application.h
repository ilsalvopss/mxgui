/***************************************************************************
 *   Copyright (C) 2011, 2012, 2013, 2014 by Terraneo Federico             *
 *                                   2026 by Salvatore Passaro             *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   As a special exception, if other files instantiate templates or use   *
 *   macros or inline functions from this file, or you compile this file   *
 *   and link it with other works to produce a work based on this file,    *
 *   this file does not by itself cause the resulting work to be covered   *
 *   by the GNU General Public License. However the source code for this   *
 *   file must still be made available in accordance with the GNU General  *
 *   Public License. This exception does not invalidate any other reasons  *
 *   why a work based on this file might be covered by the GNU General     *
 *   Public License.                                                       *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, see <http://www.gnu.org/licenses/>   *
 ***************************************************************************/

#pragma once

#include <utility>
#include <list>
#include <functional>
#include <memory>
#include <mutex>
#include <condition_variable>
#include "mxgui_settings.h"
#include "display.h"
#include "input.h"

#include "window.h"

#ifdef MXGUI_LEVEL_2

namespace mxgui {

/**
 * \ingroup pub_iface_2
 * The window manager multiplexes display and events between windows
 */
class WindowManager
{
public:
    /**
     * A message sent to the window manager, either by the input handler or by windows themselves.
     */
    struct WMMessage {
        enum class Kind {
            Input,
            BringToFront,
            Close,
            Move,
            WakeRepaint,
        };

        Kind kind;
        Window* window = nullptr;
        Rect rect;
        Event event{};

        explicit WMMessage(Event&& e) : kind(Kind::Input), event(std::move(e)) {}

        WMMessage(Window* w, Rect r) : kind(Kind::WakeRepaint), window(w), rect(std::move(r)) {}
        WMMessage(Window* w, const Point to) : kind(Kind::Move), window(w),
                                               rect(Point(to.x(), to.y()), Point(to.x(), to.y())) {}
        WMMessage(Window* w, const Kind k) : kind(k), window(w) {}
    };

    /**
     * \return an instance of the window manager (singleton)
     */
    static WindowManager& instance();

    Window::Handle createWindow(Point p, WindowPreferences&& prefs);

    void closeWindow(Window& w);

    void bringToFront(Window& w);

    void moveWindow(Window& w, Point to);

    /**
     * This recomputes the visible regions of all windows, and optionally redraws them.
     * @param alsoDraw if true, a draw is performed after having recomputed the visible regions.
     */
    void recomputeVisibleRegions(bool alsoDraw = false);

    void pushMessage(WMMessage&& m);

    [[noreturn]] void loop();

    void setDisplay(Display& d);
    
private:
    WindowManager();

    Window& hitTest(Point p);

    std::reference_wrapper<Display> display;

    std::mutex message_mutex;
    std::condition_variable cond;
    std::list<WMMessage> messages;

    std::mutex stack_mutex;
    std::list<std::shared_ptr<Window> > stack;
};

} //namespace mxgui

#endif //MXGUI_LEVEL_2
