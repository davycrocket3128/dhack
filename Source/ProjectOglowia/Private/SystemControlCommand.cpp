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

#include "SystemControlCommand.h"
#include "PlatformMisc.h"
#include "UserContext.h"
#include "PeacenetWorldStateActor.h"
#include "SystemUpgrade.h"
#include "CommonUtils.h"

// Command name: systemctl
// Description: Allows the player to poweroff or reboot the system, as well as manage system daemons.
//
// Usage:
//  systemctl poweroff - shut down the system.
//  systemctl reboot - reboot the system.
//  systemctl enable <service> - enable the specified system daemon.
//  systemctl disable <service> - disable the specified system daemon.
//  systemctl list-units - lists all registered system daemons.
//  systemctl start <service>  - starts the specified system daemon.
//  systemctl restart <service>  - restarts the specified system daemon.
//  systemctl stop <service>  - stops the specified system daemon.
void ASystemControlCommand::NativeRunCommand(UConsoleContext* InConsole, TArray<FString> InArguments)
{
    if(this->ArgumentMap["poweroff"]->AsBoolean())
    {
        InConsole->WriteLine(NSLOCTEXT("System", "Goodbye", "Goodbye."));
        FPlatformMisc::RequestExit(false);
        this->Complete();
        return;
    }
    else if(this->ArgumentMap["reboot"]->AsBoolean())
    {
        this->GetUserContext()->GetPeacenet()->QuitGame();
        return;
    }
    this->Complete();
}