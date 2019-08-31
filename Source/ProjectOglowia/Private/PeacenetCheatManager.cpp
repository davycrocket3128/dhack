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

#include "CommonUtils.h"
#include "PeacenetWorldStateActor.h"
#include "UserContext.h"
#include "Kismet/KismetSystemLibrary.h"
#include "LootableFile.h"
#include "Process.h"
#include "FileRecordUtils.h"
#include "PeacenetCheatManager.h"

void UPeacenetCheatManager::DestroyDebugConsoleContext() {
    this->ConsoleContext = nullptr;
    this->Master = nullptr;
    this->Slave = nullptr;
}

void UPeacenetCheatManager::CreateDebugConsoleContext() {
    if(!this->ConsoleContext) {
    // Set up the terminal options
        FPtyOptions options;
        options.LFlag = ICANON;
        options.C_cc[VERASE] = '\b';
        options.C_cc[VEOL] = '\r';
        options.C_cc[VEOL2] = '\n';

        // Create the pseudo terminal streams.
        UPtyStream::CreatePty(this->Master, this->Slave, options);

        // Create and initialize the console context.
        this->ConsoleContext = NewObject<UConsoleContext>(this);
        this->ConsoleContext->Setup(this->Master, this->GetPlayerUser());

        // The UE4 console does *NOT* support ANSI, and has line-editing built in.
        this->ConsoleContext->AllowANSI = false;
        this->ConsoleContext->AllowLineEditing = false;

        // Make sure we can read/write.
        TScriptDelegate<> WriteEvent;
        WriteEvent.BindUFunction(this, "HandleWrite");
        this->Master->OnWritten.Add(WriteEvent);
        this->Slave->OnWritten.Add(WriteEvent);
    }
}

void UPeacenetCheatManager::HandleWrite() {
    TCHAR c;
    while(this->Slave->ReadChar(c)) {
        if(this->Buffer.EndsWith("\n")) {
            this->PrintMessage(this->Buffer.TrimEnd());
            this->Buffer = "";
        }
        this->Buffer += c;
    }
}

UUserContext* UPeacenetCheatManager::GetPlayerUser() {
    UUserContext* User;
    UCommonUtils::GetPlayerUserContext(this->GetOuterAPlayerController(), User);
    return User;
}

APeacenetWorldStateActor* UPeacenetCheatManager::GetPeacenet() {
    // Acquire a world context through our owning player.
    APlayerController* Player = this->GetOuterAPlayerController();

    // Try to acquire the player's Peacegate user.
    UUserContext* User = nullptr;
    if(UCommonUtils::GetPlayerUserContext(Player, User)) {
        return User->GetPeacenet();
    } else {
        return nullptr;
    }
}

void UPeacenetCheatManager::PrintMessage(FString Message) {
    UKismetSystemLibrary::PrintString(this->GetOuterAPlayerController(), Message, true, true, FLinearColor(0x1B, 0xAA, 0xF7, 0xFF), 5.f);
}

void UPeacenetCheatManager::PeacenetStat() {
    if(this->GetPeacenet()) {
        this->PrintMessage("Peacenet is currently active and running all simulations.");
    } else {
        this->PrintMessage("Peacenet is currently inactive and not running any simulations.");
    }
}

void UPeacenetCheatManager::DnsTable() {
    auto Peacenet = this->GetPeacenet();
    if (Peacenet) {
        this->PrintMessage("IP Address to Computer ID Map\r\n===============\r\n\r\n");
        for(auto IPMap : Peacenet->SaveGame->ComputerIPMap) {
            this->PrintMessage(IPMap.Key + " => " + FString::FromInt(IPMap.Value));
        }
        this->PrintMessage("Domain Name Map\r\n===============\r\n\r\n");
        for(auto DnsMap : Peacenet->SaveGame->DomainNameMap) {
            this->PrintMessage(DnsMap.Key + " => " + DnsMap.Value);
        }
    } else {
        this->PrintMessage("Peacenet isn't currently active, can't see DNS table.");
    }
}

void UPeacenetCheatManager::Missions() {
    if(this->GetPeacenet()) {
        for(auto Mission : this->GetPeacenet()->Missions) {
            this->PrintMessage(Mission->GetFName().ToString());
        }
    }
    this->PrintMessage("\r\n >> Use 'UnlockMission <Mission>' to unlock a mission from above.");
}

void UPeacenetCheatManager::UnlockMission(FString MissionName) {
    
}

