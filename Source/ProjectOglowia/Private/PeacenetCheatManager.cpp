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
#include "PeacenetCheatManager.h"

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