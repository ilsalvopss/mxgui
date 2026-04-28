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
        : owner(owner), da(std::make_pair(p,Point(p.x()+width,p.y()+height))), needRedraw(false) {}

    void Drawable::enqueueForRedraw()
    {
        needRedraw=true;
        owner.needsPartialRedraw<Drawable>({}, *this);
    }

    void Drawable::onEvent(Badge<Window>, Event e) {
        // unhandled event
    }
}

#endif // MXGUI_LEVEL_2