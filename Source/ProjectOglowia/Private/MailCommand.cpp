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

#include "MailCommand.h"
#include "DocoptForUnrealBPLibrary.h"
#include "DocoptForUnreal.h"
#include "UserContext.h"
#include "SystemContext.h"

void AMailCommand::NativeRunCommand(UConsoleContext* InConsole, TArray<FString> InArguments) {
    UMailProvider* MailProvider = this->GetUserContext()->GetMailProvider();

    if(this->ArgumentMap["view"]->AsBoolean()) {
        int id = this->ArgumentMap["<id>"]->AsNumber();

        UMailMessage* Message = MailProvider->GetMessageByID(id);
        if(Message) {
            InConsole->WriteLine(FText::GetEmpty());
            InConsole->SetBold(true);
            InConsole->SetUnderline(true);
            InConsole->SetForegroundColor(EConsoleColor::Green);
            InConsole->WriteLine(Message->GetSubject());
            InConsole->SetUnderline(false);
            InConsole->WriteLine(FText::GetEmpty());
            InConsole->Write(NSLOCTEXT("MailCommand", "MailParticipants", "Participants: "));
            InConsole->ResetFormatting();
            InConsole->WriteLine(Message->GetParticipants());
            InConsole->WriteLine(FText::GetEmpty());
            InConsole->WriteLine(Message->GetMessageText());
            InConsole->WriteLine(FText::GetEmpty());
            if(Message->HasMission()) {
                InConsole->WriteLine(FText::GetEmpty());
                InConsole->SetBold(true);
                InConsole->SetForegroundColor(EConsoleColor::Magenta);
                InConsole->Write(NSLOCTEXT("MailCommand", "MissionAcquisition", "Mission acquisition: "));
                InConsole->ResetFormatting();
                InConsole->WriteLine(Message->GetMissionAcquisition());
                InConsole->WriteLine(FText::GetEmpty());
                InConsole->SetItalic(true);
                InConsole->WriteLine(FText::Format(NSLOCTEXT("MailCommand", "StartMissionPrompt", "Use 'mail start {0}' to start this mission."), FText::AsNumber(id)));
                InConsole->ResetFormatting();
                InConsole->WriteLine(FText::GetEmpty());
            }
            this->Complete();
            return;
        }

        InConsole->SetForegroundColor(EConsoleColor::Yellow);
        InConsole->WriteLine(NSLOCTEXT("MailCommand", "MessageNotFound", "No message with the specified ID was found."));
        InConsole->ResetFormatting();
    } else if(this->ArgumentMap["inbox"]->AsBoolean()) {
        InConsole->SetBold(true);
        InConsole->SetForegroundColor(EConsoleColor::Yellow);
        InConsole->WriteLine(FText::Format(NSLOCTEXT("MailCommand", "ViewingInbox", "Viewing inbox of {0}:"), FText::FromString(this->GetUserContext()->GetEmailAddress())));
        InConsole->WriteLine(FText::GetEmpty());
        InConsole->ResetFormatting();

        this->WriteMessageList(InConsole, MailProvider->GetMessagesInInbox());
    } else if(this->ArgumentMap["outbox"]->AsBoolean()) {
        InConsole->SetBold(true);
        InConsole->SetForegroundColor(EConsoleColor::Yellow);
        InConsole->WriteLine(FText::Format(NSLOCTEXT("MailCommand", "ViewingOutbox", "Viewing outbox of {0}:"), FText::FromString(this->GetUserContext()->GetEmailAddress())));
        InConsole->WriteLine(FText::GetEmpty());
        InConsole->ResetFormatting();

        InConsole->WriteLine(NSLOCTEXT("General", "NYI", "Not yet implemented."));
    } else if(this->ArgumentMap["missions"]->AsBoolean()) {
        InConsole->SetBold(true);
        InConsole->SetForegroundColor(EConsoleColor::Yellow);
        InConsole->WriteLine(FText::Format(NSLOCTEXT("MailCommand", "ViewingMissions", "Viewing available missions in mailbox of {0}:"), FText::FromString(this->GetUserContext()->GetEmailAddress())));
        InConsole->WriteLine(FText::GetEmpty());
        InConsole->ResetFormatting();

        this->WriteMessageList(InConsole, MailProvider->GetMessagesWithMissions());
    } else if(this->ArgumentMap["start"]->AsBoolean()) {
        int id = this->ArgumentMap["<id>"]->AsNumber();

        auto Message = MailProvider->GetMessageByID(id);

        if(Message) {
            if(Message->HasMission()) {
                if(Message->CanPlayMission()) {
                    InConsole->WriteLine(NSLOCTEXT("MailCommand", "StartingMission", "Starting mission..."));
                    Message->BeginMission();
                    this->Complete();
                    return;
                }

                InConsole->SetForegroundColor(EConsoleColor::Yellow);
                InConsole->WriteLine(NSLOCTEXT("MailCommand", "MissionNotPlayable", "You cannot start this mission right now - either it has already been completed or you are in another mission."));
                InConsole->ResetFormatting();

                this->Complete();
                return;
            }

            InConsole->SetForegroundColor(EConsoleColor::Yellow);
            InConsole->WriteLine(NSLOCTEXT("MailCommand", "MessageNotMission", "The specified mail message does not contain a mission, therefore it cannot be started."));
            InConsole->ResetFormatting();

            this->Complete();
            return;
        }

        InConsole->SetForegroundColor(EConsoleColor::Yellow);
        InConsole->WriteLine(NSLOCTEXT("MailCommand", "MessageNotFound", "No message with the specified ID was found."));
        InConsole->ResetFormatting();
    }
    this->Complete();
}

