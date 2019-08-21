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

#include "SystemUpgrade.h"
#include "SystemContext.h"
#include "UserContext.h"

bool USystemUpgrade::ShouldShowUpgradeInUserInterface(UUserContext* InPeacegateUser) {
    if(this->UnlockedByDefault) {
        return false;
    }

    if(this->IsUnlocked(InPeacegateUser) || this->DependenciesFulfilled(InPeacegateUser)) {
        return true;
    }

    return false;
}

void USystemUpgrade::TriggerUnlock(UUserContext* InUserContext) {
    if(!this->IsUnlocked(InUserContext)) {
        if(InUserContext->HasIdentity()) {
            InUserContext->GetPeacenetIdentity().UnlockedUpgrades.Add(this);
        }
    }
}

bool USystemUpgrade::UpgradeIsUnlockable(UUserContext* InPeacegateUser) {
    if(!this->CanUserUnlock) {
        return false;
    }

    if(!this->IsUnlocked(InPeacegateUser)) {
        return false;
    }

    if(this->IsUnlocked(InPeacegateUser)) {
        return false;
    }

    return true;
}

bool USystemUpgrade::DependenciesFulfilled(UUserContext* InUserContext) {
    if(this->Dependencies.Num()) {
        for(auto Dependency : this->Dependencies) {
            if(!Dependency->IsUnlocked(InUserContext)) {
                return false;
            }
        }
    }
    return true;
}

bool USystemUpgrade::IsUnlocked(UUserContext* InUserContext) {
    if(!InUserContext->HasIdentity()) {
        return false;
    }

    if(!this->DependenciesFulfilled(InUserContext)) {
        return false;
    }

    if(this->UnlockedByDefault) {
        return true;
    }

    return InUserContext->GetPeacenetIdentity().UnlockedUpgrades.Contains(this);
}

void USystemUpgrade::BuildManualPage(UManualPageBuilder* InBuilder) {
    // TODO: I bless the rains down in Africa.
}