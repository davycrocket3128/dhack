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
#include "Text.h"
#include "TextProperty.h"
#include "TutorialPromptState.generated.h"

class APeacenetWorldStateActor;

#define THIS_CLASS_IS_CLINGY friend APeacenetWorldStateActor;

UCLASS(BlueprintType)
class PROJECTOGLOWIA_API UTutorialPromptState : public UObject 
{
    THIS_CLASS_IS_CLINGY

    GENERATED_BODY()

    DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FTutorialActivatedEvent, const FText&, InTitle, const FText&, InText, UTutorialPromptState*, InTutorialState);

private:
    UPROPERTY()
    FText PromptTitle;
    
    UPROPERTY()
    FText PromptText;
    
    UPROPERTY()
    bool PromptActive = false;
    
    UPROPERTY()
    APeacenetWorldStateActor* MyPeacenet = nullptr;

public:
    UPROPERTY(BlueprintAssignable)
    FTutorialActivatedEvent TutorialActivated;

    UFUNCTION(BlueprintCallable)
    void ActivatePrompt(const FText& InTitle, const FText& InText);
    
    UFUNCTION(BlueprintCallable)
    void DismissPrompt();

    UFUNCTION()
    bool IsPromptActive();

public:
    UFUNCTION(BlueprintCallable, BlueprintPure)
    FText GetTutorialTitle();

    UFUNCTION(BlueprintCallable, BlueprintPure)
    FText GetTutorialText();
};