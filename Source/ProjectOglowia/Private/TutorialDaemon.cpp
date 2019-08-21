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

#include "TutorialDaemon.h"
#include "SystemContext.h"
#include "DesktopWidget.h"
#include "TutorialPromptState.h"
#include "PeacenetWorldStateActor.h"

void UTutorialDaemon::NativeStart() {
    // Bind to the tutorial prompt's activate event.
	TScriptDelegate<> TutorialEvent;
	TutorialEvent.BindUFunction(this, "UpdateTutorial");

	if(!this->GetTutorialPrompt()->TutorialActivated.Contains(TutorialEvent)) {
		this->GetTutorialPrompt()->TutorialActivated.Add(TutorialEvent);
    }

    // Dismiss the current prompt so the tutorial backend isn't hung up from us being asleep and unable
    // to dismiss prompts.
    if(this->GetTutorialPrompt()->IsPromptActive()) {
        this->GetTutorialPrompt()->DismissPrompt();
    }

    // Prevent any further prompts that were queued for us while we were asleep from showing.
    this->GetTutorialPrompt()->ClearFuture();
}

void UTutorialDaemon::NativeStop() {
    // Dismiss any active tutorial prompts.
    if(this->GetTutorialPrompt()->IsPromptActive()) {
        this->GetTutorialPrompt()->DismissPrompt();
    }

    // Stop responding to tutorial events.
	TScriptDelegate<> TutorialEvent;
	TutorialEvent.BindUFunction(this, "UpdateTutorial");

	if(this->GetTutorialPrompt()->TutorialActivated.Contains(TutorialEvent)) {
		this->GetTutorialPrompt()->TutorialActivated.Remove(TutorialEvent);
    }
}

void UTutorialDaemon::NativeTick(float DeltaSeconds){
    // Is there a tutorial ready?
    if(this->NewTutorialReady) {
        // Is the desktop in a state where tutorials can be activated?
        if(this->GetSystemContext()->GetDesktop() && this->GetSystemContext()->GetDesktop()->IsSessionActive()) {
            // Broadcast the event!
            FText Title = this->GetTutorialPrompt()->GetTutorialTitle();
            FText Message = this->GetTutorialPrompt()->GetTutorialText();
            this->GetSystemContext()->GetDesktop()->UpdateTutorial(Title, Message, this->GetTutorialPrompt());
            
            // Tutorial is no longer ready.
            this->NewTutorialReady = false;
        }
    }
}

UTutorialPromptState* UTutorialDaemon::GetTutorialPrompt() {
    return this->GetPeacenet()->GetTutorialState();
}

void UTutorialDaemon::UpdateTutorial(const FText& InTitle, const FText& InNewText, UTutorialPromptState* InTutorialPromptState) {
    // If the desktop is there, and the desktop has a session, we'll forward the tutorial right away.
    //
    // If not then we'll simply mark that we have a new tutorial to show, and try to forward it every tick
    // until we succeed.
    if(this->GetSystemContext()->GetDesktop() && this->GetSystemContext()->GetDesktop()->IsSessionActive()) {
        this->GetSystemContext()->GetDesktop()->UpdateTutorial(InTitle, InNewText, InTutorialPromptState);
    } else {
        this->NewTutorialReady =true;
    }
}