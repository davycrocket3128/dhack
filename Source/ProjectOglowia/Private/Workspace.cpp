/********************************************************************************
 * The Peacenet - bit::phoenix("software");
 * 
 * MIT License
 *
 * Copyright (c) 2018-2019 Michael VanOverbeek, Declan Hoare, Ian Clary, 
 * Trey Smith, Richard Moch, Victor Tran and Warren Harris
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 * 
 * Contributors:
 *  - Michael VanOverbeek <alkaline@bitphoenixsoftware.com>
 *
 ********************************************************************************/

#include "Workspace.h"
#include "Window.h"

bool UWorkspace::AreAnyWindowsVisible()
{
    for(auto window : this->GetWindows())
    {
        if(window->IsVisible())
            return true;
    }
    return false;
}

void UWorkspace::ShowDesktop()
{
    if(this->AreAnyWindowsVisible())
    {
        // Clear existing list of invisible windows.
        this->InvisibleWindows.Empty();

        // Add all visible windows to invisible list, and hide all windows.
        for(auto window : this->GetWindows())
        {
            if(window->IsVisible())
            {
                this->InvisibleWindows.Add(window);
                window->SetVisibility(ESlateVisibility::Hidden);
            }
        }
    }
    else
    {
        // While the invisible window list is not empty, keep
        // restoring each window and removing it from the list.
        while(this->InvisibleWindows.Num())
        {
            this->InvisibleWindows[0]->SetVisibility(ESlateVisibility::Visible);
            this->InvisibleWindows.RemoveAt(0);
        }
    }
}