void AMailCommand::WriteMessageList(UConsoleContext* InConsole, TArray<UMailMessage*> InMessages) {
    TArray<UMailMessage*> Messages;
    
    FText ParticipantsHead = NSLOCTEXT("MailCommand", "ParticipantsHead", "Participants");
    FText SubjectHead = NSLOCTEXT("MailCommand", "SubjectHead", "Subject");
    FText IdHead = NSLOCTEXT("MailCommand", "IdHead", "#");
    
    int LongestId = IdHead.ToString().Len();
    int LongestSubject = SubjectHead.ToString().Len();
    int LongestParticipants = ParticipantsHead.ToString().Len();

    for(auto Message : InMessages) {
        Messages.Add(Message);
        int IdLen = Message->GetMessageId().Len();
        int SubjectLen = Message->GetSubject().ToString().Len();
        int ParticipantsLen = Message->GetParticipants().ToString().Len();
        if(IdLen > LongestId) {
            LongestId = IdLen;
        }
        if(SubjectLen > LongestSubject) {
            LongestSubject = SubjectLen;
        }
        if(ParticipantsLen > LongestParticipants) {
            LongestParticipants = ParticipantsLen;
        } 
    }

    InConsole->SetBold(true);

    InConsole->Write(IdHead);
    for(int i = 0; i <= LongestId - IdHead.ToString().Len(); i++) {
        InConsole->Write(FText::FromString(" "));
    }
    InConsole->Write(FText::FromString(" | "));
    InConsole->Write(SubjectHead);
    for(int i = 0; i <= LongestSubject - SubjectHead.ToString().Len(); i++) {
        InConsole->Write(FText::FromString(" "));
    }
    InConsole->Write(FText::FromString(" | "));
    InConsole->WriteLine(ParticipantsHead);
    InConsole->WriteLine(FText::GetEmpty());

    InConsole->ResetFormatting();

    for(auto Message : Messages) {
        InConsole->Write(FText::FromString(Message->GetMessageId()));
        for(int i = 0; i <= LongestId - Message->GetMessageId().Len(); i++) {
            InConsole->Write(FText::FromString(" "));
        }
        InConsole->Write(FText::FromString(" | "));
        InConsole->Write(Message->GetSubject());
        for(int i = 0; i <= LongestSubject - Message->GetSubject().ToString().Len(); i++) {
            InConsole->Write(FText::FromString(" "));
        }
        InConsole->Write(FText::FromString(" | "));
        InConsole->WriteLine(Message->GetParticipants());
    }

    if(!Messages.Num()) {
        InConsole->SetForegroundColor(EConsoleColor::Magenta);
        InConsole->WriteLine(NSLOCTEXT("MailCommand", "EmptyMailbox", "This mailbox is empty, no messages can be displayed."));
        InConsole->ResetFormatting();
    }

    InConsole->WriteLine(FText::GetEmpty());
    
    InConsole->SetItalic(true);
    InConsole->WriteLine(NSLOCTEXT("MailCommand", "ViewPrompt", "Use 'mail view <#>' to view a message from the list above."));
    InConsole->SetItalic(false);

    InConsole->WriteLine(FText::GetEmpty());
}