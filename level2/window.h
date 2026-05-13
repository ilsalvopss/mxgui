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

#include "badge.h"
#include "misc_inst.h"
#include "drawable.h"

#ifdef MXGUI_LEVEL_2

namespace mxgui {
    /**
     * \ingroup pub_iface_2
     * This class contains all the configuration options for a window
     */
    struct WindowPreferences {
        /**
         * Constructor
         * \param width width of the window
         * \param height height of the window
         * \param foreground foreground color
         * \param background background color
         * \param font default font
         */
        explicit WindowPreferences(short width = 300, short height = 300, Color foreground = white,
                                   Color background = black, Font font = defaultFont)
                                   : width(width), height(height), foreground(foreground), background(background),
                                     font(font) {}

        short width;      ///< Width of the window, in pixels
        short height;     ///< Height of the window, in pixels
        Color foreground; ///< Foreground color
        Color background; ///< Background color
        Font font;        ///< Default font
    };

    class Window final : public DrawableOwner {
        friend class WindowManager;
        using CloseFn = std::function<void()>;

        /**
         * \internal
         * Simple Drawable object that paints the background of a Window.
         */
        class SolidBackground final : public Drawable {
        public:
            SolidBackground(BadgedRef<DrawableOwner>&& owner, const Rect& da, const Color color)
                            : Drawable(std::move(owner), da), color(color) {}
        private:
            Color color;

            void onDraw(DrawingContextProxy& dc) override {
                dc.clear(color);
            }
        };

        /**
         * Constructor
         * \param p      initial position of the new Window (top-left)
         * \param prefs  a WindowPreferences object
         */
        Window(Point p, WindowPreferences&& prefs);

        /**
         * Draws anything belonging to this window.
         * Also, only draws areas requested via requestedRects.
         *
         * @param dc a reference to the DrawingContext through which the window should be drawn
         * @param requestedRects a list of Rects to limit the scope of this drawing
         */
        void clippedDraw(DrawingContext& dc, const std::list<Rect>& requestedRects) const;

        /**
         * Draws anything belonging to this window within all its visible region.
         *
         * @param dc a reference to the DrawingContext through which the window should be drawn
         */
        void clippedDraw(DrawingContext& dc) const;

        /**
         * \internal
         * Called by the window manager to send user events to this window.
         * \param e event to post
         */
        void postEvent(Event e) {
            hitTestAndDispatch(e);
        }

        /**
         * \internal
         * Registers the closing callback for this window.
         * @param f a callback with signature compatible with CloseFn
         */
        void registerOnClose(const CloseFn f) { onClose = f; }
    public:

        /**
         * \ingroup pub_iface_2
         * A handle to a window. This is what external code must use to interact with a window.
         */
        class Handle {
        public:
            /**
             * Requests the WindowManager to bring the associated window to the foreground.
             */
            void bringToFront() const;

            /**
             * Requests the WindowManager to close the associated window.
             */
            void close() const;

            /**
             * Requests the WindowManager to move the associated window to a new position.
             * @param to new position for this window
             */
            void move(Point to) const;

            /**
             * Registers a callback to be called when the window is about to be closed.
             *
             * Note that a single callback can be registered for each window.
             * Also note that the callback will run in the WindowManager thread so mind blocking too much inside here.
             * @param f the callback to call when the window is about to be closed
             */
            void registerOnClose(const CloseFn f) const {
                if (const auto locked = w.lock()) {
                    locked->registerOnClose(f);
                }
            }

            /**
             * This is a convenience helper to run code that mutates something within the window, for example a drawable,
             * while having a guarantee that the window is still alive.
             *
             * Note that if the code run inside here outlives the window, it will have no effect as the WindowManager
             * will ignore messages while still retaining (and wasting) resources.
             * Take a look at Handle::onClose() if you want to be notified when the window is actually closed.
             *
             * @tparam F a callable type that can be invoked with a Window& as argument,
             *           for example std::function<void(Window&)>, or a lambda like [](Window& w) { ... }
             * @param f the callable
             */
            template<class F>
            requires std::invocable<F, Window&>
            void whileAlive(const F& f) const {
                if (const auto locked = w.lock()) {
                    f(*locked);
                }
            }

            /**
             * Creates a Drawable of type T owned by the window pointed by this handle,
             * and forwards the arguments to the constructor of T.
             * A reference to the created drawable is returned for immediate use,
             * but the window will keep ownership of it and manage its lifetime.
             *
             * @tparam T Drawable type to create, must be derived from Drawable
             * @tparam Args Args types to forward to the constructor of T
             * @param args args to forward to the constructor of T
             * @return a reference to the created drawable
             */
            template<class T, class... Args>
            T& make(Args&&... args) const {
                if (const auto locked = w.lock()) {
                    return locked->makeDrawable<T>(std::forward<Args>(args)...);
                }

                throw std::runtime_error("This window handle is no longer valid");
            }

        private:
            std::weak_ptr<Window> w;
            friend class WindowManager;

            explicit Handle(const std::shared_ptr<Window>& w) : w(w) {}
        };

        Window& getWindow() override { return *this; }

        void needsRedrawForRect(const Rect& r) override;

        /**
         * \return the window preferences
         */
        [[nodiscard]] const WindowPreferences& getPreferences() const { return prefs; }

        /**
         * Destructor
         */
        ~Window() override = default;

    private:
        Point position;                          ///< Position of the upper left corner of the window
        Rect boundingBox;                        ///< Cached bounding box of the window

        std::list<Rect> visibleRects;            ///< List of visible regions on the window, used to optimize redraws
        std::list<Rect> dirtyRects;              ///< List of invalidated regions on the window, used to optimize redraws

        WindowPreferences prefs;                 ///< Window preferences

        CloseFn onClose = nullptr;
        bool closing = false;                    ///< True if the window is logically closed and its storage is still around
    };
}

#endif //MXGUI_LEVEL_2
