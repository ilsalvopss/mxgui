//
// Created by Salvo Passaro on 16/04/26.
// This file is part of mxgui
// and is licensed as the rest of this project.
//

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

    Drawable::Drawable(BadgedRef<DrawableOwner> owner, Rect da) : owner(owner), da(std::move(da)) {}

    Drawable::Drawable(BadgedRef<DrawableOwner> owner, Point p, const short width, const short height)
        : owner(owner), da(std::make_pair(p,Point(p.x()+width-1,p.y()+height-1))) {}

    void Drawable::enqueueForRedraw() const {
        owner.needsPartialRedraw<Drawable>({}, *this);
    }
}

#endif // MXGUI_LEVEL_2