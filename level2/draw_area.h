/***************************************************************************
 *   Copyright (C) 2014 by Terraneo Federico                               *
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

#include <utility>
#include <list>
#include "point.h"

namespace mxgui {

/**
 * \ingroup pub_iface_2
 * This class represents a rectangular region
 * Inheritance from std::pair<Point,Point> is maintained for backward compatibility
 * \param first upper left point
 * \param second lower right point
 */
class Rect final : public std::pair<Point,Point>
{
public:
    // Empty rect constructor, yields an area with both points at (0,0)
    Rect() : std::pair<Point,Point>(Point(0,0),Point(0,0)) {}

    Rect(Point first, Point second) : std::pair<Point,Point>(first,second) {}
    explicit Rect(const std::pair<Point,Point>& p) : std::pair<Point,Point>(p) {}

    /**
     * Test if this is an empty area
     *
     * @return true if this area is empty
     */
    [[nodiscard]] bool empty() const {
        return first.x() >= second.x() || first.y() >= second.y();
    }

    /**
     * Test if point p is contained in this
     *
     * @param p point to check for containment
     * @return true if p is contained in this
     */
    [[nodiscard]] bool contains(const Point& p) const
    {
        return p.within(first,second);
    }

    /**
     * Test if other is contained in this
     *
     * @param other the area to check if it's contained in this
     * @return true if other in contained by this
     */
    [[nodiscard]] bool contains(const Rect& other) const
    {
        // An area contains another if the upper left point of the other area is within this area
        // and the lower right point of the other area is within this area
        return other.first.within(first,second) && other.second.within(first,second);
    }

    /**
     * Intersect this with other
     *
     * @param other region with which to intersect this
     * @return intersection
     */
    [[nodiscard]] Rect intersection(const Rect& other) const
    {
        Point newFirst(std::max(first.x(),other.first.x()),std::max(first.y(),other.first.y()));
        Point newSecond(std::min(second.x(),other.second.x()),std::min(second.y(),other.second.y()));

        if(newFirst.x() > newSecond.x() || newFirst.y() > newSecond.y())
            return {}; //Empty area

        return { newFirst, newSecond };
    }

    /**
     * Translate this by the offsets in p.
     *
     * @param p translation offset
     * @return translated Rect
     */
    [[nodiscard]] Rect translate(const Point& p) const
    {
        return { first + p, second + p };
    }

    /**
     * difference operator
     * the difference between two rectangles can be a non-rectangular shape
     * but can be represented by the union of multiple rects.
     *
     * Note: (optimization) we're guaranteed to have at most 4 rects,
     * so let's avoid a heap alloc and always return a fixed array :)
     * @param other rhs
     * @return array containing at most 4 non-empty rects
     */
    [[nodiscard]] std::array<Rect, 4> operator- (const Rect& other) const {
        std::array<Rect, 4> result;

        const auto intersection = this->intersection(other);
        // If the areas don't intersect, the difference is just this area
        if(intersection.empty()) {
            result[0] = *this;
            return result;
        }

        // Otherwise, we need to calculate the difference.
        // We can have up to 4 new areas: top, bottom, left and right of the intersection area.

        // Top area
        if(first.y() < intersection.first.y())
            result[0] = { Point(first.x(), first.y()), Point(second.x(), intersection.first.y()-1) };

        // Bottom area
        if(second.y() > intersection.second.y())
            result[1] = { Point(first.x(), intersection.second.y()+1), Point(second.x(), second.y()) };

        // Left area
        if(first.x() < intersection.first.x())
            result[2] = {
            Point(first.x(), intersection.first.y()),
            Point(intersection.first.x()-1, intersection.second.y())
            };

        // Right area
        if(second.x() > intersection.second.x())
            result[3] = {
            Point(intersection.second.x()+1, intersection.first.y()),
            Point(second.x(), intersection.second.y())
            };

        return result;
    }

    /**
     * Helper that subtracts a cover area from a list of regions.
     *
     * @param regions regions from which to subtract cover
     * @param cover region to subtract
     * @return resulting list of regions
     */
    static std::list<Rect> subtractRect(const std::list<Rect>& regions, const Rect& cover)
    {
        std::list<Rect> result;

        for (const auto& region : regions) {
            const auto intersection = region.intersection(cover);

            for (const auto pieces = region - intersection; const auto& piece : pieces) {
                if (!piece.empty())
                    result.push_back(piece);
            }
        }

        return result;
    }
};

/**
 * This class just encapsulates the Alignment_ enum so that the enum names don't
 * clobber the global namespace.
 */
class Alignment
{
public:
    /**
     * Possible alignments
     */
    enum Alignment_
    {
        TOP,
        BOTTOM,
        RIGHT,
        LEFT,
        CENTER
    };
    
private:
    Alignment(); //Just a wrapper class, disallow creating instances
};

} //namespace mxgui
