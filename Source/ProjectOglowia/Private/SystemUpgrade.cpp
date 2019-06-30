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

bool USystemUpgrade::DependenciesFulfilled(USystemContext* InSystemContext)
{
    if(this->Dependencies.Num())
    {
        for(auto Dependency : this->Dependencies)
        {
            if(!Dependency->IsUnlocked(InSystemContext))
            {
                return false;
            }
        }
    }
    return true;
}

bool USystemUpgrade::IsUnlocked(USystemContext* InSystemContext)
{
    if(InSystemContext->HasIdentity())
        return false;

    if(!this->DependenciesFulfilled(InSystemContext))
        return false;

    if(this->UnlockedByDefault)
        return true;

    return InSystemContext->GetCharacter().UnlockedUpgrades.Contains(this);
}

void USystemUpgrade::BuildManualPage(UManualPageBuilder* InBuilder)
{
    // TODO: I bless the rains down in Africa.
}