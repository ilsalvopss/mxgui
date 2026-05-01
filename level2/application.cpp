/***************************************************************************
 *   Copyright (C) 2011, 2012, 2013, 2014 by Terraneo Federico             *
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

#include "application.h"
#include <iostream>
#include <optional>
#include "misc_inst.h"

#ifdef MXGUI_LEVEL_2

using namespace std;

namespace mxgui {

WindowManager& WindowManager::instance()
{
    static WindowManager singleton;
    return singleton;
}

Window::Handle WindowManager::createWindow(const Point p, WindowPreferences&& prefs)
{
    const std::shared_ptr<Window> w(new Window(p, std::move(prefs)));
    w->visibleRects = { w->boundingBox };

    {
        std::scoped_lock lock(stack_mutex);
        stack.push_back(w); // new windows are created on top of the stack

        for (auto it = std::next(stack.rbegin()); it != stack.rend(); ++it) {
            const auto& other = *it;
            if (other->boundingBox.intersection(w->boundingBox).empty())
                continue; // if other doesn't overlap with w, we don't need to touch it

            // they somehow overlap! the intersection of their bounding boxes is now being covered by w
            // we need to remove that intersection from the visible regions of the other window
            std::list<Rect> updated_visible;
            for (const auto& region : other->visibleRects) {
                auto intersection = region.intersection(w->boundingBox);
                if (intersection.empty()) {
                    updated_visible.push_back(region);
                    continue; // no intersection, check next visible region
                }

                auto newVisibleRegions = region - intersection;
                for (const auto& newVisibleRegion : newVisibleRegions) {
                    if (!newVisibleRegion.empty())
                        updated_visible.push_back(newVisibleRegion);
                }
            }

            other->visibleRects = std::move(updated_visible);
        }
    }
    {
        auto dc = DrawingContext(display);
        w->clippedDraw(dc);
    }

    return Window::Handle { w };
}

void WindowManager::closeWindow(Window& w) {
    if (w.onClose)
        w.onClose();

    std::scoped_lock lock(stack_mutex);

    const auto victim = std::find_if(stack.begin(), stack.end(),
        [&w](const std::shared_ptr<Window>& ptr) { return ptr.get() == &w; });
    if (victim == stack.end())
        return;

    // windows below w are [begin, victim)
    for (auto it = stack.begin(); it != victim; ++it) {
        const auto& other = *it;

        if (other->boundingBox.intersection(w.boundingBox).empty())
            continue;

        auto dc = DrawingContext(display);

        for (const auto& region : w.visibleRects) {
            auto intersection = region.intersection(other->boundingBox);
            if (intersection.empty())
                continue;

            other->visibleRects.push_back(intersection);
            other->clippedDraw(dc, {intersection});
        }
    }

    stack.erase(victim);
}

void WindowManager::recomputeVisibleRegions(const bool alsoDraw)
{
    std::list<Rect> alreadyVisible;
    std::scoped_lock lock(stack_mutex);

    // topmost -> backmost
    for (auto it = stack.rbegin(); it != stack.rend(); ++it) {
        const auto& w = *it;

        std::list<Rect> visible;
        const Rect bounds = w->boundingBox;

        // the visible region of a window is the entirety of its bounding box
        // minus the parts that are covered by windows above it in the stack
        visible.push_back(bounds);

        // for each already visible region, remove it from the visible regions of this window
        for (const auto& cover : alreadyVisible) {
            if (visible.empty())
                break;

            if (bounds.intersection(cover).empty())
                continue;

            visible = Rect::subtractRect(visible, cover);
        }

        w->visibleRects = std::move(visible);
        if (alsoDraw) {
            auto dc = DrawingContext(display);
            w->clippedDraw(dc);
        }

        alreadyVisible.insert(
            alreadyVisible.end(),
            w->visibleRects.begin(),
            w->visibleRects.end()
        );
    }
}

void WindowManager::bringToFront(Window& w) {
    std::list<Rect> uncoveredRegions;

    {
        std::scoped_lock lock(stack_mutex);

        const auto it = std::find_if(stack.begin(), stack.end(), [&w](const std::shared_ptr<Window>& ptr) { return ptr.get() == &w; });
        if (it == stack.end())
            return; // window not found, do nothing TODO: maybe we should throw an exception instead?

        // The visible region of w is going to be the entirety of w, because we're bringing it to foreground.
        const auto visibleRegion = w.boundingBox;

        // for each window above w,
        for (auto above = std::next(it); above != stack.end(); ++above) {
            const auto& other = *above;

            if (other->boundingBox.intersection(visibleRegion).empty())
                continue; // if other doesn't overlap with w, we don't need to touch it

            // they somehow overlap! the intersection of their bounding boxes is now being covered by w
            // we need to remove that intersection from the visible regions of the other window
            std::list<Rect> updated;
            for (const auto& region: other->visibleRects) {
                auto intersection = region.intersection(visibleRegion);
                if (intersection.empty()) {
                    updated.push_back(region);
                    continue; // no intersection, check next visible region
                }

                // we have an intersection, we need to remove it from the visible regions of the other window
                uncoveredRegions.push_back(intersection);

                auto newVisibleRegions = region - intersection;
                for (const auto& newVisibleRegion: newVisibleRegions) {
                    if (!newVisibleRegion.empty())
                        updated.push_back(newVisibleRegion);
                }
            }

            other->visibleRects = std::move(updated);
        }

        stack.splice(stack.end(), stack, it);
    }

    w.visibleRects.clear();
    w.visibleRects.push_back(w.boundingBox);

    auto dc = DrawingContext(display);
    w.clippedDraw(dc, uncoveredRegions);
}

void WindowManager::moveWindow(Window& w, const Point to) {
    {
        std::scoped_lock lock(stack_mutex);

        const auto newBoundingBox = Rect {
            to,
            Point { static_cast<short int>(to.x() + w.prefs.width), static_cast<short int>(to.y() + w.prefs.height) }
        };

        w.position = to;
        w.boundingBox = newBoundingBox;
        //w.visibleRects = { newBoundingBox };
    }

    // It's heavy but for now it's at least correct.
    // The move requires a full stack traversal anyway
    recomputeVisibleRegions(true);
}

void WindowManager::pushMessage(WMMessage&& m)  {
    {
        std::lock_guard lock(message_mutex);

        if (m.window && m.window->closing)
            return;

        messages.push_back(std::move(m));
    }

    cond.notify_one();
}

Window& WindowManager::hitTest(Point p) {
    std::scoped_lock lock(stack_mutex);

    // traverse the stack topmost to bottom, and return the first window whose bounding box contains the point
    for (auto it = stack.rbegin(); it != stack.rend(); ++it) {
        if (const auto& w = *it; w->boundingBox.contains(p))
            return *w;
    }

    // shouldn't ever reach here: there's the desktop
}

void WindowManager::loop() {
    bool redrawNeeded = false;

    for (;;) {
        std::optional<WMMessage> msg;
        {
            std::unique_lock lock(message_mutex);
            cond.wait(lock, [&] {
                return !messages.empty() || redrawNeeded;
            });

            if (!messages.empty()) {
                msg = std::move(messages.front());
                messages.pop_front();
            }
        }

        if (msg) {
            switch (msg->kind) {
                case WMMessage::Kind::Close: {
                    auto* w = msg->window;
                    closeWindow(*w);
                    // so now no more messages related to this window should arrive as the window
                    // and its drawables are all destroyed

                    // except the ones that are already in the queue. let's drain them
                    {
                        std::unique_lock lock(message_mutex);
                        w->closing = true; // mark the window as closing, so that new messages are not accepted

                        messages.remove_if([w](const WMMessage& m) {
                            return m.window == w;
                        });
                    }
                }
                break;
                case WMMessage::Kind::WakeRepaint: {
                    auto* w = msg->window;
                    w->dirtyRects.push_back(msg->rect);
                    redrawNeeded = true;
                    continue;
                }
                break;
                case WMMessage::Kind::BringToFront: {
                    auto* w = msg->window;
                    bringToFront(*w); // create a non-owning shared_ptr
                    w->dirtyRects.clear();
                }
                break;
                case WMMessage::Kind::Move: {
                    auto* w = msg->window;
                    moveWindow(*w, msg->rect.first); // the new position is stored in the first point of the rect
                }
                break;
                case WMMessage::Kind::Input: {
                    const auto& e = msg->event;
                    if (e.hasValidPoint()) {
                        auto& w = hitTest(e.getPoint());
                        if (&w != stack.begin()->get())
                            bringToFront(w);
                        w.postEvent(Event::translate(e, -w.position));
                    }
                    if (e.hasValidKey()) {
                        const auto& w = *stack.end();
                        w->postEvent(e);
                    }
                }
                break;
                default: {
                    std::cout << "got unsupported event" << std::endl;
                }
                break;
            }
        }

        if (!redrawNeeded)
            continue;

        {
            auto dc = DrawingContext(display);

            std::scoped_lock stack_lock(stack_mutex);
            for (const auto& w: stack) {
                if (w->dirtyRects.empty())
                    continue;

                w->clippedDraw(dc, w->dirtyRects);
                w->dirtyRects.clear();
            }
        }

        redrawNeeded = false;
    }
}

void WindowManager::setDisplay(Display& d) {
    std::scoped_lock lock2(message_mutex);

    {
        std::scoped_lock lock(stack_mutex); // let's wait for any operations on the stack & drawing to finish

        display = d;
    }

    // TODO: resize all windows?

    recomputeVisibleRegions(true);
}

WindowManager::WindowManager() : display( DisplayManager::instance().getDisplay() ) {
    InputHandler::instance().registerEventCallback([this] {
        auto e = InputHandler::instance().popEvent();
        if (e.getEvent() == EventType::Default)
            return;

        pushMessage(WMMessage(std::move(e)));
    });

    WindowPreferences desktop;
    desktop.width = display.get().getWidth() - 1;
    desktop.height = display.get().getHeight() - 1;
    desktop.background = rgb565(242,221,227);
    createWindow({0, 0}, std::move(desktop));
}

} //namespace mxgui

#endif //MXGUI_LEVEL_2
