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

void AMissionActor::Advance()
{
    // Advance the current task pointer so the next task
    // begins.
    this->CurrentTask++;
    
    // Did we go past the total number of tasks? If so,
    // the mission is completed.
    if(this->CurrentTask >= this->LoadedTasks.Num())
    {
        this->Complete();
        return;
    }

    // If we get this far then we can actually continue.


    // First we check if the new task is a checkpoint, and if so
    // we create a save state and allow the player to restore
    // from this point in the mission.
    if(this->LoadedTasks[this->CurrentTask].IsCheckpoint)
    {
        this->SetCheckpoint();
    }

    // Now we can tell the new task that it has started.
    this->LoadedTasks[this->CurrentTask].Task->Start(this);
}

void AMissionActor::SetCheckpoint()
{
    // Set the checkpoint task.
    this->CheckpointTask = this->CurrentTask;

    // Store the save file as a save state.
    UGameplayStatics::SaveGameToSlot(this->Peacenet->SaveGame, "PeacegateOS_PreMission", 1);
}

void AMissionActor::Complete()
{
    // Delete the save states.
    this->DeleteSaveStates();

    // Adds the mission to the list of completed missions.
    // If it isn't in there already.
    if(!this->Peacenet->SaveGame->CompletedMissions.Contains(this->Mission))
        this->Peacenet->SaveGame->CompletedMissions.Add(this->Mission);

    // Tell The Peacenet that we've completed.
    this->Peacenet->EndMission();

    // Clean up and despawn.
    this->Peacenet = nullptr;
    this->Mission=  nullptr;

    this->Destroy();
}

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

    // Start the mission task pointer at the beginning, then load
    //  all valid tasks.
    this->CurrentTask = -1;

    this->LoadedTasks.Empty();
    for(auto Task : this->Mission->Tasks)
    {
        if(!Task.Task) continue;

        this->LoadedTasks.Add(Task);
    }

    // Back up the save file.
    UGameplayStatics::SaveGameToSlot(this->Peacenet->SaveGame, TEXT("PeacegateOS_PreMission"), 0);
}

void AMissionActor::DeleteSaveStates()
{
        // Delete the backup.
    UGameplayStatics::DeleteGameInSlot("PeacegateOS_PreMission", 0);

    // Delete objective backup if it exists.
    if(UGameplayStatics::DoesSaveGameExist("PeacegateOS_PreMission", 1))
    {
        UGameplayStatics::DeleteGameInSlot("PeacegateOS_PreMission", 1);
    }
}

void AMissionActor::Abort()
{
    // Restore the save file.
    this->Peacenet->SaveGame = Cast<UPeacenetSaveGame>(UGameplayStatics::LoadGameFromSlot("PeacegateOS_PreMission", 0));

    // Delete save states.
    this->DeleteSaveStates();

    // Destroy ourselves.
    this->Peacenet = nullptr;
    this->Mission = nullptr;

    this->Destroy();
}

void AMissionActor::SendGameEvent(FString EventName, TMap<FString, FString> InEventData)
{
    this->LoadedTasks[this->CurrentTask].Task->HandleEvent(EventName, InEventData);
}

APeacenetWorldStateActor* AMissionActor::GetPeacenet()
{
    return this->Peacenet;
}

void AMissionActor::Tick(float InDeltaSeconds)
{
    if(!this->Peacenet) return;
    if(!this->Mission) return;

    if(this->CurrentTask == -1)
    {
        this->Advance();
    }
    else if(this->LoadedTasks[this->CurrentTask].Task->GetIsFinished())
    {
        this->Advance();
    }
    else
    {
        this->LoadedTasks[this->CurrentTask].Task->Tick(InDeltaSeconds);
    }

    Super::Tick(InDeltaSeconds);
}