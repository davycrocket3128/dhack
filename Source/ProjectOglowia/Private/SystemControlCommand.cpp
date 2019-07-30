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
// Description: Allows the player to enable/disable System Upgrades, as well as poweroff or reboot the system.
//
// Usage:
//  systemctl poweroff - shut down the system.
//  systemctl reboot - reboot the system.
//  systemctl enable <upgrade> - enable the specified <upgrade> if it is enable-able.
//  systemctl disable <upgrade> - disable the specified upgrade if it is enabled and does not have any currently-enabled dependent upgrades.
//  systemctl list-upgrades - lists all installed, available and future upgrades.
//
// Example:
// [user@system ~]$ systemctl enable test
//
// {
//   "poweroff": false
//   "reboot": "false",
//    "enable": true,
//    "disable": false,
//   "<upgrade>": "test",
//   "list-upgrades": false   
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

        for(auto Upgrade : this->GetUserContext()->GetPeacenet()->GetAllSystemUpgrades())
        {
            if(Upgrade->ID.ToString() == upgradeId)
            {
                if(Upgrade->IsUnlocked(this->GetUserContext()))
                {
                    if(!Upgrade->CanUserUnlock)
                    {
                        InConsole->SetBold(true);
                        InConsole->SetForegroundColor(EConsoleColor::Red);
                        InConsole->WriteLine(NSLOCTEXT("SystemCtl", "StoryUpgrade", "Error: You cannot disable upgrades that were given to you during a mission."));
                        InConsole->ResetFormatting();
                        this->Complete();
                        return;
                    }

                    // This for loop effectively makes the disable command OwO(n^2).
                    for(auto Dep : this->GetUserContext()->GetPeacenet()->GetAllSystemUpgrades())
                    {
                        if(Dep == Upgrade) continue;

                        if(UCommonUtils::UpgradeDependsOn(this->GetUserContext(), Dep, Upgrade))
                        {
                            if(Dep->IsUnlocked(this->GetUserContext()))
                            {
                                InConsole->SetBold(true);
                                InConsole->SetForegroundColor(EConsoleColor::Red);
                                InConsole->WriteLine(NSLOCTEXT("SystemCtl", "UpgradeNeeded", "Error: This upgrade is required by another upgrade that is currently enabled.  You cannot therefore disable this upgrade."));
                                InConsole->ResetFormatting();
                                this->Complete();
                                return;
                            }
                        }
                    }

                    this->GetUserContext()->GetPeacenetIdentity().UnlockedUpgrades.Remove(Upgrade);

                    InConsole->SetForegroundColor(EConsoleColor::Green);
                    InConsole->WriteLine(NSLOCTEXT("SystemCtl", "UpgradeDisabled", "Success! Upgrade has been disabled."));
                    InConsole->ResetFormatting();
                    this->Complete();
                    return;
                }

                InConsole->SetBold(true);
                InConsole->SetForegroundColor(EConsoleColor::Red);
                InConsole->WriteLine(NSLOCTEXT("SystemCtl", "UpgradeNotActive", "Error: That upgrade is not currently active."));
                InConsole->ResetFormatting();
                this->Complete();
                break;
            }
        }

        InConsole->SetBold(true);
        InConsole->SetForegroundColor(EConsoleColor::Red);
        InConsole->WriteLine(NSLOCTEXT("SystemCtl", "UpgradeNotFound", "Error: The upgrade you specified does not exist."));
        InConsole->ResetFormatting();
        this->Complete();
        return;
    }
    else if(this->ArgumentMap["list-upgrades"]->AsBoolean())
    {
        TArray<USystemUpgrade*> Installed;
        TArray<USystemUpgrade*> Available;
        
        for(auto Upgrade : this->GetUserContext()->GetPeacenet()->GetAllSystemUpgrades())
        {
            if(Upgrade->IsUnlocked(this->GetUserContext()))
            {
                Installed.Add(Upgrade);
            }
            else if(Upgrade->UpgradeIsUnlockable(this->GetUserContext()))
            {
                Available.Add(Upgrade);
            }
        }

        InConsole->SetBold(true);
        InConsole->SetUnderline(true);
        InConsole->SetForegroundColor(EConsoleColor::Yellow);
        InConsole->WriteLine(NSLOCTEXT("SystemCtl", "InstalledUpgrades", "INSTALLED UPGRADES"));
        InConsole->ResetFormatting();
        InConsole->WriteLine(FText::GetEmpty());

        for(auto Upgrade : Installed)
        {
            InConsole->Write(FText::FromString(" - "));
            InConsole->SetForegroundColor(EConsoleColor::Green);
            InConsole->WriteLine(FText::FromName(Upgrade->ID));
            InConsole->ResetFormatting();
            
        }

        InConsole->WriteLine(FText::GetEmpty());

        InConsole->SetBold(true);
        InConsole->SetUnderline(true);
        InConsole->SetForegroundColor(EConsoleColor::Yellow);
        InConsole->WriteLine(NSLOCTEXT("SystemCtl", "AvailableUpgrades", "AVAILABLE UPGRADES"));
        InConsole->ResetFormatting();
        InConsole->WriteLine(FText::GetEmpty());

        for(auto Upgrade : Installed)
        {
            InConsole->Write(FText::FromString(" - "));
            InConsole->SetForegroundColor(EConsoleColor::Green);
            InConsole->Write(FText::FromName(Upgrade->ID));
            InConsole->ResetFormatting();
            InConsole->Write(FText::FromString(": "));
            InConsole->WriteLine(FText::Format(NSLOCTEXT("SystemCtl", "UpgradeSkill", "{0} XP"), FText::AsNumber(Upgrade->RequiredSkillPoints)));
        }

        InConsole->WriteEmptyLine();
    }
    this->Complete();
}