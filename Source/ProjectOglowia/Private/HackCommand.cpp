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

bool AHackCommand::ShouldCrashService()
{
    float volatility = (float)this->CurrentExploit->Volatility / 5.f;

    float crashiness = volatility * 0.5f;

    float chance = FMath::FRandRange(0.f, 1.f);

    return chance < crashiness;
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
            if(Service.IsCrashed) continue;

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
            MyConsole->WriteLine(Service.Service->Protocol->Name.ToString());
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
            if(Service.Port == RealPort && !Service.IsCrashed)
            {
                bool FoundVulns = false;

                MyConsole->WriteLine("Protocol: " + Service.Service->Protocol->Name.ToString());
                MyConsole->WriteLine("Detected implementation: " + Service.Service->Name.ToString() + "\n");

                if(Service.IsFiltered)
                {
                    this->RemoteSystem->AppendLog(this->GetUserContext()->GetOwningSystem()->GetIPAddress() + ": connection blocked by firewall on port " + FString::FromInt(Service.Port));
                    MyConsole->WriteLine("&4&*Service is filtered.&r&7 Firewall detected.");
                }
                else
                {
                    this->RemoteSystem->AppendLog(this->GetUserContext()->GetOwningSystem()->GetIPAddress() + ": connected to port " + FString::FromInt(Service.Port));
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

                    this->RemoteSystem->AppendLog(this->GetUserContext()->GetOwningSystem()->GetIPAddress() + ": disconnected from port " + FString::FromInt(Service.Port));
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

        for(auto& Service : this->RemoteSystem->GetComputer().FirewallRules)
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

                if(Service.IsCrashed)
                {
                    MyConsole->WriteLine("Connection refused.");

                    this->SendGameEvent("HackFail", {
                        { "Identity", FString::FromInt(this->RemoteSystem->GetCharacter().ID)},
                        { "Computer", FString::FromInt(this->RemoteSystem->GetComputer().ID)},
                        { "Exploit", this->CurrentExploit->ID.ToString()},
                        { "Payload", this->CurrentPayload->Name.ToString()},
                        { "Port", FString::FromInt(RealPort)},
                        { "Protocol", Service.Service->Protocol->Name.ToString()},
                        { "ServerSoftware", Service.Service->Name.ToString()},
                        { "Reason", "Crash"}
                    });

                    return;
                }

                MyConsole->WriteLine("Service is &F" + Service.Service->Name.ToString() + "&7.");

                if(Service.IsFiltered)
                {
                    this->RemoteSystem->AppendLog(this->GetUserContext()->GetOwningSystem()->GetIPAddress() + ": connection blocked by firewall on port " + FString::FromInt(Service.Port));

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

                this->RemoteSystem->AppendLog(this->GetUserContext()->GetOwningSystem()->GetIPAddress() + ": connected to port " + FString::FromInt(Service.Port));

                if(this->CurrentExploit->Targets.Contains(Service.Service))
                {
                    MyConsole->WriteLine("Service is &3&*vulnerable&r&7 to the &6&*" + this->CurrentExploit->FullName.ToString() + "&r&7 exploit.");

                    MyConsole->WriteLine("Deploying &4&*" + this->CurrentPayload->Name.ToString() + "&r&7...");

                    if(this->ShouldCrashService())
                    {
                        this->RemoteSystem->AppendLog(Service.Service->Name.ToString() + ": service stopped unexpectedly.");
                        this->RemoteSystem->AppendLog(this->GetUserContext()->GetOwningSystem()->GetIPAddress() + ": disconnected from port " + FString::FromInt(Service.Port));
                        Service.IsCrashed = true;
                        MyConsole->WriteLine("Connection refused.");
                        return;
                    }

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

                    this->RemoteSystem->AppendLog(this->GetUserContext()->GetOwningSystem()->GetIPAddress() + ": disconnected from port " + FString::FromInt(Service.Port));

                    return;
                }
                else
                {
                    this->RemoteSystem->AppendLog(this->GetUserContext()->GetOwningSystem()->GetIPAddress() + ": disconnected from port " + FString::FromInt(Service.Port));
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
                NSLOCTEXT("Tutorials", "GigasploitAnalyze", "While <ui>scanning</> a system will tell you the active <ui>ports</> of the system, if you'd like to see more details about a <ui>specific port</> you need to <ui>analyze</> it.\r\n\r\nThis can be done by running the <cmd>analyze <port></> command.\r\n\r\nThis will tell you more information about the port, such as what server software is running it and what <ui>exploits</> you can use.\r\n\r\n<bad>WARNING</>: Analyzing a port may result in your IP address being logged, thus decreasing your <ui>cover</>.")
            );
            this->ShowTutorialIfNotSet("tuts.gigasploit.exploits", 
                NSLOCTEXT("Tutorials", "GigasploitExploitsTitle", "Exploits"),
                NSLOCTEXT("Tutorials", "GigasploitExploits", "An <ui>exploit</> is used to tell <ui>Gigasploit</> how to attack a specific <ui>vulnerability</> in a service to allow it to <bad>deploy a payload</>.\r\n\r\nRun the <cmd>exploits</> command to see all of the exploits you currently have available to you.  You will find more exploits as you hack more systems.")
            );
            this->ShowTutorialIfNotSet("tuts.gigasploit.exploits.more", 
                NSLOCTEXT("Tutorials", "GigasploitExploitsTitle", "Exploits"),
                NSLOCTEXT("Tutorials", "GigasploitExploits", "To see more information about a particular <ui>exploit</>, you can use the <cmd>man <exploit></> command in another Terminal.\r\n\r\nSome exploits are more stable than others, and the more stable an exploit is, the less chance there is of it crashing the service it attacks.\r\n\r\nTo <ui>use</> an exploit, run <cmd>use exploit <exploit></>.")
            );
            this->ShowTutorialIfNotSet("tuts.gigasploit.payloads", 
                NSLOCTEXT("Tutorials", "GigasploitPayloadsTitle", "Payloads"),
                NSLOCTEXT("Tutorials", "GigasploitPayloads", "<ui>Payloads</> are tiny programs that get deployed to the remote computer using <ui>exploits</>.  You can use any <ui>payload</> you'd like, but there are certain payloads that are better for different jobs.\r\n\r\nThe <ui>most common</> payload you will use is the <ui>reverse shell</> - which will connect to your computer and allow you to take control of the remote computer.\r\n\r\nOther <ui>payloads</> can be used to open <ui>additional services</>, bypass <bad>firewalls</>, or even <bad>crash</> a service or the whole system.\r\n\r\nTo see all of your available payloads, run the <cmd>payloads</> command.  You will find more payloads as you hack more systems.\r\n\r\nTo <ui>use</> a payload, run the <cmd>use payload <payload></> command.")
            );
            this->ShowTutorialIfNotSet("tuts.gigasploit.hud", 
                NSLOCTEXT("Tutorials", "GigasploitHudTitle", "Gigasploit HUD"),
                NSLOCTEXT("Tutorials", "GigasploitHud", "The <ui>remote system</>'s IP address is displayed in <ui>Gigasploit</>'s prompt.  Next to the IP address is the currently-selected <ui>exploit</> in yellow and <ui>payload</> in red.\r\n\r\nBoth an <ui>exploit</> and a <ui>payload</> need to be selected before you can <ui>attack</>.")
            );
            this->ShowTutorialIfNotSet("tuts.gigasploit.attack", 
                NSLOCTEXT("Tutorials", "GigasploitAttackTitle", "Launching the attack"),
                NSLOCTEXT("Tutorials", "GigasploitAttack", "Once you are ready to <bad>launch the attack</>, run the <cmd>attack <port></> command.  This will attempt to attack the specified port with the selected exploit and deploy the payload.\r\n\r\nIf the specified port is <bad>blocked</> by a <bad>firewall</>, or the service <bad>crashes</> because of the exploit, the attack will <bad>fail</> and you'll need to try a different attack.\r\n\r\nIf the attack <ui>succeeds</>, Gigasploit will exit and, if necessary, let the <ui>payload</> have access to your <ui>Terminal</>.")
            );
        }
     }

    // Assess the player's stealthiness.  If it's different than
    // before then we need to set the player's stealthiness value
    // in the world state if the new value is below the current.
    float stealthiness = this->AssessStealthiness();

    if(this->LastStealthiness != stealthiness)
    {
        this->LastStealthiness = stealthiness;

        if(stealthiness < this->GetUserContext()->GetStealthiness())
        {
            this->GetUserContext()->SetStealthiness(stealthiness);
        }
    }
}

float AHackCommand::AssessStealthiness()
{
    // If the origin system and remote system are the same, never report anything
    // but pure stealth.  Otherwise the game goes a little apeshit.
    if(this->GetUserContext()->GetOwningSystem()->GetComputer().ID == this->RemoteSystem->GetComputer().ID)
        return 1.f;

    // Stealthiness is a percentage value.
    float stealthiness = 1.f;

    // So basically what we're going to do is...
    //
    // 1. Check how many text files in the remote system
    // contain the local system's IP address compared to how many text files
    // are actually on the system - and calculate a percentage.  This percentage counts for
    // 50% of the player's stealthiness value.
    //
    // 2. 20% of the stealthiness comes from how many services have crashed compared to the total number of
    // services on the computer in the first place.
    //
    
    // Start by calculating how many tracks the player left behind.
    float filesCount = (float) this->RemoteSystem->GetComputer().TextFiles.Num();
    float filesContainingIPAddress = 0.f;

    if(filesCount > 0)
    {
        for(auto& file : this->RemoteSystem->GetComputer().TextFiles)
        {
            if(file.Content.Contains(this->GetUserContext()->GetOwningSystem()->GetIPAddress()))
            {
                filesContainingIPAddress += 1.f;
            }
        }

        float tracksLeftPercentage = (filesContainingIPAddress / filesCount) * 0.5f;

        // Decrease stealthiness by that amount.
        stealthiness -= tracksLeftPercentage;
    }

    // Check crashiness percentage.
    float serviceCount = this->RemoteSystem->GetComputer().FirewallRules.Num();
    float crashes = 0.f;

    if(serviceCount > 0)
    {
        for(auto& rule : this->RemoteSystem->GetComputer().FirewallRules)
        {
            if(rule.IsCrashed) crashes += 1.f;
        }

        stealthiness -= (crashes / serviceCount) * 0.20f;
    }

    // Step 3 would be take off another 20% if the system's owner is currently using the system
    // but this isn't implemented.

    // Return the stealthiness value as a definite percentage.
    return FMath::Clamp(stealthiness, 0.f, 1.f);
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
            NSLOCTEXT("Tutorials", "GigasploitWelcome", "This is the <ui>Gigasploit Framework Console</>.\r\n\r\nMost <bad>hacking</> operations will be performed here.\r\n\r\nTo <bad>hack</> a system, you must select an <ui>exploit</>, a <ui>payload</>, and attack a <ui>service</>.\r\n\r\nTo <ui>scan</> the system for hack-able <ui>services</>, run the <cmd>scan</> command.")
        );
}

