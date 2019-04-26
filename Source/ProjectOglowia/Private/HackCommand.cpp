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
#include "ConnectionError.h"
#include "FirewallRule.h"
#include "PayloadAsset.h"
#include "Computer.h"
#include "ProceduralGenerationEngine.h"
#include "ProtocolVersion.h"
#include "TerminalCommandParserLibrary.h"

AHackCommand::AHackCommand()
{
    // Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

void AHackCommand::HandleCommand(FString InCommandName, TArray<FString> InArguments)
{
    UConsoleContext* MyConsole = this->GetConsole();

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
    
        MyConsole->WriteLine("\t&FAvailable exploits");
        MyConsole->WriteLine("\t--------------------&7\n");

        bool FoundExploit = false;

        for(auto Exploit : MyConsole->GetUserContext()->GetExploits())
        {
            FString ExploitName = Exploit->ID.ToString();
            if(SearchQuery.Len() == 0 || ExploitName.Contains(SearchQuery, ESearchCase::IgnoreCase))
            {
                FoundExploit = true;
                MyConsole->WriteLine("\t" + ExploitName);
            }
        }

        if(!FoundExploit)
            MyConsole->WriteLine("\t&4No exploits found matching that query.&7");

        MyConsole->WriteLine("");
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
    
        MyConsole->WriteLine("\t&FAvailable payloads");
        MyConsole->WriteLine("\t--------------------&7\n");

        bool FoundExploit = false;

        for(auto Exploit : MyConsole->GetUserContext()->GetPayloads())
        {
            FString ExploitName = Exploit->Name.ToString();
            if(SearchQuery.Len() == 0 || ExploitName.Contains(SearchQuery, ESearchCase::IgnoreCase))
            {
                FoundExploit = true;
                MyConsole->WriteLine("\t" + ExploitName);
            }
        }

        if(!FoundExploit)
            MyConsole->WriteLine("\t&4No payloads found matching that query.&7");

        MyConsole->WriteLine("");
        return;
    }

    if(InCommandName == "use")
    {
        if(!InArguments.Num())
        {
            MyConsole->WriteLine("&4&*error:&r too few arguments given. Do you want to use an exploit or payload?&7");
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
            for(auto Exploit : MyConsole->GetUserContext()->GetExploits())
            {
                if(Exploit->ID.ToString() == ExploitName)
                {
                    MyConsole->WriteLine("Using exploit &F" + Exploit->FullName.ToString() + "&7.");
                    this->CurrentExploit = Exploit;
                    return;
                }
            }

            MyConsole->WriteLine("&4&*error:&r exploit doesn't exist.&7");
            return;
        }
        else if(ItemType == "payload")
        {
            for(auto Exploit : MyConsole->GetUserContext()->GetPayloads())
            {
                if(Exploit->Name.ToString() == ExploitName)
                {
                    MyConsole->WriteLine("Using payload &F" + ExploitName + "&7.");
                    this->CurrentPayload = Exploit;
                    return;
                }
            }

            MyConsole->WriteLine("&4&*error:&r payload doesn't exist.&7");
            return;
        }
        else
        {
            MyConsole->WriteLine("&4&*error:&r Invalid item type: &7" + ItemType);
            return;
        }
    }

    if(InCommandName == "scan")
    {
        MyConsole->GetUserContext()->GetPeacenet()->SaveGame->SetValue("gigasploit.firstScan", true);

        MyConsole->WriteLine("Performing Nmap scan on remote system...");
        MyConsole->WriteLine("");
        MyConsole->WriteLine("PORT\t\tSTATUS\tSERVICE");
        MyConsole->WriteLine("-----\t\t-------\t--------");
        MyConsole->WriteLine("");
        this->RemoteSystem->GetPeacenet()->GetProcgen()->GenerateFirewallRules(this->RemoteSystem->GetComputer());
        for(auto Service : this->RemoteSystem->GetComputer().FirewallRules)
        {
            MyConsole->Write(FString::FromInt(Service.Port));
            MyConsole->Write("\t\t");

            if(Service.IsFiltered)
            {
                MyConsole->Write("&4filtered&7");
            }
            else
            {
                MyConsole->Write("&3open&7");
            }
            MyConsole->Write("\t");
            MyConsole->WriteLine(Service.Service->Name.ToString());
        }
        return;
    }

    if(InCommandName == "analyze")
    {
        if(!InArguments.Num())
        {
            MyConsole->WriteLine("&4&*error:&r too few arguments. You must specify a port to analyze.&7");
            return;
        }

        FString EnteredPort = InArguments[0];
        int RealPort = FCString::Atoi(*EnteredPort);
        if(!RealPort && EnteredPort != "0")
        {
            MyConsole->WriteLine("&4&*error:&r Port is not a number.&7");
            return;
        }

        this->ShowCoverTutorial();

        MyConsole->WriteLine("GIGASPLOIT PORT ANALYSIS:");
        MyConsole->WriteLine("    " + this->RemoteSystem->GetIPAddress() + ":" + EnteredPort + "\n");

        for(auto Service : this->RemoteSystem->GetComputer().FirewallRules)
        {
            if(Service.Port == RealPort)
            {
                bool FoundVulns = false;

                MyConsole->WriteLine("Protocol: " + Service.Service->Protocol->Name.ToString());
                MyConsole->WriteLine("Detected implementation: " + Service.Service->Name.ToString() + "\n");

                if(Service.IsFiltered)
                {
                    MyConsole->WriteLine("&4&*Service is filtered.&r&7 Firewall detected.");
                }
                else
                {
                    MyConsole->WriteLine("Known vulnerabilities:\n");

                    for(auto Exp : MyConsole->GetUserContext()->GetExploits())
                    {
                        if(Exp->Targets.Contains(Service.Service))
                        {
                            FoundVulns = true;
                            MyConsole->WriteLine(" - " + Exp->ID.ToString());
                        }
                    }

                    if(!FoundVulns)
                    {
                        MyConsole->WriteLine(" - &4&*Error:&7&r Gigasploit doesn't know any vulnerabilities in this service's implementation.");
                    }
                }

                MyConsole->GetUserContext()->GetPeacenet()->SendGameEvent("HackAnalyze", {
                    { "Computer", FString::FromInt(this->RemoteSystem->GetComputer().ID) },
                    { "Identity", FString::FromInt(this->RemoteSystem->GetCharacter().ID) },
                    { "Port", EnteredPort },
                    { "Protocol", Service.Service->Protocol->Name.ToString() },
                    { "Implementation", Service.Service->Name.ToString() },
                    { "Filtered", (Service.IsFiltered) ? "True" : "False" },
                    { "Vulnerable", (FoundVulns) ? "True" : "False" }
                });

                return;
            }
        }

        MyConsole->WriteLine("&4&*Analysis failed:&r&7 Remote system not listening on this port.");

        return;
    }

    if(InCommandName == "attack")
    {
        if(!InArguments.Num())
        {
            MyConsole->WriteLine("&4&*error:&r too few arguments. You must specify a port to attack.&7");
            return;
        }

        if(!this->CurrentExploit)
        {
            MyConsole->WriteLine("&4&*error:&r You must select an exploit to use first.&7");
            return;
        }

        if(!this->CurrentPayload)
        {
            MyConsole->WriteLine("&4&*error:&r You must select a payload to use first.&7");
            return;
        }

        FString EnteredPort = InArguments[0];
        int RealPort = FCString::Atoi(*EnteredPort);
        if(!RealPort && EnteredPort != "0")
        {
            MyConsole->WriteLine("&4&*error:&r Port is not a number.&7");
            return;
        }

        MyConsole->WriteLine("Finding service on &3" + EnteredHostname + "&7:&6" + EnteredPort + "&7...");

        this->HandleCommand("scan", InArguments);

        for(auto Service : this->RemoteSystem->GetComputer().FirewallRules)
        {
            if(Service.Port == RealPort)
            {
                this->SendGameEvent("HackAttempt", {
                    { "Identity", FString::FromInt(this->RemoteSystem->GetCharacter().ID)},
                    { "Computer", FString::FromInt(this->RemoteSystem->GetComputer().ID)},
                    { "Exploit", this->CurrentExploit->ID.ToString()},
                    { "Payload", this->CurrentPayload->Name.ToString()},
                    { "Port", FString::FromInt(RealPort)},
                    { "Protocol", Service.Service->Protocol->Name.ToString()},
                    { "ServerSoftware", Service.Service->Name.ToString()}
                });

                MyConsole->WriteLine("Service is &F" + Service.Service->Name.ToString() + "&7.");

                if(Service.IsFiltered)
                {
                    MyConsole->WriteLine("Service is &4&*FILTERED&r&7! Can't continue with exploit.");
                    
                    this->SendGameEvent("HackFail", {
                        { "Identity", FString::FromInt(this->RemoteSystem->GetCharacter().ID)},
                        { "Computer", FString::FromInt(this->RemoteSystem->GetComputer().ID)},
                        { "Exploit", this->CurrentExploit->ID.ToString()},
                        { "Payload", this->CurrentPayload->Name.ToString()},
                        { "Port", FString::FromInt(RealPort)},
                        { "Protocol", Service.Service->Protocol->Name.ToString()},
                        { "ServerSoftware", Service.Service->Name.ToString()},
                        { "Reason", "Firewall"}
                    });
                    
                    return;
                }

                MyConsole->WriteLine("Service is &3OPEN&7.");

                if(this->CurrentExploit->Targets.Contains(Service.Service))
                {
                    MyConsole->WriteLine("Service is &3&*vulnerable&r&7 to the &6&*" + this->CurrentExploit->FullName.ToString() + "&r&7 exploit.");

                    MyConsole->WriteLine("Deploying &4&*" + this->CurrentPayload->Name.ToString() + "&r&7...");

                    UUserContext* PayloadUser = this->RemoteSystem->GetHackerContext(0, MyConsole->GetUserContext());

                    TScriptDelegate<> DisconnectedEvent;
                    DisconnectedEvent.BindUFunction(this, "OnDisconnect");
                    this->CurrentPayload->Payload->Disconnected.Add(DisconnectedEvent);

                    this->IsPayloadActive = true;

                    this->CurrentPayload->Payload->DeployPayload(this->GetConsole(), MyConsole->GetUserContext(), PayloadUser);

                    this->SendGameEvent("PayloadDeploy", {
                        { "Identity", FString::FromInt(this->RemoteSystem->GetCharacter().ID)},
                        { "Computer", FString::FromInt(this->RemoteSystem->GetComputer().ID)},
                        { "Exploit", this->CurrentExploit->ID.ToString()},
                        { "Payload", this->CurrentPayload->Name.ToString()},
                        { "Port", FString::FromInt(RealPort)},
                        { "Protocol", Service.Service->Protocol->Name.ToString()},
                        { "ServerSoftware", Service.Service->Name.ToString()}
                    });

                    this->ShowPayloadTutorial();

                    return;
                }
                else
                {
                    MyConsole->WriteLine("Service is &4&*not vulnerable&r&7 to this exploit. Cannot drop payload.");
                    return;
                }
            }
        }

    }
}

