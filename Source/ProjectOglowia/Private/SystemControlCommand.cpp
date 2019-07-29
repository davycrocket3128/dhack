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

// Command name: systemctl
// Description: Allows the player to enable/disable System Upgrades, as well as poweroff or reboot the system.
//
// Usage:
//  systemctl poweroff - shut down the system.
//  systemctl reboot - reboot the system.
//  systemctl enable <upgrade> - enable the specified <upgrade> if it is enable-able.
//  systemctl disable <upgrade> - disable the specified upgrade if it is enabled and does not have any currently-enabled dependent upgrades.
//
// Example:
// [user@system ~]$ systemctl enable test
//
// {
//   "poweroff": false
//   "reboot": "false",
//    "enable": true,
//    "disable": false,
//   "<upgrade>": "test"   
// }
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
    else if(this->ArgumentMap["enable"]->AsBoolean())
    {
        if(!this->GetUserContext()->HasIdentity())
        {
            InConsole->SetBold(true);
            InConsole->SetForegroundColor(EConsoleColor::Red);
            InConsole->WriteLine(NSLOCTEXT("SystemCtl", "IdentityNeeded", "Error: This operation requires a Peacenet Identity.  Have you tried running 'identity -c' to create one?"));
            InConsole->ResetFormatting();

            this->Complete();
            return;
        }

        FString upgradeId = this->ArgumentMap["<upgrade>"]->AsString();

        for(auto Upgrade : this->GetUserContext()->GetPeacenet()->GetAllSystemUpgrades())
        {
            if(Upgrade->ID.ToString() == upgradeId)
            {
                if(Upgrade->IsUnlocked(this->GetUserContext()))
                {
                    InConsole->SetBold(true);
                    InConsole->SetForegroundColor(EConsoleColor::Red);
                    InConsole->WriteLine(NSLOCTEXT("SystemCtl", "UpgradeInstalledAlready", "Error: The specified upgrade is already enabled."));
                    InConsole->ResetFormatting();
                    this->Complete();
                    return;
                }

                if(Upgrade->UpgradeIsUnlockable(this->GetUserContext()))
                {
                    Upgrade->TriggerUnlock(this->GetUserContext());
                }
                else 
                {
                    InConsole->SetBold(true);
                    InConsole->SetForegroundColor(EConsoleColor::Red);
                    InConsole->WriteLine(NSLOCTEXT("SystemCtl", "UpgradeUnavailable", "Error: This upgrade is not currently available."));
                    InConsole->ResetFormatting();
                }

                this->Complete();
                return;
            }
        }

        InConsole->SetBold(true);
        InConsole->SetForegroundColor(EConsoleColor::Red);
        InConsole->WriteLine(NSLOCTEXT("SystemCtl", "UpgradeNotFound", "Error: The upgrade you specified does not exist."));
        InConsole->ResetFormatting();
        this->Complete();
        return;
    }
    else if(this->ArgumentMap["disable"]->AsBoolean())
    {
        if(!this->GetUserContext()->HasIdentity())
        {
            InConsole->SetBold(true);
            InConsole->SetForegroundColor(EConsoleColor::Red);
            InConsole->WriteLine(NSLOCTEXT("SystemCtl", "IdentityNeeded", "Error: This operation requires a Peacenet Identity.  Have you tried running 'identity -c' to create one?"));
            InConsole->ResetFormatting();

            this->Complete();
            return;
        }

        FString upgradeId = this->ArgumentMap["<upgrade>"]->AsString();

        // Disable that upgrade.
    }
    this->Complete();
}