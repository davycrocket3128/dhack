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


#include "MapNodeContext.h"
#include "PeacenetWorldStateActor.h"
#include "PeacenetSaveGame.h"
#include "UserContext.h"
#include "MapWidget.h"

bool UMapNodeContext::IsMissionImportant()
{
    return this->GetIdentity().IsMissionImportant;
}

UPeacenetSaveGame* UMapNodeContext::GetSaveGame()
{
    return this->MapWidget->GetUserContext()->GetPeacenet()->SaveGame;
}

FString UMapNodeContext::GetEmail()
{
    return this->GetIdentity().EmailAddress;
}

int UMapNodeContext::GetSkill()
{
    return this->GetIdentity().Skill;
}

float UMapNodeContext::GetReputation()
{
    return this->GetIdentity().Reputation;
}

FString UMapNodeContext::GetIPAddress()
{
    FComputer Computer;
    int ComputerIndex;
    bool result = this->GetSaveGame()->GetComputerByID(this->GetIdentity().ComputerID, Computer, ComputerIndex);
    
    if(!result)
        return "<error>";

    return this->MapWidget->GetUserContext()->GetPeacenet()->GetIPAddress(Computer);
}

int UMapNodeContext::GetNodeID()
{
    return this->NodeID;
}

FString UMapNodeContext::CreateBooleanName(FString InExtension)
{
    return "entity." + FString::FromInt(this->NodeID) + ".node." + InExtension;
}

FString UMapNodeContext::GetNodeName()
{
    if(this->GetIdentity().PreferredAlias.Len())
        return this->GetIdentity().PreferredAlias;
    return this->GetIdentity().CharacterName;
}

bool UMapNodeContext::IsHighlighted()
{
    return this->Highlighted;
}

void UMapNodeContext::Highlight()
{
    this->Highlighted = true;
}

void UMapNodeContext::Unhighlight()
{
    this->Highlighted = false;
}


FVector2D UMapNodeContext::GetPosition()
{
    FVector2D Ret;
    bool result = this->GetSaveGame()->GetPosition(this->NodeID, Ret);
    check(result);
    return Ret;
}

void UMapNodeContext::Setup(UMapWidget* InMapWidget, int InNodeID)
{
    check(InMapWidget);

    this->MapWidget = InMapWidget;
    this->NodeID = InNodeID;
}

FPeacenetIdentity& UMapNodeContext::GetIdentity()
{
    int IdentityIndex;
    FPeacenetIdentity Identity;
    bool result = this->MapWidget->GetUserContext()->GetPeacenet()->SaveGame->GetCharacterByID(this->NodeID, Identity, IdentityIndex);
    check(result);
    return this->MapWidget->GetUserContext()->GetPeacenet()->SaveGame->Characters[IdentityIndex];
}