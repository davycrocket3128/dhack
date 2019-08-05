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

#include "TutorialPromptState.h"
#include "PeacenetWorldStateActor.h"

FText UTutorialPromptState::GetTutorialTitle()
{
    return this->PromptTitle;
}

FText UTutorialPromptState::GetTutorialText()
{
    return this->PromptText;
}

void UTutorialPromptState::ActivatePrompt(const FText& InTitle, const FText& InText)
{
    this->PromptTitle = InTitle;
    this->PromptText = InText;
    this->PromptActive = true;

    this->TutorialActivated.Broadcast(InTitle, InText, this);
}

void UTutorialPromptState::DismissPrompt()
{
    this->PromptText = FText::GetEmpty();
    this->PromptTitle = FText::GetEmpty();
    this->PromptActive = false;
}

bool UTutorialPromptState::IsPromptActive()
{
    return this->PromptActive;
}
