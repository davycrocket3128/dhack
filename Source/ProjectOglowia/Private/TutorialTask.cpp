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

#include "TutorialTask.h"
#include "MissionActor.h"
#include "PeacenetWorldStateActor.h"
#include "PeacenetSaveGame.h"
#include "SystemContext.h"
#include "UserContext.h"

void UTutorialTask::MissionEnded()
{
    // Get rid of the objective text in case we have any.
    // Save desktops should handle the GUI side of this by not showing the mission acquisition
    // UI when not in a mission but.... there's such a thing as drunk, stoned or uncaffeinated Michael.
    this->SetObjectiveText(FText::GetEmpty());

    // And we'll let the user handle cleaning up after ending a mission.
    this->NativeMissionEnded();
    this->OnMissionEnded();
}

void UTutorialTask::SetObjectiveText(const FText& InObjectiveText)
{
    this->GetPlayerUser()->GetDesktop()->SetObjectiveText(InObjectiveText);
}

AMissionActor* UTutorialTask::GetMissionActor()
{
    return this->Mission;
}

bool UTutorialTask::IsTutorialActive()
{
    return this->GetPlayerUser()->GetPeacenet()->IsTutorialActive();
}

void UTutorialTask::ShowTutorialIfNotSet(FString InBoolean, const FText& InTitle, const FText& InTutorial)
{
    if(this->IsTutorialActive()) return;

    if(!this->IsSet(InBoolean))
    {
        this->GetPlayerUser()->GetPeacenet()->GetTutorialState()->ActivatePrompt(InTitle, InTutorial);
        this->SetBoolean(InBoolean, true);
    }
}

bool UTutorialTask::IsSet(FString InSaveBoolean)
{
    return this->GetPlayerUser()->GetPeacenet()->SaveGame->IsTrue(InSaveBoolean);
}

void UTutorialTask::SetBoolean(FString InSaveBoolean, bool InValue)
{
    this->GetPlayerUser()->GetPeacenet()->SaveGame->SetValue(InSaveBoolean, InValue);
}

bool UTutorialTask::GetIsFailed()
{
    return this->IsFailed;
}

FText UTutorialTask::GetFailMessage()
{
    return this->FailMessage;
}

void UTutorialTask::Fail(const FText& InFailMessage)
{
    check(!this->IsFailed);
    this->IsFailed = true;
    this->FailMessage = InFailMessage;
}

void UTutorialTask::HandleEvent(FString EventName, TMap<FString, FString> InEventArgs)
{
    if(this->IsFailed) return;

    this->NativeEvent(EventName, InEventArgs);
    this->OnHandleEvent(EventName, InEventArgs);
}

UUserContext* UTutorialTask::GetPlayerUser()
{
    int PlayerID = this->GetPeacenet()->SaveGame->PlayerComputerID;
    USystemContext* PlayerSystem = this->GetPeacenet()->GetSystemContext(PlayerID);
    return PlayerSystem->GetUserContext(this->GetPeacenet()->SaveGame->PlayerUserID);
}

APeacenetWorldStateActor* UTutorialTask::GetPeacenet()
{
    return this->Mission->GetPeacenet();
}

void UTutorialTask::Complete()
{
    this->IsFinished = true;
    this->SetObjectiveText(FText::GetEmpty());
}

void UTutorialTask::Start(AMissionActor* InMission)
{
    check(InMission);

    this->Mission = InMission;

    this->IsFinished = false;
    this->IsFailed = false;
    this->NativeStart();
    this->OnStart();
}

void UTutorialTask::Tick(float InDeltaSeconds)
{
    if(this->IsFailed) return;
    if(this->IsFinished) return;

    this->NativeTick(InDeltaSeconds);
    this->OnTick(InDeltaSeconds);
}

bool UTutorialTask::GetIsFinished()
{
    return this->IsFinished;
}