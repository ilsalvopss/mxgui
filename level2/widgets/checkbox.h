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
 * CheckBox Button.
 */
class CheckBox : public InteractableButton, public DrawableOwner
{
public:
    /**
     * Constructor
     * The object will be immediately enqueued for redraw
     * \param owner DrawableOwner to which this object belongs
     * \param p upper left point of the CheckBox
     * \param dimension width and height of the CheckBox ( it's a square )
     * \param text label of the checkbox
     * \param checked initial state of the checkbox
     */
    CheckBox(BadgedRef<DrawableOwner> owner, Point p, short dimension=15, const std::string& text="", bool checked=false);
    
    /**
     * Returns true if the checkbox is checked
    */
    virtual bool isChecked();

    Rect getDrawArea() const override { return da; }

    /**
     * Implements DrawableOwner::getWindow.
     * @return window to which this button belongs
     */
    Window& getWindow() override { return owner.getWindow(); }
protected:
    Rect da;
    bool checked; ///< True if the checkbox is checked
    
private:
    /**
     * Implements DrawableOwner::needsRedrawForRect.
     * Here we simply forward the invalidation to the parent DrawableOwner
     * @param r rect to redraw
     */
    void needsRedrawForRect(const Rect& r) override;

    /**
     * \internal
     * Overridden this member function to draw the object.
     * \param dc drawing context used to draw the object
     */
    void onDraw(DrawingContextProxy& dc) override;

    Point labelStartingPoint; ///< Upper left point of the label
    //Private functions
    /**
     * Overridden this member function to reset the colors of the button.
    */
    void resetState() override;
    /** 
     * Overridden this member function to set the colors of the button when it is pressed
    */
    void buttonDown() override;

    /** 
     * Overridden this member function to also set the colors of the button when it is released.
    */
    void buttonUp() override;

    /**
     * Function that handles the checkbox state change
    */
    virtual void check();
};

} //namesapce mxgui

#endif //MXGUI_LEVEL_2
