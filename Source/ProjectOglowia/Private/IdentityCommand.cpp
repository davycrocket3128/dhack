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

#include "IdentityCommand.h"
#include "UserContext.h"
#include "ProceduralGenerationEngine.h"

AIdentityCommand::AIdentityCommand()
{
    PrimaryActorTick.bCanEverTick = true;
}

const FText AIdentityCommand::MakeAliasFrom(FString InName)
{
    FString Ret;

    for(int i = 0; i < InName.Len(); i++)
    {
        TCHAR c = InName[i];

        if(FChar::IsWhitespace(c) || FChar::IsUnderscore(c))
        {
            if(Ret.Len() > 0 && FChar::IsUnderscore(Ret[Ret.Len() - 1]))
                continue;
            
            Ret += "_";
            continue;
        }
        
        Ret += FString::Chr(c);
    }

    return FText::FromString(Ret);
}

void AIdentityCommand::Tick(float DeltaSeconds)
{
    if(!this->HasName)
    {
        FString name = "";
        if(this->GetConsole()->GetLine(name))
        {
            if(name.TrimStartAndEnd().IsEmpty())
            {
                this->GetConsole()->WriteLine(NSLOCTEXT("IdentityCommand", "NameError_Blank", "Your name must not be blank."));
            }
            else
            {
                HasName = true;
                this->IdentityName = name;
                this->GetConsole()->WriteLine(FText::GetEmpty());
                this->GetConsole()->WriteLine(NSLOCTEXT("IdentityCommand", "AliasInfo1", "You can now choose an 'Alias' to use as the username portion of your Email Address."));
                this->GetConsole()->WriteLine(FText::GetEmpty());
                this->GetConsole()->WriteLine(FText::Format(NSLOCTEXT("IdentityCommand", "AliasInfo2", "This can either be based on your Full Name (i.e: {0}@example.com) or a custom name you provide."), this->MakeAliasFrom(this->IdentityName)));
                this->GetConsole()->WriteLine(FText::GetEmpty());
                this->GetConsole()->Write(NSLOCTEXT("IdentityCommand", "UseNamePrompt", "Use your full name as your Alias? [Y/n]: "));
                return;
            }
            this->GetConsole()->Write(NSLOCTEXT("IdentityCommand", "NamePrompt", "Enter your full name: "));
        }
    }

    if(!this->UseNameAsAlias)
    {
        FString answer;
        if(this->GetConsole()->GetLine(answer))
        {
            if(answer.TrimStartAndEnd().IsEmpty())
            {
                answer = "y";
            }

            answer = answer.ToLower().TrimStartAndEnd();
            if(answer == "y")
            {
                this->AliasName = this->MakeAliasFrom(this->IdentityName).ToString();
                this->UseNameAsAlias = true;
                this->HasAlias = true;
                this->GetConsole()->WriteLine(FText::GetEmpty());
                this->GetConsole()->WriteLine(FText::Format(NSLOCTEXT("IdentityCommand", "ConfirmAliasInfo", "Using {0} as your Alias."), FText::FromString(this->AliasName)));
                this->GetConsole()->WriteLine(FText::GetEmpty());
                this->GetConsole()->Write(NSLOCTEXT("IdentityCommand", "AliasConfirmPrompt", "Is this correct? [Y/n]: "));
                return;
            }
            else if(answer == "n") {
                this->UseNameAsAlias = true;
                this->GetConsole()->WriteLine(FText::GetEmpty());
                this->GetConsole()->Write(NSLOCTEXT("IdentityCommand", "AliasPrompt", "Enter your Alias: "));
                return;
            }
            else {
                this->GetConsole()->WriteLine(NSLOCTEXT("IdentityCommand", "AnswerError", "Answer must be either 'y' or 'n'."));
            }

            this->GetConsole()->Write(NSLOCTEXT("IdentityCommand", "UseNamePrompt", "Use your full name as your Alias? [Y/n]: "));
            return;
        }
    }

    if(!this->HasAlias)
    {
        FString name = "";
        if(this->GetConsole()->GetLine(name))
        {
            if(name.TrimStartAndEnd().IsEmpty())
            {
                this->GetConsole()->WriteLine(NSLOCTEXT("IdentityCommand", "AliasError_Blank", "Your alias must not be blank."));
            }
            else
            {
                HasAlias = true;
                this->AliasName = name;
                this->GetConsole()->WriteLine(FText::GetEmpty());
                this->GetConsole()->WriteLine(FText::Format(NSLOCTEXT("IdentityCommand", "ConfirmAliasInfo", "Using {0} as your Alias."), FText::FromString(this->AliasName)));
                this->GetConsole()->WriteLine(FText::GetEmpty());
                this->GetConsole()->Write(NSLOCTEXT("IdentityCommand", "AliasConfirmPrompt", "Is this correct? [Y/n]: "));
                return;
            }
            this->GetConsole()->Write(NSLOCTEXT("IdentityCommand", "AliasPrompt", "Enter your Alias: "));
        }

    }

    if(!AliasConfirmed)
    {
        FString answer;
        if(this->GetConsole()->GetLine(answer))
        {
            if(answer.TrimStartAndEnd().IsEmpty())
            {
                answer = "y";
            }
            
            answer = answer.ToLower().TrimStartAndEnd();
            if(answer == "y")
            {
                FPeacenetIdentity& identity = this->GetUserContext()->GetPeacenet()->GetNewIdentity();
                identity.CharacterName = this->IdentityName;
                identity.CharacterType = (this->GetUserContext()->GetComputer().ID == this->GetUserContext()->GetPeacenet()->GetPlayerComputer().ID) ? EIdentityType::Player : EIdentityType::NonPlayer;
                identity.Skill = 0;
                identity.Reputation = 0.f;
                identity.PreferredAlias = this->AliasName;
                identity.ComputerID = this->GetUserContext()->GetComputer().ID;
                identity.EmailAddress = identity.PreferredAlias + "@" + this->GetUserContext()->GetPeacenet()->GetProcgen()->ChooseEmailDomain();
                this->GetUserContext()->GetComputer().SystemIdentity = identity.ID;

                this->AliasConfirmed = true;
                this->Complete();
                return;
            }
            else if(answer == "n") {
                this->HasAlias = false;
                this->GetConsole()->WriteLine(FText::GetEmpty());
                this->GetConsole()->Write(NSLOCTEXT("IdentityCommand", "AliasPrompt", "Enter your Alias: "));
                return;
            }
            else {
                this->GetConsole()->WriteLine(NSLOCTEXT("IdentityCommand", "AnswerError", "Answer must be either 'y' or 'n'."));
                this->GetConsole()->WriteLine(FText::GetEmpty());
                this->GetConsole()->WriteLine(FText::Format(NSLOCTEXT("IdentityCommand", "ConfirmAliasInfo", "Using {0} as your Alias."), FText::FromString(this->AliasName)));
                this->GetConsole()->WriteLine(FText::GetEmpty());
            }

            this->GetConsole()->Write(NSLOCTEXT("IdentityCommand", "AliasConfirmPrompt", "Is this correct? [Y/n]: "));
            return;
        }

    }
}

