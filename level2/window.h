//
// Created by Salvo Passaro on 16/04/26.
// This file is part of mxgui
// and is licensed as the rest of this project.
//

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
    struct WindowPreferences
    {
        /**
         * Constructor
         * \param foreground foreground color
         * \param background background color
         * \param font default font
         */
        explicit WindowPreferences(short width = 300, short height = 300, Color foreground = white,
                                   Color background = black, Font font = defaultFont)
                                   : width(width), height(height), foreground(foreground), background(background),
                                     font(font) {}

        short width;
        short height;
        Color foreground; ///< Foreground color
        Color background; ///< Background color
        Font font;        ///< Default font
    };

    class Window final : public DrawableOwner {
        friend class WindowManager;
        using CloseFn = void(*)();

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
         */
        Window(Point p, WindowPreferences&& prefs);

        void clippedRedraw(DrawingContext& dc, const std::list<Rect>& requestedRects) const;

        void clippedRedraw(DrawingContext& dc) const { clippedRedraw(dc, visibleRects); }

        // This is different from clippedRedraw because it redraws all drawables, even not invalidated ones
        // This is used for example when the window is brought to foreground, but part of it was already visible,
        // so we redraw drawables in the regions hinted by the WindowManager.
        void clippedDraw(DrawingContext& dc, const std::list<Rect>& requestedRects) const;

        // This draws all visibleRegions of this window, and is used for example when the window is brought to foreground
        // Also, if widgets "move", they need to be redrawn even if they were not invalidated, because their position changed
        void clippedDraw(DrawingContext& dc) const { clippedDraw(dc, visibleRects); }

        /**
         * \internal
         * Called by the window manager to send user events to this window.
         * Do not call from user code
         * \param e event to post
         */
        void postEvent(Event e);

        void registerOnClose(const CloseFn f) { onClose = f; }
    public:

        /**
         * A handle to a window. This is what external code must use to interact with a window.
         */
        class Handle {
        public:
            void bringToFront() const;

            void close() const;

            void move(Point to) const;

            /**
             * Register a callback to be called when the window is about to be closed.
             *
             * Note that a single callback can be registered for each window.
             * @param f the callback to call when the window is about to be closed
             */
            void registerOnClose(const CloseFn f) const {
                if (const auto locked = w.lock()) {
                    locked->registerOnClose(f);
                }
            }

            /**
             * This is a convenience helper to run code that mutates something within the window, for example a drawable
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
        std::list<Rect> dirtyRects;              ///

        WindowPreferences prefs;                 ///< Window preferences

        CloseFn onClose = nullptr;
        bool closing = false;                    ///< True if the window is logically closed and its storage is still around
    };
}

#endif //MXGUI_LEVEL_2
