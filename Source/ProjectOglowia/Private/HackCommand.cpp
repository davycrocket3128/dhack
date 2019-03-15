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

#include "HackCommand.h"
#include "EConnectionError.h"
#include "FFirewallRule.h"
#include "PayloadAsset.h"
#include "Computer.h"
#include "UProceduralGenerationEngine.h"
#include "ProtocolVersion.h"
#include "TerminalCommandParserLibrary.h"

AHackCommand::AHackCommand()
{
    // Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

void AHackCommand::HandleCommand(FString InCommandName, TArray<FString> InArguments)
{
    UConsoleContext* Console = this->GetConsole();

    if(InCommandName == "exit")
    {
        this->Complete();
        return;
    }

    if(InCommandName == "exploits")
    {
        FString SearchQuery = "";
        for(auto Arg : InArguments)
        {
            SearchQuery += Arg + " ";
        }

        SearchQuery = SearchQuery.TrimStartAndEnd();

        TArray<FString> ExploitNames;
    
        Console->WriteLine("\t&FAvailable exploits");
        Console->WriteLine("\t--------------------&7\n");

        bool FoundExploit = false;

        for(auto Exploit : Console->GetUserContext()->GetExploits())
        {
            FString ExploitName = Exploit->ID.ToString();
            if(SearchQuery.Len() == 0 || ExploitName.Contains(SearchQuery, ESearchCase::IgnoreCase))
            {
                FoundExploit = true;
                Console->WriteLine("\t" + ExploitName);
            }
        }

        if(!FoundExploit)
            Console->WriteLine("\t&4No exploits found matching that query.&7");

        Console->WriteLine("");
        return;
    }

    if(InCommandName == "payloads")
    {
        FString SearchQuery = "";
        for(auto Arg : InArguments)
        {
            SearchQuery += Arg + " ";
        }

        SearchQuery = SearchQuery.TrimStartAndEnd();

        TArray<FString> ExploitNames;
    
        Console->WriteLine("\t&FAvailable payloads");
        Console->WriteLine("\t--------------------&7\n");

        bool FoundExploit = false;

        for(auto Exploit : Console->GetUserContext()->GetPayloads())
        {
            FString ExploitName = Exploit->Name.ToString();
            if(SearchQuery.Len() == 0 || ExploitName.Contains(SearchQuery, ESearchCase::IgnoreCase))
            {
                FoundExploit = true;
                Console->WriteLine("\t" + ExploitName);
            }
        }

        if(!FoundExploit)
            Console->WriteLine("\t&4No payloads found matching that query.&7");

        Console->WriteLine("");
        return;
    }

    if(InCommandName == "use")
    {
        if(!InArguments.Num())
        {
            Console->WriteLine("&4&*error:&r too few arguments given. Do you want to use an exploit or payload?&7");
            return;
        }

        FString ItemType = InArguments[0].ToLower();
        InArguments.RemoveAt(0);

        FString ExploitName = "";
        for(FString Arg : InArguments)
        {
            ExploitName += Arg + " ";
        }
        ExploitName = ExploitName.TrimStartAndEnd();
        
        if(ItemType == "exploit")
        {
            for(auto Exploit : Console->GetUserContext()->GetExploits())
            {
                if(Exploit->ID.ToString() == ExploitName)
                {
                    Console->WriteLine("Using exploit &F" + Exploit->FullName.ToString() + "&7.");
                    this->CurrentExploit = Exploit;
                    return;
                }
            }

            Console->WriteLine("&4&*error:&r exploit doesn't exist.&7");
            return;
        }
        else if(ItemType == "payload")
        {
            for(auto Exploit : Console->GetUserContext()->GetPayloads())
            {
                if(Exploit->Name.ToString() == ExploitName)
                {
                    Console->WriteLine("Using payload &F" + ExploitName + "&7.");
                    this->CurrentPayload = Exploit;
                    return;
                }
            }

            Console->WriteLine("&4&*error:&r payload doesn't exist.&7");
            return;
        }
        else
        {
            Console->WriteLine("&4&*error:&r Invalid item type: &7" + ItemType);
            return;
        }
    }

    if(InCommandName == "scan")
    {
        Console->WriteLine("Performing Nmap scan on remote system...");
        Console->WriteLine("");
        Console->WriteLine("PORT\t\tSTATUS\tSERVICE");
        Console->WriteLine("-----\t\t-------\t--------");
        Console->WriteLine("");
        this->RemoteSystem->GetPeacenet()->GetProcgen()->GenerateFirewallRules(this->RemoteSystem->GetComputer());
        for(auto Service : this->RemoteSystem->GetComputer().FirewallRules)
        {
            Console->Write(FString::FromInt(Service.Port));
            Console->Write("\t\t");

            if(Service.IsFiltered)
            {
                Console->Write("&4filtered&7");
            }
            else
            {
                Console->Write("&3open&7");
            }
            Console->Write("\t");
            Console->WriteLine(Service.Service->Name.ToString());
        }
        return;
    }

    if(InCommandName == "attack")
    {
        if(!InArguments.Num())
        {
            Console->WriteLine("&4&*error:&r too few arguments. You must specify a port to attack.&7");
            return;
        }

        if(!this->CurrentExploit)
        {
            Console->WriteLine("&4&*error:&r You must select an exploit to use first.&7");
            return;
        }

        if(!this->CurrentPayload)
        {
            Console->WriteLine("&4&*error:&r You must select a payload to use first.&7");
            return;
        }

        FString EnteredPort = InArguments[0];
        int RealPort = FCString::Atoi(*EnteredPort);
        if(!RealPort && EnteredPort != "0")
        {
            Console->WriteLine("&4&*error:&r Port is not a number.&7");
            return;
        }

        Console->WriteLine("Finding service on &3" + EnteredHostname + "&7:&6" + EnteredPort + "&7...");

        this->HandleCommand("scan", InArguments);

        for(auto Service : this->RemoteSystem->GetComputer().FirewallRules)
        {
            if(Service.Port == RealPort)
            {
                Console->WriteLine("Service is &F" + Service.Service->Name.ToString() + "&7.");

                if(Service.IsFiltered)
                {
                    Console->WriteLine("Service is &4&*FILTERED&r&7! Can't continue with exploit.");
                    return;
                }

                Console->WriteLine("Service is &3OPEN&7.");

                if(this->CurrentExploit->Targets.Contains(Service.Service))
                {
                    Console->WriteLine("Service is &3&*vulnerable&r&7 to the &6&*" + this->CurrentExploit->FullName.ToString() + "&r&7 exploit.");

                    Console->WriteLine("Deploying &4&*" + this->CurrentPayload->Name.ToString() + "&r&7...");

                    UUserContext* PayloadUser = this->RemoteSystem->GetHackerContext(0, Console->GetUserContext());

                    this->CurrentPayload->Payload->DeployPayload(Console->GetUserContext(), PayloadUser);

                    this->HandleCommand("exit", InArguments);
                    return;
                }
                else
                {
                    Console->WriteLine("Service is &4&*not vulnerable&r&7 to this exploit. Cannot drop payload.");
                    return;
                }
            }
        }

    }
}

void AHackCommand::Tick(float InDeltaSeconds)
{
    if(this->WaitingForCommand)
    {
        FString Command;
        if(this->GetConsole()->GetLine(Command))
        {
            FString OutputError;
            TArray<FString> Tokens = UTerminalCommandParserLibrary::Tokenize(Command, this->GetConsole()->GetUserContext()->GetHomeDirectory(), OutputError);
            this->WaitingForCommand = false;

            if(OutputError.Len())
            {
                this->GetConsole()->WriteLine("error: " + OutputError);
                return;
            }

            if(Tokens.Num())
            {
                FString SubCommandName = Tokens[0].ToLower();
                Tokens.RemoveAt(0);
                this->HandleCommand(SubCommandName, Tokens);
            }
            
        }
    }
    else
    {
        this->GetConsole()->Write("&3" + this->EnteredHostname + " &7(");
        if(this->CurrentExploit)
        {
            this->GetConsole()->Write("&6&*" + this->CurrentExploit->ID.ToString() + "&r&F");
        }
        else
        {
            this->GetConsole()->Write("&6&*none&r&F");
        }
        this->GetConsole()->Write("/");
        if(this->CurrentPayload)
        {
            this->GetConsole()->Write("&C&*" + this->CurrentPayload->Name.ToString() + "&r&7");
        }
        else
        {
            this->GetConsole()->Write("&C&*none&r&7");
        }
        this->GetConsole()->Write(")> ");

        this->WaitingForCommand = true;
    }
}

void AHackCommand::NativeRunCommand(UConsoleContext* InConsole, TArray<FString> InArguments)
{
    // Get the target IP address from docopt.
    FString TargetIP = this->ArgumentMap["<host>"]->AsString();

    this->EnteredHostname = TargetIP;

    // Prompt that we're about to connect to it.
    InConsole->WriteLine("Connecting to &F" + TargetIP + "&7...");

    // Try to get a system context for theremote host.
    EConnectionError ConnectionError = EConnectionError::None;
    if(!InConsole->GetUserContext()->GetPeacenet()->ResolveSystemContext(TargetIP, this->RemoteSystem, ConnectionError))
    {
        InConsole->WriteLine(InArguments[0] + ": could not connect to remote host.");
        this->Complete();
        return;
    }

    InConsole->WriteLine("Type &Fhelp&7 for a list of commands.");
    InConsole->WriteLine("Type &Fexploits&7 for a list of your known exploits.");
}