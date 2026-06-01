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

#include "mxgui_settings.h"
#include "../application.h"

#ifdef MXGUI_LEVEL_2

namespace mxgui::widgets {

/**
 * A mxgui::widgets::Image draws an image on the screen!
 */
class Image : public Drawable
{
    /**
     * Helper to compute DrawArea from top-left corner and ImageBase's properties
     * @param p top left
     * @param image image
     * @return a Rect with the space needed
     */
    static Rect computeDrawArea(const Point p, const ImageBase& image) {
        return {p, Point(p.x() + image.getWidth() - 1, p.y() + image.getHeight() - 1)};
    }
public:
    /**
     * Constructor
     * The object will be immediately enqueued for redraw
     * \param owner DrawableOwner to which this Image belongs
     * \param p top left corner where to draw this image
     * \param img the image (whose storage will also be managed by this widget)
     */
    template<typename ImageType>
    requires (std::is_base_of_v<ImageBase, ImageType> && !std::is_reference_v<ImageType>)
    Image(BadgedRef<DrawableOwner>&& owner, const Point p, ImageType&& img) :
    Drawable(std::move(owner), computeDrawArea(p, img)),
    img(std::make_unique<ImageType>(std::forward<ImageType>(img)))
    {
        enqueueForRedraw();
    }

    /**
     * Change the image being displayed
     * \param image new image to display
     */
    template<typename ImageType>
    requires (std::is_base_of_v<ImageBase, ImageType> && !std::is_reference_v<ImageType>)
    void setImage(ImageType&& image)
    {
        img = std::make_unique<ImageType>(std::forward<ImageType>(image));
        enqueueForRedraw();
    }

    /**
     * Overrides Drawable::isCompletelyOpaque to return true, since our onDraw() completely redraws the draw area.
     * @return true
     */
    constexpr bool isCompletelyOpaque() override {
        // if image transparency is supported in the future, dynamically decide here
        return true;
    }

private:
    /**
     * \internal
     * Overridden this member function to draw the object.
     * \param dc drawing context used to draw the object
     */
   void onDraw(DrawingContextProxy& dc) override {
       dc.drawImage(da.first, *img);
   }

    std::unique_ptr<ImageBase> img;
};

} //namesapce mxgui

#endif //MXGUI_LEVEL_2
