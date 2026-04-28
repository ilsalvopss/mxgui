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
#include "pthread_lock.h"
#include "misc_inst.h"

#ifdef MXGUI_LEVEL_2

using namespace std;

namespace mxgui {

//
// class DrawableOwner
//

void DrawableOwner::remove(const Drawable& d){
    const auto rectBeingRemoved = d.getDrawArea();

    // this is probably a bit slow.. but Drawable removal? seems pretty rare!
    {
        std::scoped_lock lock(drawables_mutex);
        drawables.remove_if([&d](const std::unique_ptr<Drawable>& ptr) { return ptr.get() == &d; });
    }

    needsRedrawForRect(rectBeingRemoved);
}

//
// class Drawable
//

Drawable::Drawable(BadgedRef<DrawableOwner> owner, Rect da) : owner(owner), da(std::move(da)), needRedraw(false) {}

Drawable::Drawable(BadgedRef<DrawableOwner> owner, Point p, const short width, const short height)
    : owner(owner), da(make_pair(p,Point(p.x()+width,p.y()+height))), needRedraw(false) {}

void Drawable::enqueueForRedraw()
{
    needRedraw=true;
    owner.needsPartialRedraw<Drawable>({}, *this);
}

void Drawable::onEvent(Badge<Window>, Event e) {
    // unhandled event
}

//
// class Window
//

void Window::Handle::bringToFront() const {
    if (const auto locked = w.lock()) {
        WindowManager::instance().pushMessage(WindowManager::WMMessage(locked.get(), WindowManager::WMMessage::Kind::BringToFront));
    }
}

void Window::Handle::close() const {
    if (const auto locked = w.lock()) {
        WindowManager::instance().pushMessage(WindowManager::WMMessage(locked.get(), WindowManager::WMMessage::Kind::Close));
    }
}

void Window::Handle::move(const Point to) const {
    if (const auto locked = w.lock()) {
        WindowManager::instance().pushMessage(WindowManager::WMMessage(locked.get(), to));
    }
}

Window::Window(Point p, WindowPreferences&& prefs) : prefs(prefs), position(p),
    boundingBox( p, {static_cast<short int>(p.x() + prefs.width), static_cast<short int>(p.y() + prefs.height)} )
{
    makeDrawable<SolidBackground>(
        Rect { Point{0, 0}, Point{ prefs.width, prefs.height } },
        prefs.background
        );
}

void Window::clippedRedraw(DrawingContext& dc, const std::list<Rect>& requestedRects) {
    for (const auto& requested: requestedRects) {
        for (const auto& visible : visibleRects) {
            auto requested_and_visible = requested.intersection(visible);
            if (requested_and_visible.empty())
                continue; // if the requested region doesn't intersect with this visible region, we don't need to draw it

            const auto localRegion = requested_and_visible.translate(-position);
            auto clippedDc = ClippedDrawingContext(dc, requested_and_visible, position);
            for (auto& drawable: drawables) {
                if (!drawable->needsRedraw())
                    continue;

                if (drawable->getDrawArea().intersection(localRegion).empty())
                    continue;

                drawable->draw<Window>({}, clippedDc);
            }
        }
    }

    for (const auto& drawable: drawables) {
        if (drawable->needsRedraw())
            drawable->redrawDone<Window>({});
    }
}

void Window::clippedDraw(DrawingContext& dc, const std::list<Rect>& requestedRects) {
    for (const auto& requested: requestedRects) {
        for (const auto& visible : visibleRects) {
            auto requested_and_visible = requested.intersection(visible);
            if (requested_and_visible.empty())
                continue; // if the requested region doesn't intersect with this visible region, we don't need to draw it

            auto clippedDc = ClippedDrawingContext(dc, requested_and_visible, position);
            for (const auto& drawable: drawables) {
                if (drawable->getDrawArea().intersection(requested_and_visible.translate(-position)).empty())
                    continue;

                drawable->draw<Window>({}, clippedDc);
            }

            /*for (const auto& drawable : drawables) {
                if (drawable->needsRedraw())
                    drawable->redrawDone<Window>({});
            }*/
        }
    }

    //redrawNeeded=false;
}

void Window::needsRedrawForRect(const Rect& r) {
    auto e = WindowManager::WMMessage(this, r.translate(position));
    WindowManager::instance().pushMessage(std::move(e));
}

//
// class WindowManager
//

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
