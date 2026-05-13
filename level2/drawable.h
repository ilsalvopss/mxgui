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

#pragma once

#include "input.h"
#include "badge.h"
#include "drawing_context_proxy.h"
#include "draw_area.h"
#include <mutex>
#include <memory>

#ifdef MXGUI_LEVEL_2

namespace mxgui {
    class Window;
    class DrawableOwner;

    /**
     * \ingroup pub_iface_2
     * Any object that can be drawn on screen has to extend Drawable
     */
    class Drawable
    {
    public:
        /**
         * Returns the draw area for this Drawable.
         * Note that you can override this if the Drawable you're implementing needs
         * to return something more complex than the area with which it was constructed.
         * \return the draw area of the object
         */
        [[nodiscard]] virtual Rect getDrawArea() const { return da; }

        /**
         * Drawing entry point accessible only to DrawableOwner(s)
         * It simply forwards the call to onDraw() but guarantees that no user code misuses the API.
         */
        template<class T>
        requires std::is_base_of_v<DrawableOwner, T>
        void draw(Badge<T>, DrawingContextProxy& dc) { onDraw(dc); }

        /**
         * Event handling entry point accessible only to DrawableOwner(s)
         * It simply forwards the call to onEvent() but guarantees that no user code misuses the API.
         */
        template<class T>
        requires std::is_base_of_v<DrawableOwner, T>
        void event(Badge<T>, Event& e) { onEvent(e); }

        // makeDrawable hands out a reference to Drawables for immediate use.
        // let's disallow copying
        Drawable(const Drawable&) = delete;
        Drawable& operator=(const Drawable&) = delete;

        /**
         * Destructor
         */
        virtual ~Drawable() = default;

    protected:
        /**
         * Constructor
         * \param owner DrawableOwner to which this object belongs
         * \param da area of the window occupied by this object
         */
        Drawable(BadgedRef<DrawableOwner> owner, Rect da);

        /**
         * Constructor
         * \param owner DrawableOwner to which this object belongs
         * \param p upper left point of the drawable in the window
         * \param width width of the drawable
         * \param height height of drawable
         */
        Drawable(BadgedRef<DrawableOwner> owner, Point p, short width, short height);

        /**
         * \internal
         * Override this member function to draw the object. Called by the parent DrawableOwner
         * (and eventually by a Window). Note that this is going to run inside the window manager thread,
         * so avoid blocking too much inside here and mind concurrent access to shared data.
         *
         * Note: the drawing context will use coordinates local to the Window at the top of your DrawableOwner's chain.
         * \param dc drawing context used to draw the object
         */
        virtual void onDraw(DrawingContextProxy& dc)=0;

        /**
         * \internal
         * Override this member function to handle user input events. Called by the parent DrawableOwner
         * (and eventually by a Window). Note that this is going to run inside the window manager thread,
         * so avoid blocking too much inside here and mind concurrent access to shared data.
         *
         * Unhandled events are dropped silently!
         * \param e event
         */
        virtual void onEvent(Event e) {}

        /**
         * Signal that this object needs to be redrawn
         */
        void enqueueForRedraw() const;

        DrawableOwner &owner;
        Rect da;         ///< Area on screen occupied by this object
    };

    /**
     * A DrawableOwner is any object that can contain drawables, such as a Window or a widget.
     * It provides the makeDrawable() function to create drawables and register them to the owner,
     * and the remove() function to remove them.
     *
     * It is the DrawableOwner's responsibility to manage the lifetime of the drawables it owns,
     * and to call their draw() and event() functions when appropiate.
     */
    class DrawableOwner {
        /**
         * Override this to decide how you want to redraw your drawables.
         * You should (probably) eventually forward this to your owner (which may be the Window);
         *
         * @param r region that needs to be redrawn, in the coordinate system of the Window
         *          at the top of the DrawableOwner's chain
         */
        virtual void needsRedrawForRect(const Rect& r) = 0;

    protected:
        mutable std::recursive_mutex drawables_mutex;
        std::list<std::unique_ptr<Drawable>> drawables;

        /**
         * Helper to indentify the Drawable responsible for handling e and dispatch the event
         *
         * @param e event to dispatch
         */
        void hitTestAndDispatch(Event& e);

    public:
        /**
         * Makes a Drawable of type T, with provided arguments, registers it to this owner and returns a reference
         * for immediate use.
         *
         * @tparam T Drawable type
         * @tparam Args construction argument types
         * @param args construction arguments
         * @return T& the drawable that was just created, ONLY for immediate use
         */
        template<class T, class... Args>
        requires std::is_base_of_v<Drawable, T>
        T& makeDrawable(Args&&... args) {
            std::lock_guard lock(drawables_mutex);
            auto raw_drawable = new T(BadgedRef(*this), std::forward<Args>(args)...);
            auto owned = std::unique_ptr<T>(raw_drawable);
            drawables.push_back(std::move(owned));
            return *raw_drawable;
        }

        /**
         * @return the Window on top of the DrawableOwner chain to which we belong
         */
        virtual Window& getWindow() = 0;

        /**
         * Removes d
         * @param d drawable to remove
         */
        void remove(const Drawable& d);

        /**
         * \internal
         * Called by a drawable to signal that it needs to be redrawn.
         * \param d drawable that needs to be redrawn
         */
        template<class T>
        requires std::is_base_of_v<Drawable, T>
        void needsPartialRedraw(Badge<T>, const T& d) {
            const auto area = d.getDrawArea();
            std::list<Rect> dirtyAndVisible = { area };  // TODO: think how to avoid a dynamic container here

            {
                std::scoped_lock lock(drawables_mutex);
                auto it = drawables.rbegin();
                for (; it != drawables.rend() && it->get() != &d; ++it) {
                    const auto intersection = (*it)->getDrawArea().intersection(area);
                    if (intersection.empty())
                        continue;

                    dirtyAndVisible = Rect::subtractRect(dirtyAndVisible, intersection);
                }

                if (it == drawables.rend()) {
                    // d doesn't intersect with anything else. full redraw
                    dirtyAndVisible = { area };
                }
            }

            for (const auto& r: dirtyAndVisible)
                if (!r.empty())
                    needsRedrawForRect(r);
        }

        virtual ~DrawableOwner() = default;
    };
}

#endif // MXGUI_LEVEL_2
