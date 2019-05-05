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
#include "ConsoleContext.h"
#include "PlaceboLatentAction.h"
#include "PiperContext.generated.h"

class UPTerminalWidget;

/**
 * Encapsulates a console context which acts as a pipe between two other consoles.
 */
UCLASS(Blueprintable)
class PROJECTOGLOWIA_API UPiperContext : public UConsoleContext
{
	GENERATED_BODY()

private:
    UPROPERTY()
	UPiperContext* Input;

    UPROPERTY()
	UConsoleContext* Output;

    UPROPERTY()
	FString Log;

public:	
    UFUNCTION()
    void SetupPiper(UPiperContext* InInput, UConsoleContext* InOutput);

    UFUNCTION()
    FString GetLog();

	virtual void Write(const FString& InText, float DeltaSeconds) override;
	virtual void WriteLine(const FString& InText, float DeltaSeconds) override;
	virtual void OverwriteLine(const FString& InText, float DeltaSeconds) override;
	virtual UConsoleContext* CreateChildContext(USystemContext* InSystemContext, int InUserID) override;
	virtual void Clear() override;
	virtual bool GetLine(FString& OutLine) override;
	virtual UPTerminalWidget* GetTerminal() override;

	FString GetInputBuffer();

	virtual void ReadLine(UObject* WorldContextObject, struct FLatentActionInfo LatentInfo, FString& OutText) override;
};