void AIdentityCommand::NativeRunCommand(UConsoleContext* InConsole, TArray<FString> InArguments)
{
    if(InArguments.Contains("-c"))
    {
        if(InConsole->GetUserContext()->HasIdentity())
        {
            InConsole->WriteLine(NSLOCTEXT("IdentityCommand", "AlreadyHasIdentity", "Error: You already have a Peacenet Identity.  You cannot create a new one on this system."));
            this->Complete();
        }
        else {
            InConsole->WriteLine(NSLOCTEXT("IdentityCommand", "BeginningIdentityCreation", "Beginning Peacenet Identity creation..."));
            InConsole->WriteLine(FText::GetEmpty());
            InConsole->WriteLine(NSLOCTEXT("IdentityCommand", "NameInfo", "Please enter your full name.  This name is what other people in The Peacenet will call you when talking to you.  If you wish, you may enter a username or alias instead."));
            InConsole->WriteLine(FText::GetEmpty());
            InConsole->Write(NSLOCTEXT("IdentityCommand", "NamePrompt", "Enter your full name: "));
        }
    }
    else {
        if(InConsole->GetUserContext()->HasIdentity())
        {
            InConsole->WriteLine(NSLOCTEXT("IdentityCommand", "CurrentIdentity", "Current Peacenet Identity"));
            InConsole->WriteLine(NSLOCTEXT("IdentityCommand", "CurrentIdentityDashes", "========================="));
            InConsole->WriteLine(FText::GetEmpty());
            InConsole->WriteLine(FText::Format(NSLOCTEXT("IdentityCommand", "CurrentName", "Name: {0}"), FText::FromString(this->GetUserContext()->GetCharacterName())));
            InConsole->WriteLine(FText::Format(NSLOCTEXT("IdentityCommand", "CurrentEmail", "Email: {0}"), FText::FromString(this->GetUserContext()->GetEmailAddress())));

            InConsole->WriteLine(FText::GetEmpty());
            InConsole->WriteLine(FText::Format(NSLOCTEXT("IdentityCommand", "SkillCounter", "You have {0} Skill points."), FText::AsNumber(this->GetUserContext()->GetPeacenetIdentity().Skill)));
            InConsole->WriteLine(FText::GetEmpty());
            
            this->Complete();
        }
        else {
            InConsole->WriteLine(FText::Format(NSLOCTEXT("IdentityCommand", "MustCreateIdentity", "Error: You do not have a Peacenet Identity yet.  Try creating one by running '{0} -c'."), FText::FromString(this->CommandName)));
            this->Complete();
        }
    }
}