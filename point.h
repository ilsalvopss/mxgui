/***************************************************************************
 *   Copyright (C) 2010, 2011 by Terraneo Federico                         *
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
#include <ostream>
namespace mxgui {

/**
 * \ingroup pub_iface
 * Point class. Points are immutable except they can be assigned with operator=
 */
class Point
{
public:
    /**
     * Constructor, create an instance of the Point class given x and y
     */
    Point(short int x, short int y): x_(x), y_(y) {}

    /**
     * Default constructor, yields a point to (0,0)
     */
    Point(): x_(0), y_(0) {}

    /**
     * \return x coordinate
     */
    short int x() const { return x_; }

    /**
     * \return the y coordinate
     */
    short int y() const { return y_; }

    /**
     * Compare two points for equality
     */
    bool operator== (const Point& p) const
    {
        return (this->x_ == p.x_) && (this->y_ == p.y_);
    }

    /**
     * Compare two points for inequality
     */
    bool operator!= (const Point& p) const
    {
        return (this->x_ != p.x_) || (this->y_ != p.y_);
    }

    Point operator- () const {
#ifdef MXGUI_PEDANTIC_CHECKS
        if (x_ == std::numeric_limits<short int>::min() || y_ == std::numeric_limits<short int>::min())
            throw std::overflow_error("Point negation overflow");
#endif
        return { static_cast<short int>(-x_), static_cast<short int>(-y_) };
    }

    // Add two points, yielding a new point with the sum of the coordinates
    Point operator+ (const Point& p) const {
#ifdef MXGUI_PEDANTIC_CHECKS
        if (p.x_ > 0 && x_ > std::numeric_limits<short int>::max() - p.x_)
            throw std::overflow_error("Point addition overflow");
        if (p.x_ < 0 && x_ < std::numeric_limits<short int>::min() - p.x_)
            throw std::underflow_error("Point addition underflow");
        if (p.y_ > 0 && y_ > std::numeric_limits<short int>::max() - p.y_)
            throw std::overflow_error("Point addition overflow");
        if (p.y_ < 0 && y_ < std::numeric_limits<short int>::min() - p.y_)
            throw std::underflow_error("Point addition underflow");
#endif
        return { static_cast<short int>(x_ + p.x_), static_cast<short int>(y_ + p.y_) };
    }

    /**
    * \param b upper left corner of test area
    * \param c lower right corner of test ares
    * \return true if this is within the area identified by b and c
    */
    [[nodiscard]] bool within(const Point& b, const Point& c) const {
        return x_ >= b.x_ && y_ >= b.y_ && x_ < c.x_ && y_ < c.y_;
    }

    // debug print for std::cout <<
    friend std::ostream& operator<<(std::ostream& os, const Point& p) {
        return os << "(" << p.x_ << "," << p.y_ << ")";
    }

    //Uses default copy constructor and operator=
private:
    short int x_,y_;
};

/**
 * \param a point to test
 * \param b upper left corner of test area
 * \param c lower right corner of test ares
 * \return true if point a is within the area identified by b and c
 */
inline bool within(const Point& a, const Point& b, const Point& c)
{
    return a.x()>=b.x() && a.y()>=b.y() && a.x()<c.x() && a.y()<c.y();
}

} // namespace mxgui
