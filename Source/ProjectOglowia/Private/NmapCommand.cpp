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
void ANmapCommand::NativeRunCommand(UConsoleContext* InConsole, TArray<FString> InArguments)
{
    this->Complete();
}