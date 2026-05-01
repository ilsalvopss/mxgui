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
    boundingBox( p, {static_cast<short int>(p.x() + prefs.width - 1), static_cast<short int>(p.y() + prefs.height - 1)} )
{
    makeDrawable<SolidBackground>(
        Rect { Point{0, 0}, Point{ static_cast<short int>(prefs.width - 1), static_cast<short int>(prefs.height - 1) } },
        prefs.background
        );
}

void Window::clippedDraw(DrawingContext& dc, const std::list<Rect>& requestedRects) const {
    for (const auto& requested: requestedRects) {
        for (const auto& visible : visibleRects) {
            auto requested_and_visible = requested.intersection(visible);
            if (requested_and_visible.empty())
                continue; // if the requested region doesn't intersect with this visible region, we don't need to draw it

            auto clippedDc = ClippedDrawingContext(dc, requested_and_visible, position);
            std::scoped_lock lock(drawables_mutex);

            for (const auto& drawable: drawables) {
                if (drawable->getDrawArea().intersection(requested_and_visible.translate(-position)).empty())
                    continue;

                drawable->draw<Window>({}, clippedDc);
            }
        }
    }
}

void Window::clippedDraw(DrawingContext& dc) const {
    for (const auto& visible : visibleRects) {
        auto clippedDc = ClippedDrawingContext(dc, visible, position);
        std::scoped_lock lock(drawables_mutex);

        for (const auto& drawable: drawables) {
            if (drawable->getDrawArea().intersection(visible.translate(-position)).empty())
                continue;

            drawable->draw<Window>({}, clippedDc);
        }
    }
}

void Window::needsRedrawForRect(const Rect& r) {
    auto e = WindowManager::WMMessage(this, r.translate(position));
    WindowManager::instance().pushMessage(std::move(e));
}

}

#endif //MXGUI_LEVEL_2
