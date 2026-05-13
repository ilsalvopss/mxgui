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

#include "input.h"
#include "drawable.h"

#ifdef MXGUI_LEVEL_2

namespace mxgui {
    //
    // class DrawableOwner
    //

    void DrawableOwner::hitTestAndDispatch(Event& e) {
        std::scoped_lock lock(drawables_mutex);
        auto it = drawables.rbegin();
        for (; it != std::prev(drawables.rend()); ++it) {
            if ((*it)->getDrawArea().contains(e.getPoint())) {
                break;
            }
        }

        (*it)->event<DrawableOwner>({}, e);
    }

    void DrawableOwner::remove(const Drawable& d) {
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

    Drawable::Drawable(BadgedRef<DrawableOwner> owner, Rect da) : owner(*owner), da(std::move(da)) {}

    Drawable::Drawable(BadgedRef<DrawableOwner> owner, Point p, const short width, const short height)
        : owner(*owner), da(std::make_pair(p,Point(p.x()+width-1,p.y()+height-1))) {}

    void Drawable::enqueueForRedraw() const {
        owner.needsPartialRedraw<Drawable>({}, *this);
    }
}

#endif // MXGUI_LEVEL_2