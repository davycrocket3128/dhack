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

#pragma once

#include "CoreMinimal.h"
#include "MissionTask.h"
#include "UserContext.h"
#include "StoryCharacter.h"
#include "PeacenetSaveGame.h"
#include "HackTask.generated.h"

class UHackSubtask;

USTRUCT(BlueprintType)
struct FHackSubtaskInfo
{
    GENERATED_BODY()

public:
    UPROPERTY(Instanced, BlueprintReadOnly, EditAnywhere)
    UHackSubtask* Subtask;
};

UCLASS(BlueprintType)
class PROJECTOGLOWIA_API UHackTask : public UMissionTask
{
    friend UHackSubtask; // They're the best of friends. <3

    GENERATED_BODY()

private:
    UPROPERTY()
    int TargetEntity = 0;

    UPROPERTY()
    bool HasSubtasks = false;

    UPROPERTY()
    bool IsInHack = false;

    UPROPERTY()
    int CurrentSubtask = -1;

    UPROPERTY()
    TArray<UHackSubtask*> RealSubtasks;

    UPROPERTY()
    bool IsPayloadDeployed = false;

    UPROPERTY()
    bool AllSubtasksCompleted = false;

public:
    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    UStoryCharacter* StoryCharacter;

    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    bool FailOnCoverBlow = false;

    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    TArray<FHackSubtaskInfo> SubTasks;

protected:
    UFUNCTION()
    void AdvanceSubtask();

    virtual void NativeStart() override;
    virtual void NativeTick(float InDeltaSeconds) override;
    virtual void NativeEvent(FString EventName, TMap<FString, FString> InEventArgs) override;
};