void AHackCommand::OnDisconnect()
{
    this->IsPayloadActive = false;
    this->CurrentPayload->Payload->Disconnected.Clear();
    this->SendGameEvent("HackSuccess", {
        { "Identity", FString::FromInt(this->RemoteSystem->GetCharacter().ID)},
        { "Computer", FString::FromInt(this->RemoteSystem->GetComputer().ID)}
    });
    this->HandleCommand("exit", { "" });
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
        if(!this->IsPayloadActive)
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

    if(!this->IsTutorialActive())
    {
        if(this->IsSet("gigasploit.firstScan"))
        {
            this->ShowTutorialIfNotSet("tuts.gigasploit.analyze", 
                NSLOCTEXT("Tutorials", "GigasploitAnalyzeTitle", "Analyzing a port"),
                NSLOCTEXT("Tutorials", "GigasploitAnalyze", "Gigasploit Analyze Tutorial")
            );
            this->ShowTutorialIfNotSet("tuts.gigasploit.exploits", 
                NSLOCTEXT("Tutorials", "GigasploitExploitsTitle", "Exploits"),
                NSLOCTEXT("Tutorials", "GigasploitExploits", "Gigasploit Exploits Tutorial")
            );
            this->ShowTutorialIfNotSet("tuts.gigasploit.payloads", 
                NSLOCTEXT("Tutorials", "GigasploitPayloadsTitle", "Payloads"),
                NSLOCTEXT("Tutorials", "GigasploitPayloads", "Gigasploit Payloads Tutorial")
            );
            this->ShowTutorialIfNotSet("tuts.gigasploit.use", 
                NSLOCTEXT("Tutorials", "GigasploitUseTitle", "Using"),
                NSLOCTEXT("Tutorials", "GigasploitUse", "Gigasploit Use Tutorial")
            );
            this->ShowTutorialIfNotSet("tuts.gigasploit.attack", 
                NSLOCTEXT("Tutorials", "GigasploitAttackTitle", "Attack"),
                NSLOCTEXT("Tutorials", "GigasploitAttack", "Gigasploit Attack Tutorial")
            );
        }
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

    // Broadcast a mission event that a hack has started.
    InConsole->GetUserContext()->GetPeacenet()->SendGameEvent("HackStart", {
        { "Computer", FString::FromInt(this->RemoteSystem->GetComputer().ID) },
        { "Identity", FString::FromInt(this->RemoteSystem->GetCharacter().ID) },
        { "Host", TargetIP },        
    });

    this->ShowTutorialIfNotSet("tuts.gigasploit.welcome",
            NSLOCTEXT("CommandNames", "Gigasploit", "Gigasploit Framework Console"),
            NSLOCTEXT("Tutorials", "GigasploitWelcome", "Gigasploit Welcome Tutorial Text")
        );
}

void AHackCommand::ShowCoverTutorial()
{
    this->ShowTutorialIfNotSet("tuts.gameplay.cover",
        NSLOCTEXT("Tutorials", "CoverTitle", "Cover"),
        NSLOCTEXT("Tutorials", "Cover", "Cover tutorial")
    );
}

void AHackCommand::ShowPayloadTutorial()
{
        this->ShowTutorialIfNotSet("tuts.gameplay.cover.cleanup",
        NSLOCTEXT("Tutorials", "CoverTitle", "Cover"),
        NSLOCTEXT("Tutorials", "CoverCleanup", "Cleaning up after yourself")
    );
}