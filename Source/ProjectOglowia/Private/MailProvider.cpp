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

#include "MailProvider.h"
#include "SystemContext.h"
#include "PeacenetWorldStateActor.h"
#include "PeacenetSaveGame.h"

UMailMessage* UMailProvider::GetMessageByID(int InID)
{
    for(FEmail Email : this->GetMailMessages())
    {
        if(Email.ID == InID)
        {
            UMailMessage* Message = NewObject<UMailMessage>();
            Message->Setup(this, InID);
            return Message;
        }
    }
    return nullptr;
}

TArray<UMailMessage*> UMailProvider::GetMessagesWithMissions()
{
    TArray<UMailMessage*> ret;
    for(auto message : this->GetMessagesInInbox())
    {
        if(message->HasMission() && !message->MissionIsCompleted())
        {
            ret.Add(message);
        }
    }
    return ret;
}

APeacenetWorldStateActor* UMailProvider::GetPeacenet()
{
    return this->OwningSystem->GetPeacenet();
}

int UMailProvider::GetIdentityID()
{
    return this->OwningSystem->GetCharacter().ID;
}

int UMailProvider::GetInboxCount()
{
    return this->GetInbox().Num();
}

TArray<UMailMessage*> UMailProvider::GetMessagesInInbox()
{
    TArray<UMailMessage*> Ret;
    for(auto Message : this->GetInbox())
    {
        UMailMessage* NewMessage = NewObject<UMailMessage>(this);
        NewMessage->Setup(this, Message.ID);
        Ret.Add(NewMessage);
    }
    return Ret;
}

int UMailProvider::GetOutboxCount()
{
    return this->GetOutbox().Num();
}

int UMailProvider::GetMissionsCount()
{
    int count = 0;

    TArray<UMissionAsset*> missions;

    for(auto& message : this->GetMailMessages())
    {
        if(message.ToEntities.Contains(this->GetIdentityID()) && message.Mission)
        {
            if(!missions.Contains(message.Mission))
            {
                if(!this->OwningSystem->GetPeacenet()->IsMissionCompleted(message.Mission))
                {
                    missions.Add(message.Mission);
                    count++;
                }
            }
        }
    }

    return count;
}

void UMailProvider::SendMailInternal(TArray<int> InRecipients, FString InSubject, FString InMessageText, int InReplyTo)
{
    // This is what the message's from value will be.
    int MyEntityID = this->OwningSystem->GetCharacter().ID;

    // Create the new message and set the from value.
    FEmail NewMail = this->GetPeacenet()->GetNewEmailMessage();
    NewMail.FromEntity = MyEntityID;
    
    // Set the subject and message text values.
    NewMail.Subject = InSubject;
    NewMail.MessageBody = InMessageText;

    // Check all entities in recipient list to make sure they exist in the save file.
    for(auto EntityID : InRecipients)
    {
        check(this->GetPeacenet()->IdentityExists(EntityID));
    }

    // Set the recipient list.
    NewMail.ToEntities = InRecipients;

    // If "in reply to" is not -1, check to make sure it is a valid message ID.
    if(InReplyTo != -1)
    {
        bool FoundMail = false;

        for(auto ExistingMessage : this->GetPeacenet()->GetEmailMessages())
        {
            if(ExistingMessage.ID == InReplyTo)
            {
                NewMail.InReplyTo = InReplyTo;
                FoundMail = true;
                break;
            }
        }

        check(FoundMail);
    }

    // Notify all recipients that they've received an email.
    for(auto Recipient : InRecipients)
    {
        // Does the recipient have a loaded system context?
        // If it doesn't, we don't notify the recipient because we'd need to
        // create a new system context which means less precious RAM.
        if(this->OwningSystem->GetPeacenet()->IdentityHasSystemContext(Recipient))
        {
            // Get the existing recipient system context.
            USystemContext* RecSys = this->OwningSystem->GetPeacenet()->GetSystemContext(Recipient);

            // Now we can notify its mail provider of the new message.
            RecSys->GetMailProvider()->NotifyReceivedMessage(NewMail.ID);
        }
    }
}

void UMailProvider::NotifyReceivedMessage(int InMessageID)
{
    bool Exists = false;

    for(auto& Message : this->GetMailMessages())
    {
        if(Message.ID == InMessageID)
        {
            Exists = true;
            break;
        }
    }

    check(Exists);

    UMailMessage* BlueprintMessage = NewObject<UMailMessage>(this);
    BlueprintMessage->Setup(this, InMessageID);
    this->NewMessageReceived.Broadcast(BlueprintMessage);
}

void UMailProvider::Setup(USystemContext* InOwningSystem)
{
    this->OwningSystem = InOwningSystem;
}

TArray<FEmail> UMailProvider::GetMailMessages()
{
    TArray<FEmail> MyEmails;
    for(auto Email : this->GetPeacenet()->GetEmailMessages())
    {
        if(Email.FromEntity == this->OwningSystem->GetCharacter().ID || Email.ToEntities.Contains(this->OwningSystem->GetCharacter().ID))
            MyEmails.Add(Email);
    }
    return MyEmails;
}

TArray<FEmail> UMailProvider::GetInbox()
{
    TArray<FEmail> Ret;
    for(auto Message : this->GetMailMessages())
    {
        if(Message.InReplyTo == -1 && Message.ToEntities.Contains(this->OwningSystem->GetCharacter().ID))
        {
            Ret.Add(Message);
        }
    }
    return Ret;
}

TArray<FEmail> UMailProvider::GetOutbox()
{
    TArray<FEmail> Ret;
    for(auto Message : this->GetMailMessages())
    {
        if(Message.FromEntity == this->OwningSystem->GetCharacter().ID)
            Ret.Add(Message);
    }
    return Ret;
}