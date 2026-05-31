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
 * Simple Drawable object that paints the background of a Window or another widget.
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

} //namesapce mxgui

#endif //MXGUI_LEVEL_2
