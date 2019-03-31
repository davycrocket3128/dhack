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

#include "MissionActor.h"
#include "PeacenetWorldStateActor.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/SaveGame.h"

AMissionActor::AMissionActor()
{
    // Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

void AMissionActor::Setup(APeacenetWorldStateActor* InPeacenet, UMissionAsset* InMission)
{
    check(InPeacenet);
    check(InMission);

    // Assign our mission data.
    this->Peacenet = InPeacenet;
    this->Mission = InMission;

    // Back up the save file.
    UGameplayStatics::SaveGameToSlot(this->Peacenet->SaveGame, TEXT("PeacegateOS_PreMission"), 0);
}

void AMissionActor::Abort()
{
    // Restore the save file.
    this->Peacenet->SaveGame = Cast<UPeacenetSaveGame>(UGameplayStatics::LoadGameFromSlot("PeacegateOS_PreMission", 0));

    // Delete the backup.
    UGameplayStatics::DeleteGameInSlot("PeacegateOS_PreMission", 0);

    // Delete objective backup if it exists.
    if(UGameplayStatics::DoesSaveGameExist("PeacegateOS_PreMission", 1))
    {
        UGameplayStatics::DeleteGameInSlot("PeacegateOS_PreMission", 1);
    }

    // Destroy ourselves.
    this->Peacenet = nullptr;
    this->Mission = nullptr;

    this->Destroy();
}

void AMissionActor::Tick(float InDeltaSeconds)
{
    if(!this->Peacenet) return;
    if(!this->Mission) return;

    Super::Tick(InDeltaSeconds);
}