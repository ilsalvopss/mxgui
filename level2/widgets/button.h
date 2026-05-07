/***************************************************************************
 *   Copyright (C) 2024 by Aaron Tognoli                                   *
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

#include "interactable_button.h"

#ifdef MXGUI_LEVEL_2

namespace mxgui::widgets {

/**
 * A basic interactive Button.
 */
class Button : public InteractableButton, public DrawableOwner
{
public:
    /**
     * Constructor
     * The object will be immediately enqueued for redraw
     * \param owner DrawableOwner to which this object belongs
     * \param da area on screen occupied by this object
     * \param text text written in the Button
     */
    Button(BadgedRef<DrawableOwner>&& owner, const Rect& da, const std::string& text="");
    
    /**
     * Constructor
     * The object will be immediately enqueued for redraw
     * \param owner DrawableOwner to which this object belongs
     * \param p upper left point of the button
     * \param width width of the button
     * \param height height of the button
     * \param text text written in the button
     */
    Button(BadgedRef<DrawableOwner>&& owner, Point p, short width, short height, const std::string& text="");

    /**
     * Implements DrawableOwner::needsRedrawForRect.
     * Here we simply forward the invalidation to the parent DrawableOwner
     * @param r rect to redraw
     */
    void needsRedrawForRect(const Rect& r) override;

    /**
     * Implements DrawableOwner::getWindow.
     * @return window to which this button belongs
     */
    Window& getWindow() override { return owner.getWindow(); }
protected:
    /** 
     * Overridden this member function to set the colors of the button when it is pressed
    */
    virtual void buttonDown();

    /**
     * Overridden this member function to reset the colors of the button.
    */
    virtual void resetState();
    /** 
     * Overridden this member function to also set the colors of the button when it is released.
    */
    virtual void buttonUp();

    /**
     * \internal
     * Overridden this member function to draw the object.
     * \param dc drawing context used to draw the object
     */
    void onDraw(DrawingContextProxy& dc) override;
};

} //namesapce mxgui

#endif //MXGUI_LEVEL_2