void AHackCommand::ShowCoverTutorial()
{
    this->ShowTutorialIfNotSet("tuts.gameplay.cover",
        NSLOCTEXT("Tutorials", "CoverTitle", "Cover"),
        NSLOCTEXT("Tutorials", "Cover", "During a <bad>hack</>, you must remain under <ui>cover</>.\r\n\r\nYour <ui>cover</> meter is displayed in the top-right corner of the screen.  The lower the percentage, the closer you are to <bad>blowing your cover</>.\r\n\r\nActions such as crashing remote services, analyzing a port, using louder exploits or payloads, and forgetting to delete logs will decrease your <ui>cover meter</>.\r\n\r\nYour cover meter will rise back up to 100% over time if you avoid doing these things.\r\n\r\nIf your cover falls below 45%, you will enter a state of <bad>ALERT</> and will need to <ui>regain your cover</>.")
    );
}

void AHackCommand::ShowPayloadTutorial()
{
        this->ShowTutorialIfNotSet("tuts.gameplay.cover.cleanup",
        NSLOCTEXT("Tutorials", "CoverTitle", "Cover"),
        NSLOCTEXT("Tutorials", "CoverCleanup", "Every action you perform on a remote system has the potential of <bad>leaving tracks behind</> in the form of <ui>log files</>.\r\n\r\nTo avoid <bad>blowing your cover</>, be sure to <ui>delete</> any logs left in <cmd>/var/log</>.")
    );
}