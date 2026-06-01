/***************************************************************************
 *   Copyright (C) 2015 by Terraneo Federico                               *
 *                 2026 by Salvatore Passaro                               *
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

#include <vector>
#include <string>
#include <misc_inst.h>

#include "../drawable.h"

namespace mxgui::widgets {

class SimplePlot : public Drawable
{
public:
    class Dataset
    {
    public:
        Dataset(const std::vector<float>& data, Color color)
            : data(&data), color(color) {}

        const std::vector<float>* data;
        Color color;
    };

    SimplePlot(BadgedRef<DrawableOwner>&& owner, Rect da);
    
    void plot(const std::vector<float>& data, Color color=white, bool fullRedraw=false);
    
    void plot(const std::vector<Dataset>& dataset, bool fullRedraw=false);

    void setFont(const Font& font) { this->font=font; }
    

    /**
     * Overrides Drawable::isCompletelyOpaque to return true, since our onDraw() completely redraws the draw area.
     * @return true
     */
    constexpr bool isCompletelyOpaque() override { return true; }

    Point upperLeft;
    Point lowerRight;
    Font font;
    Color foreground;
    Color background;
    
    float ymin;
    float ymax;
    
private:
    void onDraw(DrawingContextProxy& dc) override;

    std::mutex data_mutex;
    std::string number(float num);
    std::vector<Dataset> dataset;
    int numElem;
    bool first;

    float prevYmin,prevYmax;
};

} //namespace mxgui
