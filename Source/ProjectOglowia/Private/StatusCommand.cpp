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

#include "StatusCommand.h"
#include "UserContext.h"

void AStatusCommand::NativeRunCommand(UConsoleContext* InConsole, TArray<FString> InArguments)
{
    InConsole->WriteLine(NSLOCTEXT("StatusCommand", "StatusTitle", "Peacegate OS System Status"));
    InConsole->WriteLine(FText::GetEmpty());

    InConsole->WriteLine(NSLOCTEXT("StatusCommand", "System", "SYSTEM:"));
    InConsole->WriteLine(FText::Format(NSLOCTEXT("StatusCommand", "Hostname", "Hostname: {0}"), FText::FromString(this->GetUserContext()->GetHostname())));
    InConsole->WriteLine(FText::Format(NSLOCTEXT("StatusCommand", "IPAddress", "IP address: {0}"), FText::FromString(this->GetUserContext()->GetIPAddress())));
    InConsole->WriteLine(FText::Format(NSLOCTEXT("StatusCommand", "Username", "Current user: {0}@{1}"), FText::FromString(this->GetUserContext()->GetUsername()), FText::FromString(this->GetUserContext()->GetHostname())));

    InConsole->WriteLine(FText::GetEmpty());

    InConsole->WriteLine(NSLOCTEXT("StatusCommand", "Identity", "IDENTITY:"));
    
    if(this->GetUserContext()->HasIdentity())
    {
        InConsole->WriteLine(FText::Format(NSLOCTEXT("StatusCommand", "Fullname", "Full Name: {0}"), FText::FromString(this->GetUserContext()->GetCharacterName())));
        InConsole->WriteLine(FText::Format(NSLOCTEXT("StatusCommand", "Alias", "Alias: {0}"), FText::FromString(this->GetUserContext()->GetPeacenetIdentity().PreferredAlias)));
        InConsole->WriteLine(FText::Format(NSLOCTEXT("StatusCommand", "Email", "Email Address: {0}"), FText::FromString(this->GetUserContext()->GetEmailAddress())));

        InConsole->WriteLine(FText::Format(NSLOCTEXT("StatusCommand", "Skill", "Skill points: {0}"), this->GetUserContext()->GetPeacenetIdentity().Skill));

    }
    else {
        InConsole->WriteLine(NSLOCTEXT("StatusCommand", "NoIdentity", "You don't currently have a Peacenet Identity, so no information can be shown here."));
    }

    InConsole->WriteLine(FText::GetEmpty());

    this->Complete();
}