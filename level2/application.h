/***************************************************************************
 *   Copyright (C) 2011, 2012, 2013, 2014 by Terraneo Federico             *
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

#include <utility>
#include <vector>
#include <list>
#include <functional>
#include <iostream>
#include <memory>
#include "mxgui_settings.h"
#include "display.h"
#include "input.h"
#include "drawing_context_proxy.h"
#include "misc_inst.h"
#include "../badge.h"

#ifdef MXGUI_LEVEL_2

namespace mxgui {

//forward decls
class Window;
class WindowManager;
class Drawable;

/**
 * \ingroup pub_iface_2
 * This class contains all the configuration options for a window
 */
class WindowPreferences
{
public:
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

/**
 * \ingroup pub_iface_2
 * Windows are the central point of mxgui level 2, this part of the library
 * allows multiple applications to run concurrently and share the same display.
 * Applications are classes that derive from Window.
 */
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

    void clippedRedraw(DrawingContext& dc, const std::list<Rect>& requestedRects);

    void clippedRedraw(DrawingContext& dc) { clippedRedraw(dc, visibleRects); }

    // This is different from clippedRedraw because it redraws all drawables, even not invalidated ones
    // This is used for example when the window is brought to foreground, but part of it was already visible,
    // so we redraw drawables in the regions hinted by the WindowManager.
    void clippedDraw(DrawingContext& dc, const std::list<Rect>& requestedRects);

    // This draws all visibleRegions of this window, and is used for example when the window is brought to foreground
    // Also, if widgets "move", they need to be redrawn even if they were not invalidated, because their position changed
    void clippedDraw(DrawingContext& dc) { clippedDraw(dc, visibleRects); }

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

/**
 * \ingroup pub_iface_2
 * The window manager multiplexes display and events between windows
 */
class WindowManager
{
public:
    struct WMMessage {
        enum class Kind {
            Input,
            BringToFront,
            Close,
            WakeRepaint,
        };

        Kind kind;
        Window* window = nullptr;
        Rect rect;
        Event event{};

        explicit WMMessage(Event&& e) : kind(Kind::Input), event(std::move(e)) {}
        WMMessage(Window* w, Rect r) : kind(Kind::WakeRepaint), window(w), rect(std::move(r)) {}
        WMMessage(Window* w, const Kind k) : kind(k), window(w) {}
    };

    /**
     * \return an instance of the window manager (singleton)
     */
    static WindowManager& instance();

    Window::Handle createWindow(Point p, WindowPreferences&& prefs);

    void closeWindow(Window& w);

    void bringToFront(Window& w);

    /**
     * This recomputes the visible regions of all windows, and optionally redraws them.
     * @param alsoDraw if true, a draw is performed after having recomputed the visible regions.
     */
    void recomputeVisibleRegions(bool alsoDraw = false);

    void pushMessage(WMMessage&& m) {
        {
            std::lock_guard lock(message_mutex);

            if (m.window && m.window->closing)
                return;

            messages.push_back(std::move(m));
        }

        cond.notify_one();
    }

    [[noreturn]] void loop() {
        bool redrawNeeded = false;

        for (;;) {
            std::optional<WMMessage> msg;
            {
                std::unique_lock lock(message_mutex);
                cond.wait(lock, [&] {
                    return !messages.empty() || redrawNeeded;
                });

                if (!messages.empty()) {
                    msg = std::move(messages.front());
                    messages.pop_front();
                }
            }

            if (msg) {
                switch (msg->kind) {
                    case WMMessage::Kind::Close: {
                        auto* w = msg->window;
                        closeWindow(*w);
                        // so now no more messages related to this window should arrive as the window
                        // and its drawables are all destroyed

                        // except the ones that are already in the queue. let's drain them
                        {
                            std::unique_lock lock(message_mutex);
                            w->closing = true; // mark the window as closing, so that new messages are not accepted

                            messages.remove_if([w](const WMMessage& m) {
                                return m.window == w;
                            });
                        }
                    } break;
                    case WMMessage::Kind::WakeRepaint: {
                        auto* w = msg->window;
                        w->dirtyRects.push_back(msg->rect);
                        redrawNeeded = true;
                        continue;
                    } break;
                    case WMMessage::Kind::BringToFront: {
                        auto* w = msg->window;
                        bringToFront(*w); // create a non-owning shared_ptr
                        w->dirtyRects.clear(); // FIXME: what??
                    } break;
                    default: {
                        std::cout << "got unsupported event" << std::endl;
                    } break;
                }
            }

            if (!redrawNeeded)
                continue;

            {
                auto dc = DrawingContext(display);

                std::scoped_lock stack_lock(stack_mutex);
                for (const auto& w : stack) {
                    if (w->dirtyRects.empty())
                        continue;

                    w->clippedRedraw(dc, w->dirtyRects);
                    w->dirtyRects.clear();
                }
            }

            redrawNeeded = false;
        }
    }

    void setDisplay(Display& d) {
        std::scoped_lock lock2(message_mutex);

        {
            std::scoped_lock lock(stack_mutex); // let's wait for any operations on the stack & drawing to finish

            display = d;
        }

        // TODO: resize all windows?

        recomputeVisibleRegions(true);
    }
    
private:
    WindowManager();

    std::reference_wrapper<Display> display;

    std::mutex message_mutex;
    std::condition_variable cond;
    std::list<WMMessage> messages;

    std::mutex stack_mutex;
    std::list<std::shared_ptr<Window> > stack;
};

} //namespace mxgui

#endif //MXGUI_LEVEL_2
