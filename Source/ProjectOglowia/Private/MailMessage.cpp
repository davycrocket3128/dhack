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

#include "MailMessage.h"
#include "MailProvider.h"
#include "PeacenetSaveGame.h"
#include "PeacenetWorldStateActor.h"

FString UMailMessage::GetMessageId()
{
    return FString::FromInt(this->MessageID);
}

bool UMailMessage::MissionIsCompleted()
{
    if(!this->HasMission()) return false;

    return this->Provider->GetPeacenet()->IsMissionCompleted(this->GetMainMessage().Mission);
}

bool UMailMessage::CanPlayMission()
{
    // Obviously we can't play the mission if it doesn't FUCKING EXIST.
    // FOR THE LOVE OF <insert person here>
    if(!this->HasMission()) return false;

    // We also can't play if the player is in a mission.
    if(this->Provider->GetPeacenet()->IsInMission()) return false;

    return !this->Provider->GetPeacenet()->IsMissionCompleted(this->GetMainMessage().Mission);
}

void UMailMessage::BeginMission()
{
    check(this->HasMission());

    this->Provider->GetPeacenet()->StartMission(this->GetMainMessage().Mission);
}

FText UMailMessage::GetParticipants()
{
    FString Ret = "You";

    int FirstOther = -1;

    if(this->GetMainMessage().FromEntity != this->Provider->GetIdentityID())
    {
        FirstOther = this->GetMainMessage().FromEntity;
    }
    else
    {
        FirstOther = this->GetMainMessage().ToEntities[0];
    }

    FPeacenetIdentity& Identity = this->Provider->GetPeacenet()->GetCharacterByID(this->GetMainMessage().FromEntity);
    Ret += ", " + Identity.CharacterName;

    TArray<int> UniqueOthers;

    if(this->GetMainMessage().FromEntity != FirstOther && this->GetMainMessage().FromEntity != this->Provider->GetIdentityID())
    {
        UniqueOthers.Add(this->GetMainMessage().FromEntity);
    }

    for(int Other: this->GetMainMessage().ToEntities)
    {
        if(Other == FirstOther) continue;
        if(Other == this->Provider->GetIdentityID()) continue;
        if(UniqueOthers.Contains(Other)) continue;

        UniqueOthers.Add(Other);
    }

    for(auto& Reply : this->GetReplies())
    {
        if(Reply.FromEntity != FirstOther && Reply.FromEntity != this->Provider->GetIdentityID() && !UniqueOthers.Contains(Reply.FromEntity))
        {
            UniqueOthers.Add(Reply.FromEntity);
        }

        for(int Other: Reply.ToEntities)
        {
            if(Other == FirstOther) continue;
            if(Other == this->Provider->GetIdentityID()) continue;
            if(UniqueOthers.Contains(Other)) continue;

            UniqueOthers.Add(Other);
        }

    }

    if(UniqueOthers.Num())
    {
        if(UniqueOthers.Num() == 1)
        {
            Ret += ", 1 other";
        }
        else
        {
            Ret += ", " + FString::FromInt(UniqueOthers.Num()) + " others";
        }
    }
    return FText::FromString(Ret);
}

FText UMailMessage::GetMissionAcquisition()
{
    if(this->HasMission())
    {
        return this->GetMainMessage().Mission->Description;
    }
    return FText::FromString("Not a mission.");
}

void UMailMessage::Setup(UMailProvider* InProvider, int InMessageID)
{
    check(InProvider);
    check(InMessageID >= 0);

    this->MessageID = InMessageID;
    this->Provider = InProvider;
}

FEmail UMailMessage::GetMainMessage()
{
    for(auto& Message : this->Provider->GetMailMessages())
    {
        if(Message.ID == this->MessageID)
            return Message;
    }
    return FEmail();
}

TArray<FEmail> UMailMessage::GetReplies()
{
    TArray<FEmail> Replies;

    for(auto& Message : this->Provider->GetMailMessages())
    {
        if(Message.InReplyTo == this->MessageID)
            Replies.Add(Message);
    }

    return Replies;
}

FText UMailMessage::GetSubject()
{
    if(this->HasMission())
    {
        return this->GetMainMessage().Mission->Name;
    }
    return FText::FromString(this->GetMainMessage().Subject);
}

bool UMailMessage::HasMission()
{
    return this->GetMainMessage().Mission;
}

FText UMailMessage::GetMessageText()
{
    if(this->HasMission())
    {
        FString Text = this->GetMainMessage().Mission->MailMessageText.ToString();
        if(Text.Contains("%agent"))
        {
            FPeacenetIdentity Agent = this->Provider->GetPeacenet()->GetCharacterByID(this->Provider->GetIdentityID());
            Text = Text.Replace(TEXT("%agent"), *Agent.CharacterName);
        }
        return FText::FromString(Text);
    }
    else
    {
        return FText::FromString(this->GetMainMessage().MessageBody);
    } 
}

bool UMailMessage::HasAttachments()
{
    return this->GetAttachmentCount();
}

int UMailMessage::GetAttachmentCount()
{
    return 0;
}