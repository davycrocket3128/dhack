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
#include "PeacenetWorldStateActor.h"
#include "PeacegateFileSystem.h"
#include "TerminalColor.h"
#include "PtyStream.h"
#include "LineNoiseUE4.h"
#include "ConsoleColor.h"
#include "ConsoleContext.generated.h"

class UUserContext;

/**
 * Encapsulates a terminal.
 */
UCLASS(BlueprintType)
class PROJECTOGLOWIA_API UConsoleContext : public UObject
{
	GENERATED_BODY()
	DECLARE_DYNAMIC_DELEGATE(FForceTtyUpdate);
	DECLARE_DYNAMIC_DELEGATE_TwoParams(FConsoleGetSize, int&, OutRows, int&, OutColumns);

private:
	UPROPERTY()
	bool IsBackgroundColorSet = false;

	UPROPERTY()
	bool IsForegroundColorSet = false;

	UPROPERTY()
	EConsoleColor BackgroundColor = EConsoleColor::Black;

	UPROPERTY()
	EConsoleColor ForegroundColor = EConsoleColor::White;

	UPROPERTY()
	bool IsBold = false;

	UPROPERTY()
	bool IsItalic = false;

	UPROPERTY()
	bool IsDim = false;

	UPROPERTY()
	bool IsUnderline = false;

	UPROPERTY()
	bool IsBlinking = false;

	UPROPERTY()
	bool IsReversed = false;

	UPROPERTY()
	bool IsHidden = false;

	UPROPERTY()
	ULineNoise* LineNoise = nullptr;

	UPROPERTY()
	FForceTtyUpdate UpdateTty;

	UPROPERTY()
	FConsoleGetSize GetSizeDelegate;

	UPROPERTY()
	UPtyStream* Pty;

	UPROPERTY()
	UUserContext* UserContext;

	UPROPERTY()
	FString WorkingDirectory;

private:
	UFUNCTION()
	void SetTerminalMode();

	UFUNCTION()
	void WriteToPty(FString str);

public:
	UFUNCTION()
	void OnTtyUpdate(UObject* Callee, FName FunctionName);

	UFUNCTION()
	void OnGetSize(UObject* Callee, FName FunctionName);

	UFUNCTION()
	void Setup(UPtyStream* InPty, UUserContext* InUserContext);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Console Context")
	UUserContext* GetUserContext();

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Console Context")
	FString GetWorkingDirectory();

	UFUNCTION(BlueprintCallable, Category = "Console|Formatting")
	void SetBold(bool InValue);

	UFUNCTION(BlueprintCallable, Category = "Console|Formatting")
	void SetItalic(bool InValue);

	UFUNCTION(BlueprintCallable, Category = "Console|Formatting")
	void SetUnderline(bool InValue);

	UFUNCTION(BlueprintCallable, Category = "Console|Formatting")
	void ResetFormatting();

	UFUNCTION(BlueprintCallable, Category = "Console|Formatting")
	void SetReversed(bool InValue);

	UFUNCTION(BlueprintCallable, Category = "Console|Formatting")
	void SetColors(EConsoleColor InForeground, EConsoleColor InBackground);

	UFUNCTION(BlueprintCallable, Category = "Console|Formatting")
	void SetForegroundColor(EConsoleColor InColor);

	UFUNCTION(BlueprintCallable, Category = "Console|Formatting")
	void ResetForegroundColor();

	UFUNCTION()
	bool GetLine(FString& OutLine);

	UFUNCTION(BlueprintCallable, Category = "Console Context")
	void SetWorkingDirectory(const FString& InPath);

	UFUNCTION(BlueprintCallable, Category = "Filesystem")
	FString CombineWithWorkingDirectory(const FString& InPath);

	UFUNCTION(BlueprintCallable, Category = "Bash", meta = (Pure))
	FString GetDisplayWorkingDirectory();
	
	UFUNCTION(BlueprintCallable, Category = "Console")
	void WriteLine(const FText& InText, float DelaySeconds = 0.f);

	UFUNCTION(BlueprintCallable, Category = "Console")
	void Write(const FText& InText, float DelaySeconds = 0.f);

	UFUNCTION(BlueprintCallable, Category = "Console")
	void OverwriteLine(const FText& InText, float DeltaSeconds = 0.f);

	UFUNCTION(BlueprintCallable, Category="Console")
	void Clear();

	UFUNCTION(BlueprintCallable, Category="Console", meta=(Latent, LatentInfo="LatentInfo", HidePin="WorldContextObject", DefaultToSelf="WorldContextObject"))
	void ReadLine(UObject* WorldContextObject, struct FLatentActionInfo LatentInfo, FString& OutText);

	// Creates a console context that uses this context's output as input and writes output to an internal buffer which
	// can then be used as input later.
	UFUNCTION()
	UConsoleContext* Pipe();

	// Creates a console context that uses this console's output as input and uses the specified console for output.
	UFUNCTION()
	UConsoleContext* PipeOut(UConsoleContext* InConsole);

	// Creates a console context that uses this console as input and writes output to the specified pty buffer.
	UFUNCTION()
	UConsoleContext* Redirect(UPtyFifoBuffer* InStream);

	UFUNCTION()
	UPtyStream* GetPty();

	UFUNCTION()
	int GetCursorRow();

	UFUNCTION()
	int GetCursorColumn();

	UFUNCTION()
	FIntPoint GetCursorPosition();

	UFUNCTION()
	int GetRows();

	UFUNCTION()
	int GetColumns();

	UFUNCTION()
	FIntPoint GetTerminalSize();

	UFUNCTION()
	void Beep();

	UFUNCTION()
	void InitAdvancedGetLine(FString InPrompt);

	UFUNCTION()
	bool UpdateAdvancedGetLine(FString& Line);
};
