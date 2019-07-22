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

bool AHackCommand::ShouldCrashService()
{
    float volatility = (float)this->CurrentExploit->Volatility / 5.f;

    float crashiness = volatility * 0.5f;

    float chance = FMath::FRandRange(0.f, 1.f);

    return chance < crashiness;
}

void AHackCommand::OnDisconnect()
{
    this->IsPayloadActive = false;
    this->CurrentPayload->Payload->Disconnected.Clear();
    this->SendGameEvent("HackSuccess", {
        { "Identity", FString::FromInt(this->RemoteSystem->GetCharacter().ID)},
        { "Computer", FString::FromInt(this->RemoteSystem->GetComputer().ID)}
    });
    
    // Finish up.
    this->Complete();
}

FText AHackCommand::GetShellPrompt()
{
    // The text that will be substituted into the above format string.
    FString Host = this->EnteredHostname;
    FText Exploit = NSLOCTEXT("Gigasploit", "None", "none");
    FText Payload = NSLOCTEXT("Gigasploit", "None", "none");

    // Get the actual exploit name.
    if(this->CurrentExploit)
    {
        Exploit = this->CurrentExploit->FullName;
    }

    if(this->CurrentPayload)
    {
        Payload = this->CurrentPayload->FullName;
    }

    // Write the hack command status shit to the console directly.
    // New advanced getline API isn't great for future-proofed ANSI
    // escape sequences.
    this->GetConsole()->WriteLine(FText::GetEmpty());
    this->GetConsole()->SetBold(true);
    this->GetConsole()->Write(NSLOCTEXT("GsfConsole", "CurrentExploit", "Current exploit: "));
    this->GetConsole()->SetBold(false);
    this->GetConsole()->SetForegroundColor(EConsoleColor::Yellow);
    this->GetConsole()->WriteLine(Exploit);
    this->GetConsole()->ResetForegroundColor();

    this->GetConsole()->SetBold(true);
    this->GetConsole()->Write(NSLOCTEXT("GsfConsole", "CurrentPayload", "Current payload: "));
    this->GetConsole()->SetBold(false);
    this->GetConsole()->SetForegroundColor(EConsoleColor::Red);
    this->GetConsole()->WriteLine(Payload);
    this->GetConsole()->ResetForegroundColor();
    
    this->GetConsole()->ResetFormatting();

    this->GetConsole()->WriteLine(FText::GetEmpty());

    return FText::Format(NSLOCTEXT("GsfConsole", "Prompt", "{0}> "), FText::FromString(Host));
}

void AHackCommand::Tick(float InDeltaSeconds)
{
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

    // If the payload is NOT active then we'll call up to our base Tick method to let the
    // shell system take over.
    if(!this->IsPayloadActive)
    {
        Super::Tick(InDeltaSeconds);
    }
}