void UPeacenetCheatManager::Upgrades() {
    if(this->GetPeacenet()) {
        for(auto Upgrade : this->GetPeacenet()->GetAllSystemUpgrades()) {
            this->PrintMessage(Upgrade->GetFName().ToString());
        }
    }
    this->PrintMessage("\r\n >> Use 'UnlockUpgrade <Upgrade>' or 'LockUpgrade <Upgrade>' to unlock an upgrade above.");
}

void UPeacenetCheatManager::UnlockUpgrade(FString Upgrade) {
    if(this->GetPlayerUser()) {
        for(auto& UpgradeAsset : this->GetPeacenet()->GetAllSystemUpgrades()) {
            if(UpgradeAsset->GetFName().ToString() == Upgrade) {
                if(!this->GetPlayerUser()->GetPeacenetIdentity().UnlockedUpgrades.Contains(UpgradeAsset)) {
                    this->GetPlayerUser()->GetPeacenetIdentity().UnlockedUpgrades.Add(UpgradeAsset);
                    this->PrintMessage("Unlocked upgrade successfully.");
                    return;
                }
            }
        }
    }
}

void UPeacenetCheatManager::ComputerInfo(int EntityID) {
    if(this->GetPeacenet()) {
        for(auto& Computer : this->GetPeacenet()->SaveGame->Computers) {
            if(Computer.ID == EntityID) {
                this->PrintMessage("Folders in FS:");
                this->PrintMessage(" >> " + FString::FromInt(Computer.Filesystem.Num()));
                this->PrintMessage("\r\nFile records in FS:");
                this->PrintMessage(" >> " + FString::FromInt(Computer.FileRecords.Num()));
                this->PrintMessage("\r\nText file records:");
                this->PrintMessage(" >> " + FString::FromInt(Computer.TextFiles.Num()));
                this->PrintMessage("\r\nFirewall rules:");
                for(auto& Rule : Computer.FirewallRules) {
                    this->PrintMessage(" >> Port: " + FString::FromInt(Rule.Port) + ", Crashed: " + ((Rule.IsCrashed) ? "True" : "False") + ", FW Filtered: " + ((Rule.IsFiltered) ? "True" : "False"));
                }
                this->PrintMessage("\r\nOwning Identity:");
                if(Computer.SystemIdentity == -1) {
                    this->PrintMessage(" >> None");
                } else {
                    this->PrintMessage(" >> " + FString::FromInt(Computer.SystemIdentity));
                }
                this->PrintMessage("\r\nUsers");
                for(auto& User : Computer.Users) {
                    this->PrintMessage(" >> " + FString::FromInt(User.ID) + ": " + User.Username);
                }
                this->PrintMessage("\r\nComputer type:");
                switch(Computer.ComputerType) {
                    case EComputerType::EmailServer:
                        this->PrintMessage(" >> Email server");
                        break;
                    case EComputerType::Personal:
                        this->PrintMessage("General-ourpose Peacegate instance/personal computer");
                        break;
                    default:
                        this->PrintMessage(" >> Unknown/unused computer type");
                        break;
                }
                return;
            }
        }
        this->PrintMessage("Computer not found.");
    }
}

void UPeacenetCheatManager::IdentityInfo(int EntityID) {
    if(this->GetPeacenet()) {
        for(auto& Identity : this->GetPeacenet()->SaveGame->Characters) {
            if(Identity.ID == EntityID) {
                this->PrintMessage("Name:");
                this->PrintMessage(" >> " + Identity.CharacterName);
                this->PrintMessage("\r\nPreferred Alias:");
                this->PrintMessage(" >> " + Identity.PreferredAlias);
                this->PrintMessage("\r\nSkill:");
                this->PrintMessage(" >> " + FString::FromInt(Identity.Skill));
                this->PrintMessage("\r\nEmail User:");
                this->PrintMessage(" >> " + Identity.EmailAddress);
                this->PrintMessage("\r\nIs Mission-Important:");
                if(Identity.IsMissionImportant) {
                    this->PrintMessage(" >> Yes");
                } else {
                    this->PrintMessage(" >> No");
                }
                this->PrintMessage("\r\nIdentity Type:");
                switch(Identity.CharacterType) {
                    case EIdentityType::Player:
                        this->PrintMessage(" >> Player");
                        break;
                    case EIdentityType::NonPlayer:
                        this->PrintMessage(" >> Procedurally generated NPC");
                        break;
                    case EIdentityType::Story:
                        this->PrintMessage(" >> Story-based NPC");
                        break;
                }
                
                
                return;
            }
        }
        this->PrintMessage("Identity not found.");
    }
}

