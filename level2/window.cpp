//
// Created by Salvo Passaro on 16/04/26.
// This file is part of mxgui
// and is licensed as the rest of this project.
//

#include "application.h"
#include "window.h"

#ifdef MXGUI_LEVEL_2

namespace mxgui {

void Window::Handle::bringToFront() const {
    if (const auto locked = w.lock()) {
        WindowManager::instance().pushMessage(
            WindowManager::WMMessage(locked.get(), WindowManager::WMMessage::Kind::BringToFront)
            );
    }
}

void Window::Handle::close() const {
    if (const auto locked = w.lock()) {
        WindowManager::instance().pushMessage(
            WindowManager::WMMessage(locked.get(), WindowManager::WMMessage::Kind::Close)
            );
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

void Window::clippedRedraw(DrawingContext& dc, const std::list<Rect>& requestedRects) const {
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

void Window::clippedDraw(DrawingContext& dc, const std::list<Rect>& requestedRects) const {
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

}

#endif //MXGUI_LEVEL_2