float AHackCommand::AssessStealthiness()
{
    // If the origin system and remote system are the same, never report anything
    // but pure stealth.  Otherwise the game goes a little apeshit.
    if(this->GetUserContext()->GetComputer().ID == this->RemoteSystem->GetComputer().ID)
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
            if(file.Content.Contains(this->GetUserContext()->GetIPAddress()))
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
    InConsole->WriteLine(FText::Format(NSLOCTEXT("Gigasploit", "ConnectingToHost", "Connecting to &F{0}&7..."), FText::FromString(TargetIP)));

    // Try to get a system context for theremote host.
    EConnectionError ConnectionError = EConnectionError::None;
    FComputer RemotePC;
    if(!InConsole->GetUserContext()->DnsResolve(TargetIP, RemotePC, ConnectionError))
    {
        InConsole->WriteLine(FText::Format(NSLOCTEXT("Gigasploit", "ConnectionError", "{0}: could not connect to remote host."), FText::FromString(InArguments[0])));
        this->Complete();
        return;
    }

    this->RemoteSystem = this->GetUserContext()->GetPeacenet()->GetSystemContext(RemotePC.ID);

    InConsole->WriteLine(NSLOCTEXT("Gigasploit", "HelpPrompt", "Type &Fhelp&7 for a list of commands."));
    InConsole->WriteLine(NSLOCTEXT("Gigasploit", "ExploitsPrompt", "Type &Fexploits&7 for a list of your known exploits."));

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

    // Generate all the firewall rules for our remote system if necessary.
    this->RemoteSystem->GetPeacenet()->GetProcgen()->GenerateFirewallRules(this->RemoteSystem->GetComputer());
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

bool AHackCommand::RunSpecialCommand(UConsoleContext* InConsole, FString Command, TArray<FString> Arguments)
{
    if(Command == "exploits")
    {
        FString SearchQuery = "";
        for(auto Arg : Arguments)
        {
            SearchQuery += Arg + " ";
        }

        SearchQuery = SearchQuery.TrimStartAndEnd();

        TArray<FString> ExploitNames;
    
        InConsole->WriteLine(NSLOCTEXT("Gigasploit", "AvailableExploits", "&FAvailable exploits"));
        InConsole->WriteLine(NSLOCTEXT("Gigasploit", "AvailableExploitsUnderline", "--------------------&7\n"));

        bool FoundExploit = false;

        for(auto Exploit : InConsole->GetUserContext()->GetExploits())
        {
            FString ExploitName = Exploit->ID.ToString();
            if(SearchQuery.Len() == 0 || ExploitName.Contains(SearchQuery, ESearchCase::IgnoreCase))
            {
                FoundExploit = true;
                InConsole->WriteLine(FText::FromString(ExploitName), 0.2f);
            }
        }

        if(!FoundExploit)
            InConsole->WriteLine(NSLOCTEXT("Gigasploit", "NoExploitsFound", "\t&4No exploits found matching that query.&7"));

        InConsole->WriteLine(FText::GetEmpty());
        return true;
    }

    if(Command == "payloads")
    {
        FString SearchQuery = "";
        for(auto Arg : Arguments)
        {
            SearchQuery += Arg + " ";
        }

        SearchQuery = SearchQuery.TrimStartAndEnd();

        TArray<FString> ExploitNames;
    
        InConsole->WriteLine(NSLOCTEXT("Gigasploit", "AvailablePayloads", "&FAvailable payloads"));
        InConsole->WriteLine(NSLOCTEXT("Gigasploit", "AvailablePayloadsUnderline", "--------------------&7\n"));

        bool FoundExploit = false;

        for(auto Exploit : InConsole->GetUserContext()->GetPayloads())
        {
            FString ExploitName = Exploit->Name.ToString();
            if(SearchQuery.Len() == 0 || ExploitName.Contains(SearchQuery, ESearchCase::IgnoreCase))
            {
                FoundExploit = true;
                InConsole->WriteLine(FText::FromString(ExploitName), 0.2f);
            }
        }

        if(!FoundExploit)
            InConsole->WriteLine(NSLOCTEXT("Gigasploit", "NoPayloadsFound", "&4No payloads found matching that query.&7"));

        InConsole->WriteLine(FText::GetEmpty());
        return true;
    }

    if(Command == "use")
    {
        if(!Arguments.Num())
        {
            InConsole->WriteLine(NSLOCTEXT("Gigasploit", "UseCommandUsage", "&4&*error:&r too few arguments given. Do you want to use an exploit or payload?&7"));
            return true;
        }

        FString ItemType = Arguments[0].ToLower();
        Arguments.RemoveAt(0);

        FString ExploitName = "";
        for(FString Arg : Arguments)
        {
            ExploitName += Arg + " ";
        }
        ExploitName = ExploitName.TrimStartAndEnd();
        
        if(ItemType == "exploit")
        {
            for(auto Exploit : InConsole->GetUserContext()->GetExploits())
            {
                if(Exploit->ID.ToString() == ExploitName)
                {
                    InConsole->WriteLine(FText::Format(NSLOCTEXT("Gigasploit", "UsingExploit", "Using exploit &F{0}&7."), Exploit->FullName));
                    this->CurrentExploit = Exploit;
                    return true;
                }
            }

            InConsole->WriteLine(NSLOCTEXT("Gigasploit", "ExploitDoesNotExist", "&4&*error:&r exploit doesn't exist.&7"));
            return true;
        }
        else if(ItemType == "payload")
        {
            for(auto Exploit : InConsole->GetUserContext()->GetPayloads())
            {
                if(Exploit->Name.ToString() == ExploitName)
                {
                    InConsole->WriteLine(FText::Format(NSLOCTEXT("Gigasploit", "UsingPayload", "Using payload &F{0}&7."), Exploit->FullName));
                    this->CurrentPayload = Exploit;
                    return true;
                }
            }

            InConsole->WriteLine(NSLOCTEXT("Gigasploit", "PayloadDoesNotExist", "&4&*error:&r payload doesn't exist.&7"));
            return true;
        }
        else
        {
            InConsole->WriteLine(NSLOCTEXT("Gigasploit", "InvalidItemType", "&4&*error:&r Invalid item type.&7"));
            return true;
        }
    }

    if(Command == "scan")
    {
        InConsole->GetUserContext()->GetPeacenet()->SetSaveValue("gigasploit.firstScan", true);

        InConsole->WriteLine(NSLOCTEXT("Gigasploit", "StartingScan", "Performing Nmap scan on remote system..."), 0.3f);
        InConsole->WriteLine(FText::GetEmpty());
        InConsole->WriteLine(NSLOCTEXT("Gigasploit", "ScanHeadings", "PORT\t\tSTATUS\tSERVICE"), 0.5f);
        InConsole->WriteLine(NSLOCTEXT("Gigasploit", "ScanUnderline", "-----\t\t-------\t--------"));
        InConsole->WriteLine(FText::GetEmpty());
        for(auto Service : this->RemoteSystem->GetComputer().FirewallRules)
        {
            if(Service.IsCrashed) continue;

            InConsole->Write(FText::FromString(FString::FromInt(Service.Port)), 1.f); // FIXME: Please.  Use FText::AsNumber() but NO GROUPING!!
            InConsole->Write(FText::FromString("\t\t"));

            if(Service.IsFiltered)
            {
                InConsole->Write(NSLOCTEXT("Gigasploit", "Filtered", "filtered"));
            }
            else
            {
                InConsole->Write(NSLOCTEXT("Gigasploit", "Open", "open"));
            }
            InConsole->Write(FText::FromString("\t"));
            InConsole->WriteLine(Service.Service->Protocol->Name);
        }
        return true;
    }

    if(Command == "analyze")
    {
        if(!Arguments.Num())
        {
            InConsole->WriteLine(NSLOCTEXT("Gigasploit", "AnalyzeUsage", "&4&*error:&r too few arguments. You must specify a port to analyze.&7"));
            return true;
        }

        FString EnteredPort = Arguments[0];
        int RealPort = FCString::Atoi(*EnteredPort);
        if(!RealPort && EnteredPort != "0")
        {
            InConsole->WriteLine(NSLOCTEXT("Gigasploit", "PortNotNumber", "&4&*error:&r Port is not a number.&7"));
            return true;
        }

        this->ShowCoverTutorial();

        InConsole->WriteLine(NSLOCTEXT("Gigasploit", "PortAnalysis", "GIGASPLOIT PORT ANALYSIS:"));
        
        InConsole->WriteLine(FText::Format(NSLOCTEXT("Gigasploit", "PortAnalysisIP", "    {0}:{1}\n"), FText::FromString(this->RemoteSystem->GetIPAddress()), FText::FromString(EnteredPort)));

        for(auto Service : this->RemoteSystem->GetComputer().FirewallRules)
        {
            if(Service.Port == RealPort && !Service.IsCrashed)
            {
                bool FoundVulns = false;

                InConsole->WriteLine(FText::Format(NSLOCTEXT("Gigasploit", "AnalysisProtocol", "Protocol: {0}"), Service.Service->Protocol->Name));
                InConsole->WriteLine(FText::Format(NSLOCTEXT("Gigasploit", "DetectedService", "Detected implementation: {0}\n"), Service.Service->Name));

                if(Service.IsFiltered)
                {
                    this->RemoteSystem->AppendLog(this->GetUserContext()->GetIPAddress() + ": connection blocked by firewall on port " + FString::FromInt(Service.Port));
                    InConsole->WriteLine(NSLOCTEXT("Gigasploit", "FurewallDetected", "&4&*Service is filtered.&r&7 Firewall detected."));
                }
                else
                {
                    this->RemoteSystem->AppendLog(this->GetUserContext()->GetIPAddress() + ": connected to port " + FString::FromInt(Service.Port));
                    InConsole->WriteLine(NSLOCTEXT("Gigasploit", "KnownVulnerabilities", "Known vulnerabilities:\n"));

                    for(auto Exp : InConsole->GetUserContext()->GetExploits())
                    {
                        if(Exp->Targets.Contains(Service.Service))
                        {
                            FoundVulns = true;
                            InConsole->Write(FText::FromString(" - "));
                            InConsole->WriteLine(Exp->FullName);
                        }
                    }

                    if(!FoundVulns)
                    {
                        InConsole->WriteLine(NSLOCTEXT("Gigasploit", "NoVulnerabilities", " - &4&*Error:&7&r Gigasploit doesn't know any vulnerabilities in this service's implementation."));
                    }

                    this->RemoteSystem->AppendLog(this->GetUserContext()->GetIPAddress() + ": disconnected from port " + FString::FromInt(Service.Port));
                }

                InConsole->GetUserContext()->GetPeacenet()->SendGameEvent("HackAnalyze", {
                    { "Computer", FString::FromInt(this->RemoteSystem->GetComputer().ID) },
                    { "Identity", FString::FromInt(this->RemoteSystem->GetCharacter().ID) },
                    { "Port", EnteredPort },
                    { "Protocol", Service.Service->Protocol->Name.ToString() },
                    { "Implementation", Service.Service->Name.ToString() },
                    { "Filtered", (Service.IsFiltered) ? "True" : "False" },
                    { "Vulnerable", (FoundVulns) ? "True" : "False" }
                });

                return true;
            }
        }

        InConsole->WriteLine(NSLOCTEXT("Gigasploit", "AnalysisFailed", "&4&*Analysis failed:&r&7 Remote system not listening on this port."));

        return true;
    }

    if(Command == "attack")
    {
        if(!Arguments.Num())
        {
            InConsole->WriteLine(NSLOCTEXT("Gigasploit", "AttackUsage", "&4&*error:&r too few arguments. You must specify a port to attack.&7"));
            return true;
        }

        if(!this->CurrentExploit)
        {
            InConsole->WriteLine(NSLOCTEXT("Gigasploit", "MustSelectExploit", "&4&*error:&r You must select an exploit to use first.&7"));
            return true;
        }

        if(!this->CurrentPayload)
        {
            InConsole->WriteLine(NSLOCTEXT("Gigasploit", "MustSelectPayload", "&4&*error:&r You must select a payload to use first.&7"));
            return true;
        }

        FString EnteredPort = Arguments[0];
        int RealPort = FCString::Atoi(*EnteredPort);
        if(!RealPort && EnteredPort != "0")
        {
            InConsole->WriteLine(NSLOCTEXT("Gigasploit", "PortNotNumber", "&4&*error:&r Port is not a number.&7"));
            return true;
        }

        InConsole->WriteLine(FText::Format(NSLOCTEXT("Gigasploit", "FindingService", "Finding service on &3{0}&7:&6{1}&7..."), FText::FromString(EnteredHostname), FText::FromString(EnteredPort)));

        this->RunSpecialCommand(InConsole, "scan", Arguments);

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
                    InConsole->WriteLine(NSLOCTEXT("Gigasploit", "ConnectionRefused", "Connection refused."));

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

                    return true; 
                }

                InConsole->WriteLine(FText::Format(NSLOCTEXT("Gigasploit", "ServiceIs", "Service is &F{0}&7."), Service.Service->Name));

                if(Service.IsFiltered)
                {
                    this->RemoteSystem->AppendLog(this->GetUserContext()->GetIPAddress() + ": connection blocked by firewall on port " + FString::FromInt(Service.Port));

                    InConsole->WriteLine(NSLOCTEXT("Gigasploit", "ServiceIsFiltered", "Service is &4&*FILTERED&r&7! Can't continue with exploit."));
                    
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
                    
                    return true;
                }

                InConsole->WriteLine(NSLOCTEXT("Gigasploit", "ServiceIsOpen", "Service is &3OPEN&7."));

                this->RemoteSystem->AppendLog(this->GetUserContext()->GetIPAddress() + ": connected to port " + FString::FromInt(Service.Port));

                if(this->CurrentExploit->Targets.Contains(Service.Service))
                {
                    InConsole->WriteLine(FText::Format(NSLOCTEXT("Gigasploit", "ServiceIsVulnerable", "Service is &3&*vulnerable&r&7 to the &6&*{0}&r&7 exploit."), this->CurrentExploit->FullName));

                    InConsole->WriteLine(FText::Format(NSLOCTEXT("Gigasploit", "DeployingPayload", "Deploying &4&*{0}&r&7..."), this->CurrentPayload->FullName));

                    if(this->ShouldCrashService())
                    {
                        this->RemoteSystem->AppendLog(Service.Service->Name.ToString() + ": service stopped unexpectedly.");
                        this->RemoteSystem->AppendLog(this->GetUserContext()->GetIPAddress() + ": disconnected from port " + FString::FromInt(Service.Port));
                        Service.IsCrashed = true;
                        InConsole->WriteLine(NSLOCTEXT("Gigasploit", "ConnectionRefused", "Connection refused."));
                        return true;
                    }

                    UUserContext* PayloadUser = this->RemoteSystem->GetHackerContext(0, InConsole->GetUserContext());

                    TScriptDelegate<> DisconnectedEvent;
                    DisconnectedEvent.BindUFunction(this, "OnDisconnect");
                    this->CurrentPayload->Payload->Disconnected.Add(DisconnectedEvent);

                    this->IsPayloadActive = true;

                    this->CurrentPayload->Payload->DeployPayload(this->GetConsole(), InConsole->GetUserContext(), PayloadUser);

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

                    this->RemoteSystem->AppendLog(this->GetUserContext()->GetIPAddress() + ": disconnected from port " + FString::FromInt(Service.Port));

                    return true;
                }
                else
                {
                    this->RemoteSystem->AppendLog(this->GetUserContext()->GetIPAddress() + ": disconnected from port " + FString::FromInt(Service.Port));
                    InConsole->WriteLine(NSLOCTEXT("Gigasploit", "ServiceNotVulnerable", "Service is &4&*not vulnerable&r&7 to this exploit. Cannot drop payload."));
                    return true;
                }
            }
        }
        return true;
    }

    return false;
}