void UPeacenetCheatManager::ResetFirewall(int EntityID) {
    if(this->GetPeacenet()) {
        for(auto& Computer : this->GetPeacenet()->SaveGame->Computers) {
            if(Computer.ID == EntityID) {
                Computer.FirewallRules.Empty();
                this->PrintMessage("This computer's Firewall will now be re-generated next time you nmap it.");
                return;
            }
        }
        this->PrintMessage("Computer wasn't found by entity ID in the current save file.");
    }
}

void UPeacenetCheatManager::OpenPort(int EntityID, int Port) {
    if(this->GetPeacenet()) {
        for(auto& Computer : this->GetPeacenet()->SaveGame->Computers) {
            if(Computer.ID == EntityID) {
                for(auto& Rule : Computer.FirewallRules) {
                    if(Rule.Port == Port) {
                        Rule.IsFiltered = false;
                        this->PrintMessage("Port is no longer filtered!");
                        return;
                    }
                }
                this->PrintMessage("Port is not listening, cannot apply this cheat to it.");
                return;
            }
        }
        this->PrintMessage("Computer wasn't found by entity ID in the current save file.");
    }
}

void UPeacenetCheatManager::CompleteMission() {

}

void UPeacenetCheatManager::EndMission() {

}

void UPeacenetCheatManager::Panic() {

}

void UPeacenetCheatManager::ForceTextMode() {

}

void UPeacenetCheatManager::UnforceTextMode() {

}

void UPeacenetCheatManager::SetCover(float Percentage) {

}

void UPeacenetCheatManager::Uncrash(int EntityID) {
    if(this->GetPeacenet()) {
        for(auto& Computer : this->GetPeacenet()->SaveGame->Computers) {
            if(Computer.ID == EntityID) {
                Computer.Crashed = false;
                this->PrintMessage("Computer is no longer crashed and should be hackable again.");
                return;
            }
        }
        this->PrintMessage("Computer wasn't found by entity ID in the current save file.");
    }
}

void UPeacenetCheatManager::UncrashService(int EntityID, int Port) {
    if(this->GetPeacenet()) {
        for(auto& Computer : this->GetPeacenet()->SaveGame->Computers) {
            if(Computer.ID == EntityID) {
                for(auto& Rule : Computer.FirewallRules) {
                    if(Rule.Port == Port) {
                        Rule.IsCrashed = false;
                        this->PrintMessage("Service on specified port is no longer crashed, and should be hackable again!");
                        return;
                    }
                }
                this->PrintMessage("Port isn't listening, cannot use this cheat right now.");
                return;
            }
        }
        this->PrintMessage("Computer wasn't found by entity ID in the current save file.");
    }
}

void UPeacenetCheatManager::LockUpgrade(FString Upgrade) {
    if(this->GetPlayerUser()) {
        for(int i = 0; i < this->GetPlayerUser()->GetPeacenetIdentity().UnlockedUpgrades.Num(); i++) {
            USystemUpgrade* UpgradeAsset = this->GetPlayerUser()->GetPeacenetIdentity().UnlockedUpgrades[i];
            if(UpgradeAsset->GetFName().ToString() == Upgrade) {
                this->GetPlayerUser()->GetPeacenetIdentity().UnlockedUpgrades.RemoveAt(i);
                this->PrintMessage("Locked upgrade successfully.");
                return;
            }
        }
    }
}

void UPeacenetCheatManager::SetSkill(int Skill) {
    if(this->GetPlayerUser()) {
        this->GetPlayerUser()->GetPeacenetIdentity().Skill = Skill;
        this->PrintMessage("Set player skill points to " + FString::FromInt(Skill));
    }
}

void UPeacenetCheatManager::AddSkill(int Skill) {
    if(this->GetPlayerUser()) {
        this->GetPlayerUser()->GetPeacenetIdentity().Skill += Skill;
        this->PrintMessage("Added " + FString::FromInt(Skill) + " to player skill points.");
    }
}

void UPeacenetCheatManager::RemoveSkill(int Skill) {
    if(this->GetPlayerUser()) {
        this->GetPlayerUser()->GetPeacenetIdentity().Skill -= Skill;
        if(this->GetPlayerUser()->GetPeacenetIdentity().Skill < 0) {
            this->GetPlayerUser()->GetPeacenetIdentity().Skill = 0;
        }
        this->PrintMessage("Removed " + FString::FromInt(Skill) + " from player skill points.");
    }
}

