/***************************************************************************
 *   Copyright (C) 2014 by Terraneo Federico                               *
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

#include "drawing_context_proxy.h"

#ifdef MXGUI_LEVEL_2

namespace mxgui {

//
// class DrawingContextProxy
//

DrawingContextProxy::~DrawingContextProxy() = default;

//
// class FullScreenDrawingContextProxy
//

FullScreenDrawingContextProxy::FullScreenDrawingContextProxy(Display& display)
    : dc(display) {}

void FullScreenDrawingContextProxy::write(Point p, const char *text)
{
    dc.write(p,text);
}

void FullScreenDrawingContextProxy::clippedWrite(Point p, Point a, Point b, const char *text)
{
    dc.clippedWrite(p,a,b,text);
}

void FullScreenDrawingContextProxy::clear(Color color)
{
    dc.clear(color);
}

void FullScreenDrawingContextProxy::clear(Point p1, Point p2, Color color)
{
    dc.clear(p1,p2,color);
}

void FullScreenDrawingContextProxy::line(Point a, Point b, Color color)
{
    dc.line(a,b,color);
}

void FullScreenDrawingContextProxy::scanLine(Point p, const Color *colors, unsigned short length)
{
    dc.scanLine(p,colors,length);
}

Color *FullScreenDrawingContextProxy::getScanLineBuffer()
{
    return dc.getScanLineBuffer();
}

void FullScreenDrawingContextProxy::scanLineBuffer(Point p, unsigned short length)
{
    dc.scanLineBuffer(p,length);
}

void FullScreenDrawingContextProxy::drawImage(Point p, const ImageBase& img)
{
    dc.drawImage(p,img);
}

void FullScreenDrawingContextProxy::clippedDrawImage(Point p, Point a, Point b, const ImageBase& img)
{
    dc.clippedDrawImage(p,a,b,img);
}

void FullScreenDrawingContextProxy::drawRectangle(Point a, Point b, Color c)
{
    dc.drawRectangle(a,b,c);
}

short int FullScreenDrawingContextProxy::getHeight() const
{
    return dc.getHeight();
}

short int FullScreenDrawingContextProxy::getWidth() const
{
    return dc.getWidth();
}

void FullScreenDrawingContextProxy::setTextColor(std::pair<Color,Color> colors)
{
    dc.setTextColor(colors);
}

std::pair<Color,Color> FullScreenDrawingContextProxy::getTextColor() const
{
    return dc.getTextColor();
}

void FullScreenDrawingContextProxy::setFont(const Font& font)
{
    dc.setFont(font);
}

Font FullScreenDrawingContextProxy::getFont() const
{
    return dc.getFont();
}

//
// class ClippedDrawingContextProxy
//

void ClippedDrawingContext::write(const Point p, const char *text) {
    dc.clippedWrite(origin + p, clippingRect.first, clippingRect.second, text);
}

void ClippedDrawingContext::clippedWrite(const Point p, const Point a, const Point b, const char* text) {
    const auto clipped_a = clippingRect.intersection({origin + a, origin + b});
    if (clipped_a.empty())
        return; // requested clipping area is completely outside clippingRect, don't write anything

    dc.clippedWrite(origin + p, clipped_a.first, clipped_a.second, text);
}

void ClippedDrawingContext::clear(const Color color) {
    dc.clear(clippingRect.first, clippingRect.second, color);
}

void ClippedDrawingContext::clear(const Point p1, const Point p2, const Color color) {
    const auto intersection = clippingRect.intersection({origin + p1, origin + p2});
    if (intersection.empty())
        return; // area to clear is completely outside clippingRect, don't clear anything

    dc.clear(intersection.first, intersection.second, color);
}

void ClippedDrawingContext::line(const Point a, const Point b, const Color color) {
    dc.clippedLine(origin + a, origin + b, clippingRect.first, clippingRect.second, color);
}

void ClippedDrawingContext::scanLine(const Point p, const Color *colors, const unsigned short length) {
    const auto absolute_p = origin + p;
    if (absolute_p.y() < clippingRect.first.y() || absolute_p.y() >= clippingRect.second.y())
        return; // line is completely outside clippingRect (vertically), don't draw anything

    auto x0 = absolute_p.x();
    auto x1 = absolute_p.x() + length;

    if (x1 <= clippingRect.first.x() || x0 >= clippingRect.second.x())
        return; // line is completely outside clippingRect (horizontally), don't draw anything

    if (x0 < clippingRect.first.x()) {
        // line starts before clippingRect, skip the first pixels
        const auto skip = clippingRect.first.x() - x0;
        colors += skip;

        // update x0 to the first pixel inside clippingRect
        x0 = clippingRect.first.x();
    }

    if (x1 > clippingRect.second.x()) // line ends after clippingRect
        x1 = clippingRect.second.x(); // update x1 to the last pixel inside clippingRect

    dc.scanLine({ x0, absolute_p.y() }, colors, x1 - x0);
}

void ClippedDrawingContext::drawImage(const Point p, const ImageBase& img) {
    dc.clippedDrawImage(origin + p, clippingRect.first, clippingRect.second, img);
}

void ClippedDrawingContext::clippedDrawImage(const Point p, const Point a, const Point b, const ImageBase& img) {
    const auto intersection = clippingRect.intersection({origin + a, origin + b});
    if (intersection.empty())
        return; // image is completely outside clippingRect, don't draw anything

    dc.clippedDrawImage(origin + p, intersection.first, intersection.second, img);
}

void ClippedDrawingContext::drawRectangle(const Point a, const Point b, const Color c) {
    const auto absoluteRect = Rect{origin + a, origin + b};
    if (clippingRect.contains(absoluteRect)) {
        dc.drawRectangle(absoluteRect.first, absoluteRect.second, c);
        return;
    }

    if (clippingRect.intersection(absoluteRect).empty())
        return; // rectangle is completely outside clippingRect, don't draw anything

    line(a, {b.x(), a.y()}, c);  // top
    line({a.x(), b.y()}, b, c);   // bottom
    line(a, {a.x(), b.y()}, c);  // left
    line({b.x(), a.y()}, b, c);   // right
}

} //namespace miosix

#endif //MXGUI_LEVEL_2
