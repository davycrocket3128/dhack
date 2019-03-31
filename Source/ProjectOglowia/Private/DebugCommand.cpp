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

#include "DebugCommand.h"
#include "UUserContext.h"
#include "PeacenetWorldStateActor.h"
#include "CString.h"
#include "MissionAsset.h"
#include "UPeacenetSaveGame.h"

void ADebugCommand::NativeRunCommand(UConsoleContext* InConsole, TArray<FString> InArguments)
{
    if(!InArguments.Num())
    {
        InConsole->WriteLine("usage: " + this->CommandName + " <action> [id]");
    }
    else
    {
        FString action = InArguments[0].ToLower();
        FString id = "";
        if(InArguments.Num()>1)
            id = InArguments[1];
            
        if(action == "characters")
        {
            InConsole->WriteLine("id\t\tname");
            InConsole->WriteLine("--\t\t-------\n");
            for(auto& character : InConsole->GetUserContext()->GetPeacenet()->SaveGame->Characters)
            {
                if(id.Len())
                    if(!character.CharacterName.ToLower().Contains(id)) continue;
                InConsole->WriteLine(FString::FromInt(character.ID) + "\t" + character.CharacterName);
            }
        }
        else if (action == "character")
        {
            int cid = FCString::Atoi(*id);

            FPeacenetIdentity Identity;
            int Index;
            bool result = InConsole->GetUserContext()->GetPeacenet()->SaveGame->GetCharacterByID(cid, Identity, Index);

            if(result)
            {
                InConsole->WriteLine("Index: " + FString::FromInt(Index));
                InConsole->WriteLine("Name: " + Identity.CharacterName);
                InConsole->WriteLine("Alias: " + Identity.PreferredAlias);
                InConsole->WriteLine("Type: " + FString::FromInt((int)Identity.CharacterType));
                InConsole->WriteLine("Email: " + Identity.EmailAddress);
                InConsole->WriteLine("Skill: " + FString::FromInt(Identity.Skill));
                InConsole->WriteLine("Comp. ID: " + FString::FromInt(Identity.ComputerID));
            }
            else
            {
                InConsole->WriteLine("No entity with that ID was found.");
            }
        }
        else if (action == "computer")
        {
            int cid = FCString::Atoi(*id);

            FComputer Computer;
            int Index;
            bool result = InConsole->GetUserContext()->GetPeacenet()->SaveGame->GetComputerByID(cid, Computer, Index);

            if(result)
            {
                InConsole->WriteLine("Index: " + FString::FromInt(Index));
                InConsole->WriteLine("Owner Type: " + FString::FromInt((int)Computer.ComputerType));
                InConsole->WriteLine("Type: " + FString::FromInt((int)Computer.ComputerType));
                InConsole->WriteLine("Users: ");
                for(auto& User : Computer.Users)
                {
                    InConsole->WriteLine(" - " + FString::FromInt(User.ID) + ": " + User.Username);
                }
                InConsole->WriteLine("Folder records: " + FString::FromInt(Computer.Filesystem.Num()));
                InConsole->WriteLine("File records: " + FString::FromInt(Computer.FileRecords.Num()));
                InConsole->WriteLine("Non-asset (pure text) file records: " + FString::FromInt(Computer.TextFiles.Num()));
            }
            else
            {
                InConsole->WriteLine("No entity with that ID was found.");
            }
        }
        else if (action == "ips")
        {
            InConsole->WriteLine("ip_addr\t\tcomputer");
            InConsole->WriteLine("-------\t\t---------\n");
            
            TArray<FString> IPs;
            InConsole->GetUserContext()->GetPeacenet()->SaveGame->ComputerIPMap.GetKeys(IPs);
            for(auto& IP : IPs)
            {
                InConsole->WriteLine(IP + "\t\t" + FString::FromInt(InConsole->GetUserContext()->GetPeacenet()->SaveGame->ComputerIPMap[IP]));
            }            
        }
        else if(action == "missions")
        {
            for(auto Mission: InConsole->GetUserContext()->GetPeacenet()->Missions)
            {
                InConsole->WriteLine(Mission->GetName());
            }
        }
        else if(action == "unlock_mission")
        {
            for(auto Mission: InConsole->GetUserContext()->GetPeacenet()->Missions)
            {
                if(Mission->GetName() == id)
                {
                    InConsole->GetUserContext()->GetPeacenet()->SendMissionMail(Mission);
                }
            }
        }
        else if(action == "possess_comp")
        {
            int cid = FCString::Atoi(*id);

            FComputer Computer;
            int Index;
            bool result = InConsole->GetUserContext()->GetPeacenet()->SaveGame->GetComputerByID(cid, Computer, Index);

            if(result)
            {
                InConsole->GetUserContext()->GetOwningSystem()->Setup(cid,InConsole->GetUserContext()->GetOwningSystem()->GetCharacter().ID,  InConsole->GetUserContext()->GetPeacenet());
                bool HasRoot = false;
                for(auto& User : Computer.Users)
                {
                    if(User.ID == 0)
                    {
                        HasRoot = true;
                    }
                }

                if(!HasRoot)
                {
                    InConsole->WriteLine("WARNING: computer doesn't have a peacegate user, creating new root user. may fuck up the game if michael hasn't had caffeine.");
                    FUser Root;
                    Root.ID = 0;
                    Root.Username = "root";
                    Root.Domain = EUserDomain::Administrator;
                    InConsole->GetUserContext()->GetOwningSystem()->GetComputer().Users.Add(Root);
                }
                InConsole->GetUserContext()->Setup(InConsole->GetUserContext()->GetOwningSystem(), 0);
            }
            else
            {
                InConsole->WriteLine("Entity not found.");
            }
        }
        else if(action == "comps")
        {
            for(auto& Computer : InConsole->GetUserContext()->GetPeacenet()->SaveGame->Computers)
            {
                InConsole->WriteLine(FString::FromInt(Computer.ID));
            }
        }
        else if (action == "doms")
        {
            // No, I don't mean the kind of dom that likes bondage.
            // I mean domain names.  It's called an abbreviation,
            // you dirty-minded fucktard.  Get your mind out of the gutter
            // or quit reading my code.
            InConsole->WriteLine("domain\t\tip");
            InConsole->WriteLine("-------\t\t---------\n");
            
            TArray<FString> Domains;
            InConsole->GetUserContext()->GetPeacenet()->SaveGame->DomainNameMap.GetKeys(Domains);
            for(auto& Domain : Domains)
            {
                InConsole->WriteLine(Domain + "\t\t" + InConsole->GetUserContext()->GetPeacenet()->SaveGame->DomainNameMap[Domain]);
            }            

        }
    }
    this->Complete();
}