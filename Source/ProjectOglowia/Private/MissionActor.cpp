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

void AMissionActor::SilentFail()
{
    // Fail the mission directly without alerting the world state.
    this->IsFailed = true;

    // No UI is shown so we'll manually abandon the mission.
    this->AbandonMission();
}

void AMissionActor::FireCompletionEvents()
{
    if(this->Mission && this->Mission->OnCompletion.Num())
    {
        for(auto Event : this->Mission->OnCompletion)
        {
            if(Event.Action)
            {
                Event.Action->MissionCompleted(this);
            }
        }
    }
}

void AMissionActor::Advance()
{
    // Advance the current task pointer so the next task
    // begins.
    this->CurrentTask++;
    
    // Did we go past the total number of tasks? If so,
    // the mission is completed.
    if(this->CurrentTask >= this->LoadedTasks.Num())
    {
        // Let the last task know we've ended.
        int last = this->CurrentTask-1;
        if(last >= 0 && this->LoadedTasks[last].Task)
        {
            this->LoadedTasks[last].Task->MissionEnded();
        }

        // This lets the mission creator execute actions inside the game
        // after the player's completed the mission, such as sending the player
        // an email.
        this->FireCompletionEvents();

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
    this->Peacenet->EndMission(true);

    // If the mission says "update the music state" then we'll do that now.
    if(this->Mission->UpdateFreeRoamMusicState)
    {
        // Because I'm a good programmer with a ton of confidence and happiness thanks to
        // a certain friend of mine with red hair, I have already created a function in the public
        // API for doing EXACTLY what this if statement needs to do on truth.  NO. CODE. DUPLICATION.
        this->Peacenet->IncreaseGameStat("StoryMusicState");        
    }

    // Allow the game to show a "mission completed" UI.
    this->Peacenet->BroadcastMissionComplete(this->Mission);

    // Tell the rest of the game "HEY MOTHERFUCKERS, THE PLAYER JUST COMPLETED A BLOODY MISSION WOOOOOOOOOOO!"
    this->Peacenet->SendGameEvent("MissionComplete", {
        { "MissionID", this->Mission->GetFName().ToString()}
    });

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
    // Let the current task know that we're ending.
    if(this->CurrentTask >= 0 && this->CurrentTask < this->LoadedTasks.Num())
    {
        if(this->LoadedTasks[this->CurrentTask].Task)
        {
            this->LoadedTasks[this->CurrentTask].Task->MissionEnded();
        }
    }

    // Restore the save file.
    this->Peacenet->SaveGame = Cast<UPeacenetSaveGame>(UGameplayStatics::LoadGameFromSlot("PeacegateOS_PreMission", 0));

    // Delete save states.
    this->DeleteSaveStates();

    // Peacenet should know we're done.
    this->Peacenet->EndMission(false);

    // Destroy ourselves.
    this->Peacenet = nullptr;
    this->Mission = nullptr;

    this->Destroy();
}

void AMissionActor::SendGameEvent(FString EventName, TMap<FString, FString> InEventData)
{
    if(this->CurrentTask >= 0 && this->CurrentTask < this->LoadedTasks.Num() && this->LoadedTasks[this->CurrentTask].Task)
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

    if(this->IsFailed) return;

    if(this->CurrentTask == -1)
    {
        this->Advance();
    }
    else if(this->LoadedTasks[this->CurrentTask].Task->GetIsFinished())
    {
        this->Advance();
    }
    else if(this->LoadedTasks[this->CurrentTask].Task->GetIsFailed())
    {
        this->IsFailed = true;
        this->Peacenet->FailMission(this->LoadedTasks[this->CurrentTask].Task->GetFailMessage());
    }
    else
    {
        this->LoadedTasks[this->CurrentTask].Task->Tick(InDeltaSeconds);

        
    }

    Super::Tick(InDeltaSeconds);
}

bool AMissionActor::HasCheckpoint()
{
    return this->CheckpointTask != -1;
}

void AMissionActor::RestoreCheckpoint()
{
    // DO NOT DO THIS if the mission isn't failed!
    check(this->IsFailed);

    // Restoring the checkpoint is essentially doing the reverse of setting the checkpoint.
    // But we can only do it if there is a checkpoint in the first place.
    if(this->HasCheckpoint())
    {
        // Instead of setting the checkpoint task to the current task, we set the current task to the
        // checkpoint task.
        this->CurrentTask = this->CheckpointTask;

        // And instead of saving the game to the shadow save file, we load the game from it.
        UPeacenetSaveGame* ShadowGame = Cast<UPeacenetSaveGame>(UGameplayStatics::LoadGameFromSlot("PeacegateOS_PreMission", 1));
        this->Peacenet->SaveGame = ShadowGame;

        // Then we basically act like we completed the objective before the checkpoint one.
        this->LoadedTasks[this->CurrentTask].Task->Start(this);

        // And we resume the mission.
        this->IsFailed = false;
    }
}

void AMissionActor::RestartMission()
{
    // Do not allow restarting of the mission if we're not failed.
    check(this->IsFailed);

    // Load the premission shadow save into the main save game.
    UPeacenetSaveGame* ShadowSave = Cast<UPeacenetSaveGame>(UGameplayStatics::LoadGameFromSlot("PeacegateOS_PreMission", 0));
    this->Peacenet->SaveGame = ShadowSave;

    // Set the checkpoint task AND the current task to -1 causing a complete mission reset.
    this->CheckpointTask = -1;
    this->CurrentTask = -1;

    // We're no longer failed.
    this->IsFailed = false;
}

UMissionAsset* AMissionActor::GetMissionAsset()
{
    return this->Mission;
}

void AMissionActor::FailCurrentTask(const FText& InFailReason)
{
    this->IsFailed = true;
    this->Peacenet->FailMission(InFailReason);
}

void AMissionActor::AbandonMission()
{
    // Only allow this if the mission is failed.
    check(this->IsFailed);

    // Bye bye mo'fo's.
    this->Abort();
}

FText AMissionActor::GetMissionName()
{
    return this->Mission->Name;
}