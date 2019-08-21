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

#include "NmapCommand.h"
#include "UserContext.h"
#include "CommonUtils.h"
#include "ProtocolVersion.h"

// nmap
//
// Summary: Allows mapping out of networks and systems.
//
// Usage:
//   nmap  - scan for adjacent systems to hack.
//  nmap [target]  - scan a specific target.
//
// Argument summary:
//   [target]:  Specifies a target for nmap to scan.  It can be either a hostname, IP address, or host:port.
//              In the case of just a hostname, the hostname is resolved by the Peacenet World State to an
//              IP address which is then used to map to a computer ID.  If this is unsuccessful, the network
//              simulation will emit a timeout error.
//
//              IP addresses are handled in much the same way, however DNS resolution is skipped.  If a port
//              number is specified by the target, it is parsed out before resolution begins and tells the game
//              to list information about a specific service on the resolved host if the service is open.
//
//              If a port cannot be resolved to a service, the network simulation will emit a connection refused
//              error.
//
//              EXAMPLES:
//                nmap kaylincomputing.com      - resolves "kaylincomputing.com" to an IP address, resolving
//                                                this IP address to a computer ID, then lists all open and filtered
//                                                Peacegate services.
//                nmap 142.58.37.4              - maps the IP address 142.58.37.4 to a computer ID and lists all active
//                                               and filtered Peacenet services.
//                nmap kaylincomputing.com:80  - Maps kaylincomputing.com to a computer ID and lists detailed information
//                                               about the service running on port 80 of kaylincomputing.com's computer.
//                nmap 86.75.30.9:80           - See nmap kaylincomputing.com:80 example.
void ANmapCommand::NativeRunCommand(UConsoleContext* InConsole, TArray<FString> InArguments) {
    if(this->ArgumentMap["<target>"]->IsEmpty()) {
        // Scan for nearby hosts.
        for(auto Host : this->GetUserContext()->GetNearbyHosts()) {
            InConsole->WriteLine(FText::FromString(Host));
        }
    } else {
        FString Address;
        int Port;
        bool HasPort;

        // Parse out the port and determine if we even have one.
        UCommonUtils::ParseHost(this->ArgumentMap["<target>"]->AsString(), Address, HasPort, Port);

        // Now we can do the DNS resolve.
        FComputer Computer;
        EConnectionError ConnectionError;
        bool Resolved = this->GetUserContext()->DnsResolve(Address, Computer, ConnectionError);

        InConsole->WriteLine(FText::Format(NSLOCTEXT("Nmap", "ScanReport", "Nmap scan report for {0}:"), FText::FromString(Address)));

        if(Resolved) {
            InConsole->WriteLine(NSLOCTEXT("Nmap", "HostUp", "Host is up."));

            if(HasPort) {
                for(auto Service : Computer.FirewallRules) {
                    if(Service.Port == Port && !Service.IsCrashed) {
                        bool FoundVulns = false;

                        InConsole->SetBold(true);
                        InConsole->Write(NSLOCTEXT("Nmap", "AnalysisProtocol", "Protocol: "));
                        InConsole->SetBold(false);
                        InConsole->WriteLine(Service.Service->Protocol->Name);

                        InConsole->SetBold(true);
                        InConsole->Write(NSLOCTEXT("Nmap", "DetectedService", "Detected implementation: "));
                        InConsole->SetBold(false);
                        InConsole->WriteLine(Service.Service->Name);

                        InConsole->WriteEmptyLine();

                        if(Service.IsFiltered) {
                            InConsole->SetBold(true);
                            InConsole->SetForegroundColor(EConsoleColor::Red);
                            InConsole->WriteLine(NSLOCTEXT("Nmap", "FurewallDetected", "Service is filtered. Firewall detected."));
                            InConsole->ResetFormatting();
                        } else {
                            InConsole->SetBold(true);
                            InConsole->WriteLine(NSLOCTEXT("Nmap", "KnownVulnerabilities", "Known vulnerabilities:\n"));
                            InConsole->ResetFormatting();

                            for(auto Exp : InConsole->GetUserContext()->GetExploits()) {
                                if(Exp->Targets.Contains(Service.Service)) {
                                    FoundVulns = true;
                                    InConsole->Write(FText::FromString(" - "));
                                    InConsole->SetForegroundColor(EConsoleColor::Yellow);
                                    InConsole->WriteLine(Exp->FullName);
                                    InConsole->ResetForegroundColor();
                                }
                            }

                            if(!FoundVulns) {
                                InConsole->Write(FText::FromString(" - "));
                                InConsole->SetForegroundColor(EConsoleColor::Red);
                                InConsole->WriteLine(NSLOCTEXT("Nmap", "NoVulnerabilities", "Error: Nmap doesn't know any vulnerabilities in this service's implementation."));
                                InConsole->ResetFormatting();
                            }

                            this->Complete();
                            return;
                        }
                    }
                }

                InConsole->SetBold(true);
                InConsole->SetForegroundColor(EConsoleColor::Red);
                InConsole->WriteLine(NSLOCTEXT("Nmap", "AnalysisFailed", "Analysis failed: Remote system not listening on this port."));
                InConsole->ResetFormatting();

            } else {
                int LongestSvcLen = 0;
                int LongestPortLen = 0;
                int LongestStatLen = 0;

                TArray<FString> Ports;
                TArray<FText> Names;
                TArray<bool> Filters;

                for(auto Svc : Computer.FirewallRules) {
                    if(Svc.IsCrashed) {
                        continue;
                    }

                    FText FilteredStat = NSLOCTEXT("Nmap", "Open", "Open");
                    if(Svc.IsFiltered) {
                        FilteredStat = NSLOCTEXT("Nmap", "Filtered", "Filtered");
                    }

                    int StatLen = FilteredStat.ToString().Len();
                    if(StatLen > LongestStatLen) {
                        LongestStatLen = StatLen;
                    }

                    FText Name = Svc.Service->Protocol->Name;
                    if(Name.ToString().Len() > LongestSvcLen) {
                        LongestSvcLen = Name.ToString().Len();
                    }

                    FString SvcPort = FString::FromInt(Svc.Port);
                    if(SvcPort.Len() > LongestPortLen) {
                        LongestPortLen = SvcPort.Len();
                    }

                    Names.Add(Name);
                    Ports.Add(SvcPort);
                    Filters.Add(Svc.IsFiltered);
                }

                FText NameHead = NSLOCTEXT("Nmap", "ScanNameHead", "Name");
                FText PortHead = NSLOCTEXT("Nmap", "ScanPortHead", "Port");
                FText FilteredHead = NSLOCTEXT("Nmap", "ScanFilterHead", "Status");
                
                int NameHeadLen = NameHead.ToString().Len();
                int PortHeadLen = PortHead.ToString().Len();
                int FilterHeadLen = FilteredHead.ToString().Len();

                if(NameHeadLen > LongestSvcLen) {
                    LongestSvcLen = NameHeadLen;
                }
                if(PortHeadLen > LongestPortLen) {
                    LongestPortLen = PortHeadLen;
                }
                if(FilterHeadLen > LongestStatLen) {
                    LongestStatLen = FilterHeadLen;
                }

                InConsole->SetBold(true);
                InConsole->Write(PortHead);
                for(int i = 0; i <= LongestPortLen - PortHeadLen; i++) {
                    InConsole->Write(FText::FromString(" "));
                }
                InConsole->Write(FilteredHead);
                for(int i = 0; i <= LongestStatLen - FilterHeadLen; i++) {
                    InConsole->Write(FText::FromString(" "));
                }
                InConsole->WriteLine(NameHead);
                InConsole->SetBold(false);

                InConsole->WriteLine(FText::GetEmpty());

                for(int i = 0; i < Names.Num(); i++) {
                    FText Name = Names[i];
                    FText SvcPort = FText::FromString(Ports[i]);
                    FText Filter = Filters[i] ? NSLOCTEXT("Nmap", "Filtered", "Filtered") : NSLOCTEXT("Nmap", "Open", "Open");

                    int NameLen = Name.ToString().Len();
                    int PortLen = SvcPort.ToString().Len();
                    int FilterLen = Filter.ToString().Len();

                    InConsole->Write(SvcPort);
                    for(int j = 0; j <= (LongestPortLen - PortLen); j++) {
                        InConsole->Write(FText::FromString(" "));
                    }
                    InConsole->Write(Filter);
                    for(int j = 0; j <= (LongestStatLen - FilterLen); j++) {
                        InConsole->Write(FText::FromString(" "));
                    }
                    InConsole->WriteLine(Name);
                }

                InConsole->ResetFormatting();
                InConsole->WriteLine(FText::GetEmpty());
            }
        } else {
            InConsole->SetForegroundColor(EConsoleColor::Magenta);
            InConsole->WriteLine(UCommonUtils::GetConnectionError(ConnectionError));
            InConsole->ResetFormatting();
        }
    }

    this->Complete();
}