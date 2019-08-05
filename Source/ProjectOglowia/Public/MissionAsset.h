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
#include "Engine/DataAsset.h"
#include "Text.h"
#include "TextProperty.h"
#include "StoryCharacter.h"
#include "MissionTask.h"
#include "PostCompletionMissionAction.h"
#include "MissionAsset.generated.h"

USTRUCT(BlueprintType)
struct FMissionTaskInfo
{
    GENERATED_BODY()

public:
    UPROPERTY(Editanywhere, BlueprintReadOnly)
    bool IsCheckpoint = false;

    UPROPERTY(Editanywhere, BlueprintReadOnly, Instanced)
    UMissionTask* Task;
};

USTRUCT(BlueprintType)
struct PROJECTOGLOWIA_API FMissionAction
{
    GENERATED_BODY()

public:
    UPROPERTY(BlueprintReadOnly, EditAnywhere, Instanced)
    UPostCompletionMissionAction* Action;
};

UCLASS(Blueprintable, BlueprintType)
class PROJECTOGLOWIA_API UMissionAsset : public UDataAsset
{
    GENERATED_BODY()

public:
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Metadata")
    FText Name;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Metadata")
    FText Description;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Metadata")
    FText MailMessageText;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Metadata")
    bool IsSideMission = false;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Metadata")
    TArray<UMissionAsset*> RequiredMissions;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Metadata")
    UStoryCharacter* Assigner;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Tasks")
    TArray<FMissionTaskInfo> Tasks;

    // Whether or not the game should change the song(s) played during Free Roam.
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "When The Fucking Mission Is Completed")
    bool UpdateFreeRoamMusicState = false;

    // If set to true, the game will not show the "Mission Complete" UI when the mission is completed.
    // Use this for missions that seamlessly transition to other gameplay sequences or when you don't want the
    // player to know that they were in a mission the whole time.  Example: The prologue tutorial.
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "When The Fucking Mission Is Completed")
    bool CompleteSilently = false;

    // A list of actions to perform when the mission's been completed.
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "When The Fucking Mission Is Completed")
    TArray<FMissionAction> OnCompletion;
};