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

#include "SendEmailAction.h"
#include "UserContext.h"
#include "SystemContext.h"
#include "MissionActor.h"
#include "MissionAsset.h"
#include "Email.h"
#include "PeacenetWorldStateActor.h"
#include "PeacenetSaveGame.h"

void USendEmailAction::NativeMissionCompleted()
{
    // Get the subject line of the email.  If empty, it is "Re: <mission name>".
    FText realSubject = this->Subject.IsEmpty() ? FText::Format(NSLOCTEXT("Email", "ReplySubject", "Re: {0}"), this->GetMission()->GetMissionAsset()->Name) : this->Subject;

    // Get the real sender of the email.  If none is provided then we'll use the mission's giver.
    UStoryCharacter* realSender = this->Sender ? this->Sender : this->GetMission()->GetMissionAsset()->Assigner;

    // Make sure the sender definitely isn't null.
    check(realSender);

    // Get the player's mail provider.
    UMailProvider* PlayerMail = this->GetPlayerUser()->GetOwningSystem()->GetMailProvider();

    // Get the save file.
    UPeacenetSaveGame* SaveGame = this->GetPeacenet()->SaveGame;

    // Create a new email structure.
    FEmail mail;
    mail.ID = SaveGame->EmailMessages.Num();
    mail.InReplyTo = -1;
	SaveGame->GetStoryCharacterID(realSender, mail.FromEntity);
    mail.ToEntities.Add(this->GetPlayerUser()->GetOwningSystem()->GetCharacter().ID);
	mail.Subject = realSubject.ToString();
	mail.MessageBody = this->MessageBody.ToString();

    // Store it in the save.
    SaveGame->EmailMessages.Add(mail);
	
	// Notify the player of the new email message.
	PlayerMail->NotifyReceivedMessage(mail.ID);
}