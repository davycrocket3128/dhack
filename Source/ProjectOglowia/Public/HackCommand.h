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
#include "TerminalCommand.h"
#include "Exploit.h"
#include "Payload.h"
#include "PayloadAsset.h"
#include "UserContext.h"
#include "HackCommand.generated.h"

UCLASS(BlueprintType)
class PROJECTOGLOWIA_API AHackCommand : public ATerminalCommand
{
    GENERATED_BODY()

public:
    AHackCommand();

private:
    UPROPERTY()
    FString EnteredHostname;

    UPROPERTY()
    USystemContext* RemoteSystem;

    UPROPERTY()
    UExploit* CurrentExploit;

    UPROPERTY()
    UPayloadAsset* CurrentPayload;

    UPROPERTY()
    bool WaitingForCommand = true;

protected:
    UFUNCTION()
    void HandleCommand(FString InCommandName, TArray<FString> InArguments);

    virtual void Tick(float InDeltaSeconds) override;
    virtual void NativeRunCommand(UConsoleContext* InConsole, TArray<FString> InArguments) override;
};