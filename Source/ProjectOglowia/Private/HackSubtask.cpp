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

#include "HackSubtask.h"

void UHackSubtask::SetObjectiveText(const FText& InObjectiveText)
{
    this->GetPlayerUser()->GetDesktop()->SetObjectiveText(InObjectiveText);
}

UUserContext* UHackSubtask::GetPlayerUser()
{
    return this->HackTask->GetPlayerUser();
}

USystemContext* UHackSubtask::GetHackedSystem()
{
    // UHackTask is friends with us so we can just steal private data from it lol.
    // Friendship, love and gratitude... It's normal.  And lovely.  Bliss.
    int HackedIdentity = this->HackTask->TargetEntity;

    // And the Peacenet state will be able to give us that entity's System Context.
    // Woohoo.  This is almost easier than ordering lunch at Tim Hortons.
    //
    // Yeah, I'll have a Turkey Bacon Club and a Large Ice Cap to go, thank you.
    // That'll be $11.50.
    return this->GetPlayerUser()->GetPeacenet()->GetSystemContext(HackedIdentity);
}

void UHackSubtask::Fail(const FText& InFailMessage)
{
    check(!this->IsFinished);
    check(!this->IsFailed);

    this->IsFailed = true;
    this->FailMessage = InFailMessage;
}

void UHackSubtask::Finish()
{
    check(!this->IsFinished);

    this->IsFinished = true;
    this->SetObjectiveText(FText::GetEmpty());
}

void UHackSubtask::Start(UHackTask* InHackTask)
{
    check(InHackTask);

    this->HackTask = InHackTask;
    this->IsFinished = false;
    this->IsFailed = false;

    this->NativeStart();
    this->OnStart();
}

void UHackSubtask::Tick(float InDeltaSeconds)
{
    this->NativeTick(InDeltaSeconds);
    this->OnTick(InDeltaSeconds);
}

bool UHackSubtask::GetIsFailed()
{
    return this->IsFailed;
}

bool UHackSubtask::GetIsFinished()
{
    return this->IsFinished;
}

FText UHackSubtask::GetFailMessage()
{
    return this->FailMessage;
}

void UHackSubtask::GameEvent(FString EventName, TMap<FString, FString> InEventData)
{
    this->NativeGameEvent(EventName, InEventData);
    this->OnGameEvent(EventName, InEventData);
}