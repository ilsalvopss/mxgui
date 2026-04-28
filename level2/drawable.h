//
// Created by Salvo Passaro on 16/04/26.
// This file is part of mxgui
// and is licensed as the rest of this project.
//

#pragma once

#include "badge.h"
#include "drawing_context_proxy.h"
#include "draw_area.h"

#ifdef MXGUI_LEVEL_2

namespace mxgui {
    class Window;
    class Drawable;

    /**
     * A DrawableOwner is any object that can contain drawables, such as a Window or a Widget.
     * It provides the makeDrawable() function to create drawables and register them to the owner,
     * and the remove() function to remove them.
     *
     * It is the DrawableOwner's responsibility to manage the lifetime of the drawables it owns,
     * and to call their onDraw() function when needed.
     *
     * TODO: when implementing events, it is also the DrawableOwner's responsibility to forward events to the drawables it owns
     */
    class DrawableOwner {
        virtual void needsRedrawForRect(const Rect& r) = 0;

    protected:
        std::mutex drawables_mutex;
        std::list<std::unique_ptr<Drawable>> drawables;

    public:
        template<class T, class... Args>
        requires std::is_base_of_v<Drawable, T>
        T& makeDrawable(Args&&... args) {
            auto raw_drawable = new T(BadgedRef(*this), std::forward<Args>(args)...);
            {
                auto owned = std::unique_ptr<T>(raw_drawable);
                std::lock_guard lock(drawables_mutex);
                drawables.push_back(std::move(owned));
            }
            return *raw_drawable;
        }

        virtual Window& getWindow() = 0;

        void remove(const Drawable& d);

        /**
         * \internal
         * Called by a drawable to signal that it needs to be redrawn.
         * Do not call from user code
         * \param d drawable that needs to be redrawn
         */
        template<class T>
        requires std::is_base_of_v<Drawable, T>
        void needsPartialRedraw(Badge<T>, const T& d) {
            needsRedrawForRect(d.getDrawArea());
        }

        virtual ~DrawableOwner() = default;
    };

    /**
     * \ingroup pub_iface_2
     * Any object that can be drawn on screen has to extend Drawable
     */
    class Drawable
    {
    public:
        /**
         * \return the draw area of the object
         */
        [[nodiscard]] Rect getDrawArea() const { return da; }

        template<class T>
        requires std::is_base_of_v<DrawableOwner, T>
        void draw(Badge<T>, DrawingContextProxy& dc) { onDraw(dc); }

        /**
         * \internal
         * Called after onDraw() by the parent window when the Drawable is being
         * redrawn, do not call this directly.
         */
        template<class T>
        requires std::is_base_of_v<DrawableOwner, T>
        void redrawDone(Badge<T>) { onRedrawDone(); needRedraw=false; }

        /**
         * \internal
         * Override this member function to handle user input events. Called by the
         * parent Window, do not call this directly.
         * \param e event
         */
        virtual void onEvent(Badge<Window>, Event e);

        /**
         * \return true if this Drawable needs to be redrawn
         */
        [[nodiscard]] bool needsRedraw() const { return needRedraw; }

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
         * Override this member function to draw the object. Called by the parent DrawableOwner (and eventually by a Window).
         * Note that this is going to run inside the window manager thread, so avoid blocking
         * too much inside here and mind concurrent access to shared data.
         * \param dc drawing context used to draw the object
         */
        virtual void onDraw(DrawingContextProxy& dc)=0;

        /**
         * \internal
         * Override this member function to do custom work after you've been redrawn.
         * For example, a Drawable that is also a DrawableOwner and has child drawables may want to propagate this.
         */
        virtual void onRedrawDone() {}

        /**
         * Signal that this object needs to be redrawn
         */
        void enqueueForRedraw();

        DrawableOwner &owner;
        Rect da;         ///< Area on screen occupied by this object
        bool needRedraw; ///< True if this object needs to be redrawn
    };
}

#endif // MXGUI_LEVEL_2
