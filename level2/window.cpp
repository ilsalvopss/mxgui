/***************************************************************************
 *   Copyright (C) 2026 by Salvatore Passaro                               *
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
            clippedDc.setTextColor({prefs.foreground, prefs.background});
            clippedDc.setFont(prefs.font);
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
        clippedDc.setTextColor({prefs.foreground, prefs.background});
        clippedDc.setFont(prefs.font);
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
