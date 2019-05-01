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

#include "HackTask.h"
#include "PeacenetWorldStateActor.h"

void UHackTask::NativeTick(float InDeltaSeconds)
{
    // Are we in the hack and is the payload active?
    if(this->IsInHack && this->IsPayloadDeployed && !this->AllSubtasksCompleted)
    {
        // If the current task is -1 then we automatically advance.
        if(this->CurrentSubtask == -1)
        {
            this->AdvanceSubtask();
            return;
        }

        // Tick the subtask.
        this->RealSubtasks[this->CurrentSubtask]->Tick(InDeltaSeconds);

        // If it's failed, we fail.
        if(this->RealSubtasks[this->CurrentSubtask]->GetIsFailed())
        {
            this->Fail(this->RealSubtasks[this->CurrentSubtask]->GetFailMessage());
            return;
        }

        // If it's completed, we advance.
        if(this->RealSubtasks[this->CurrentSubtask]->GetIsFinished())
        {
            this->AdvanceSubtask();
            return;
        }
    }
}

void UHackTask::NativeStart()
{
    // Check that we have a story character to hack.
    check(this->StoryCharacter);

    // Get the target entity.
    bool result = this->GetPlayerUser()->GetPeacenet()->SaveGame->GetStoryCharacterID(this->StoryCharacter, this->TargetEntity);
    check(result);

    // Reset the state of the hack.
    this->IsInHack = false;
    this->IsPayloadDeployed = false;
    this->AllSubtasksCompleted = false;

    // Reset the state of the subtasks.
    this->CurrentSubtask = -1;
    this->RealSubtasks = TArray<UMissionTask*>();

    // Load in only VALID subtasks.
    for(auto& Subtask : this->SubTasks)
    {
        if(Subtask.Task)
            this->RealSubtasks.Add(Subtask.Task);
    }


}

void UHackTask::AdvanceSubtask()
{
    // Increase the subtask pointer by one so the next subtask becomes the current.
    this->CurrentSubtask++;
    
    // If the pointer is at the end of the subtask list, then we allow the player to disconnect from the remote computer
    // without failing the mission.
    if(this->CurrentSubtask >= this->RealSubtasks.Num())
    {
        this->AllSubtasksCompleted = true;
        return;
    }

    // Start the current subtask.
    this->RealSubtasks[this->CurrentSubtask]->Start(this->GetMissionActor());
}

void UHackTask::NativeEvent(FString EventName, TMap<FString, FString> InEventArgs)
{
    if(EventName == "HackStart" && !IsInHack)
    {
        if(InEventArgs["Identity"] == FString::FromInt(this->TargetEntity))
        {
            this->IsInHack = true;
        }
    }

    if(EventName == "PayloadDeploy" && IsInHack && !IsPayloadDeployed)
    {
        if(InEventArgs["Identity"] == FString::FromInt(this->TargetEntity))
        {
            this->IsPayloadDeployed = true;
            this->CurrentSubtask = -1;
        }
    }

    if(EventName == "HackSuccess" && IsInHack && IsPayloadDeployed)
    {
        if(InEventArgs["Identity"] == FString::FromInt(this->TargetEntity))
        {
            // If the hack ends before all sub-tasks are completed, we fail.
            if(!this->AllSubtasksCompleted)
            {
                this->Fail(NSLOCTEXT("TaskFailures", "HackTaskMissingSubtasks", "Not all objectives were completed before the hack was ended."));
                return;
            }

            this->Complete();
        }
    }
}