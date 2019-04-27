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

#include "AlertManager.h"
#include "PeacenetWorldStateActor.h"

AAlertManager::AAlertManager()
{
     	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

void AAlertManager::ResetStealthIncreaseTimer(int EntityID)
{
    if(this->GetStealthStatus(EntityID).IsInAlert) return;

    if(this->GetStealthStatus(EntityID).IsSuspicious)
    {
        this->GetStealthStatus(EntityID).TimeUntilStealthIncrease = this->SuspiciousStealthIncreaseInterval;
    }
    else
    {
        this->GetStealthStatus(EntityID).TimeUntilStealthIncrease = this->StealthIncreaseInterval;
    }
}

void AAlertManager::Setup(APeacenetWorldStateActor* InPeacenet)
{
    this->Peacenet = InPeacenet;
}

FStealthStatus& AAlertManager::GetStealthStatus(int InEntityID)
{
    for(int i = 0; i < this->StealthStatuses.Num(); i++)
    {
        if(this->StealthStatuses[i].EntityID == InEntityID)
        {
            return this->StealthStatuses[i];
        }
    }

    FStealthStatus NewStatus;
    NewStatus.EntityID = InEntityID;

    this->StealthStatuses.Add(NewStatus);

    return this->StealthStatuses[this->StealthStatuses.Num() - 1];
}

void AAlertManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    for(auto& StealthStatus : this->StealthStatuses)
    {
        if(StealthStatus.IsInAlert) continue;

        float stealthiness = StealthStatus.Stealthiness;

        if(stealthiness >= 1.f) continue;

        if(StealthStatus.Cooldown >= 0.f)
        {
            StealthStatus.Cooldown -= DeltaTime;
            continue;
        }

        StealthStatus.TimeUntilStealthIncrease -= DeltaTime;

        if(StealthStatus.TimeUntilStealthIncrease <= 0.f)
        {
            this->ResetStealthIncreaseTimer(StealthStatus.EntityID);
            stealthiness += 0.01f;
            StealthStatus.Stealthiness = FMath::Clamp(stealthiness, 0.f, 1.f);

            if(stealthiness >= 0.70f)
            {
                StealthStatus.IsSuspicious = false;
            }
        }

    }
}