void UPeacenetCheatManager::Lootables() {
    if(this->GetPeacenet()) {
        TArray<ULootableFile*> LootableFiles;
        this->GetPeacenet()->LoadAssets<ULootableFile>("LootableFile", LootableFiles);
        for(auto Lootable : LootableFiles) {
            this->PrintMessage(Lootable->GetFName().ToString());
        }
    }
}

void UPeacenetCheatManager::DropLootable(int EntityID, FString Lootable) {
    if(this->GetPeacenet()) {
        TArray<ULootableFile*> LootableFiles;
        this->GetPeacenet()->LoadAssets<ULootableFile>("LootableFile", LootableFiles);
        for(auto LootableAsset : LootableFiles) {
            if(LootableAsset->GetFName().ToString() == Lootable) {
                for(FComputer& Computer : this->GetPeacenet()->SaveGame->Computers) {
                    if(Computer.ID == EntityID) {
                        USystemContext* SystemContext = this->GetPeacenet()->GetSystemContext(Computer.ID);
                        UPeacegateFileSystem* FileSystem = SystemContext->GetFilesystem(0);
                        LootableAsset->Spawn(FileSystem, this->GetPeacenet()->GetWorldGeneratorRng());
                        this->PrintMessage("Lootable successfully dropped into filesystem.");
                        return;
                    }
                    this->PrintMessage("Computer entity not found.");
                }
                return;
            }
        }
        this->PrintMessage("Lootable file data not found.");
    }
}

void UPeacenetCheatManager::DropLootablePlayer(FString Lootable) {
    if(this->GetPeacenet()) {
        this->DropLootable(this->GetPeacenet()->SaveGame->PlayerComputerID, Lootable);
    }
}

void UPeacenetCheatManager::ForceRegenerateRandomLootables(int EntityID) {
    if(this->GetPeacenet()) {
        for(auto& Computer : this->GetPeacenet()->SaveGame->Computers) {
            if(Computer.ID == EntityID) {
                this->PrintMessage("Random lootable spawn state reset for this computer.");
                this->PrintMessage(" >> Next time the computer is DNS-resolved in-game (nmap, gsfconsole, etc), new random lootables will be generated.");
                Computer.SpawnedRandomLootables = false;
                return;
            }
        }
        this->PrintMessage("Computer not found.");
    }
}

void UPeacenetCheatManager::ExecBinary(FString Path) {
    if(this->GetPlayerUser()) {
        this->CreateDebugConsoleContext();
        UProcess* Process = nullptr;
        if(UFileRecordUtils::LaunchProcess(Path, TArray<FString>{ Path }, this->ConsoleContext, nullptr, Process)) {
            TScriptDelegate<> Killed;
            Killed.BindUFunction(this, "DestroyDebugConsoleContext");
            Process->OnKilled.Add(Killed);
        } else {
            this->PrintMessage("Binary not found.");
        }
    }
}

void UPeacenetCheatManager::DnsResolve(FString Host) {
    if(this->GetPeacenet()) {
        FComputer Computer;
        EConnectionError Error = EConnectionError::None;
        if(this->GetPeacenet()->DnsResolve(Host, Computer, Error)) {
            this->PrintMessage(FString::FromInt(Computer.ID));
        } else {
            switch(Error) {
                case EConnectionError::HostNotFound:
                    this->PrintMessage("Host not found.");
                    break;
                case EConnectionError::ConnectionTimedOut:
                    this->PrintMessage("Connection time-out.");
                    break;
                case EConnectionError::ConnectionRefused:
                    this->PrintMessage("Connection refused.");
                    break;
                default:
                    this->PrintMessage("Unknown DNS resolution result.");
                    break;
            }
        }
    }
}

void UPeacenetCheatManager::StdIn(FString Text) {
    if(this->Slave) {
        for(TCHAR c : Text) {
            this->Slave->WriteChar(c);
        }
        this->Slave->WriteChar('\r');
        this->Slave->WriteChar('\n');
    } else {
        this->PrintMessage("!!! ERROR !!! You shouldn't be using this command right now.  This command is meant to be used in conjunction with the Peacenet 'ExecBinary' command, to allow you to write text into the standard input stream of a Peacenet binary's console while said binary is running inside the Unreal console.  So, instead, try:");
        this->PrintMessage(" >> ExecBinary /bin/bash");
        this->PrintMessage(" >> StdIn " + Text);
    }
}