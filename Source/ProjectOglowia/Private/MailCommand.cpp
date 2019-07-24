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

void AMailCommand::NativeRunCommand(UConsoleContext* InConsole, TArray<FString> InArguments)
{
    UMailProvider* MailProvider = this->GetUserContext()->GetMailProvider();

    if(this->ArgumentMap["view"]->AsBoolean())
    {
        int id = this->ArgumentMap["<id>"]->AsNumber();

        UMailMessage* Message = MailProvider->GetMessageByID(id);
        if(Message)
        {
            InConsole->WriteLine(FText::GetEmpty());
            InConsole->WriteLine(Message->GetSubject());
            InConsole->WriteLine(FText::GetEmpty());
            InConsole->Write(NSLOCTEXT("MailCommand", "MailParticipants", "Participants: "));
            InConsole->WriteLine(Message->GetParticipants());
            InConsole->WriteLine(FText::GetEmpty());
            InConsole->WriteLine(Message->GetMessageText());
            InConsole->WriteLine(FText::GetEmpty());
            if(Message->HasMission())
            {
                InConsole->WriteLine(FText::GetEmpty());
                InConsole->Write(NSLOCTEXT("MailCommand", "MissionAcquisition", "Mission acquisition: "));
                InConsole->WriteLine(Message->GetMissionAcquisition());
                InConsole->WriteLine(FText::GetEmpty());
                InConsole->WriteLine(FText::Format(NSLOCTEXT("MailCommand", "StartMissionPrompt", "Use 'mail start {0}' to start this mission."), FText::AsNumber(id)));
                InConsole->WriteLine(FText::GetEmpty());
            }
            this->Complete();
            return;
        }

        InConsole->WriteLine(NSLOCTEXT("MailCommand", "MessageNotFound", "No message with the specified ID was found."));
    }

    this->Complete();
}