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

#pragma once

#include <utility>
#include <vector>
#include <list>
#include "point.h"

namespace mxgui {

/**
 * \ingroup pub_iface_2
 * This class represents a simple rectangle on screen
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

    [[nodiscard]] bool empty() const {
        return first.x() >= second.x() || first.y() >= second.y();
    }

    [[nodiscard]] bool contains(const Point& p) const
    {
        return p.within(first,second);
    }

    [[nodiscard]] bool contains(const Rect& other) const
    {
        // An area contains another if the upper left point of the other area is within this area
        // and the lower right point of the other area is within this area
        // TODO: check if this is consistent with "strictness"
        return other.first.within(first,second) && other.second.within(first,second);
    }

    [[nodiscard]] Rect intersection(const Rect& other) const
    {
        Point newFirst(std::max(first.x(),other.first.x()),std::max(first.y(),other.first.y()));
        Point newSecond(std::min(second.x(),other.second.x()),std::min(second.y(),other.second.y()));

        if(newFirst.x() >= newSecond.x() || newFirst.y() >= newSecond.y())
            return {}; //Empty area

        return { newFirst, newSecond };
    }

    [[nodiscard]] Rect translate(const Point& p) const
    {
        return { first + p, second + p };
    }

    // let's do a difference s.t. we return one (or more!) new Rects.
    // more is because the difference of two rectangles can be a non-rectangular shape,
    // which we can represent as the union of multiple rectangles.
    // TODO: (optimization) we're guaranteed to have at most 4 rects,
    //       we probably should use a fixed-size array instead of a vector, to avoid dynamic memory allocation.
    [[nodiscard]] std::vector<Rect> operator- (const Rect& other) const
    {
        std::vector<Rect> result;

        const auto intersection = this->intersection(other);
        // If the areas don't intersect, the difference is just this area
        if(intersection.empty())
        {
            result.push_back(*this);
            return result;
        }

        // Otherwise, we need to calculate the difference.
        // We can have up to 4 new areas: top, bottom, left and right of the intersection area.

        // Top area
        if(first.y() < intersection.first.y())
            result.emplace_back(Point(first.x(), first.y()), Point(second.x(), intersection.first.y()));

        // Bottom area
        if(second.y() > intersection.second.y())
            result.emplace_back(Point(first.x(), intersection.second.y()), Point(second.x(), second.y()));

        // Left area
        if(first.x() < intersection.first.x())
            result.emplace_back(
                Point(first.x(), intersection.first.y()),
                Point(intersection.first.x(), intersection.second.y())
                );

        // Right area
        if(second.x() > intersection.second.x())
            result.emplace_back(
                Point(intersection.second.x(), intersection.first.y()),
                Point(second.x(), intersection.second.y())
                );

        return result;
    }

    static std::list<Rect> subtractRect(const std::list<Rect>& regions, const Rect& cover)
    {
        std::list<Rect> result;

        for (const auto& region : regions) {
            const auto intersection = region.intersection(cover);

            const auto pieces = region - intersection;
            for (const auto& piece : pieces) {
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
