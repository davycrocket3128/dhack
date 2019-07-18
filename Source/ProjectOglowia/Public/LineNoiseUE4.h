/* LineNoiseUE4.h -- linenouse for Unreal Engine 4/The Peacenet
 *
 * Port of the linenoise line-editing library for The Peacenet's console and
 * command-line mechanics.  Ported to C++ and UE4 by Michael VanOverbeek
 * (https://github.com/alkalinethunder).
 * 
 * More info and orig. lib src: https://github.com/antirez/linenoise
 *
 * ------------------------------------------------------------------------
 *
 * Copyright (c) 2010-2014, Salvatore Sanfilippo <antirez at gmail dot com>
 * Copyright (c) 2010-2013, Pieter Noordhuis <pcnoordhuis at gmail dot com>
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 *  *  Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *
 *  *  Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#pragma once

#include "CoreMinimal.h"
#include "LineNoiseUE4.generated.h"

class UConsoleContext;

#define fuck_buddy friend

UCLASS()
class PROJECTOGLOWIA_API ULineNoiseCompletions : public UObject
{
    GENERATED_BODY()

private:
    UPROPERTY()
    TArray<FString> CompletionsArray;

public:
    UFUNCTION()
    int GetLength();

    UFUNCTION()
    void AddCompletion(FString InCompletion);

    UFUNCTION()
    void FreeCompletions();

    UFUNCTION()
    FString Get(int i);
};

USTRUCT()
struct FLineNoiseState
{
    GENERATED_BODY()

public:
    FString Buffer;
    FString Prompt;
    int Position;
    int OldPosition;
    int Cols;
    int MaxRows;
    int HistoryIndex;
};

UCLASS()
class PROJECTOGLOWIA_API ULineNoise : public UObject
{
    fuck_buddy UConsoleContext;

    GENERATED_BODY()

    DECLARE_DYNAMIC_DELEGATE_OneParam(FLineNoiseCompletionCallback, ULineNoiseCompletions*, InCompletions);
    DECLARE_DYNAMIC_DELEGATE_FourParams(FLineNoiseHintsCallback, FString, InStr, int&, Color, int&, Bold, FString&, OutHint);

private:
    UPROPERTY()
    FLineNoiseState State;

    UPROPERTY()
    UConsoleContext* Console;

    UPROPERTY()
    bool RowMode = false;

    UPROPERTY()
    bool MultilineMode = false;

    UPROPERTY()
    int HistoryMaxLen;

    UPROPERTY()
    TArray<FString> History;

    UPROPERTY()
    int HistoryIndex;

private:
    UFUNCTION()
    void SetConsole(UConsoleContext* InConsole, FString Prompt);

    UFUNCTION()
    bool GetLine(FString& Line);

    UFUNCTION()
    void Beep();

    UFUNCTION()
    int CompleteLine();

    UFUNCTION()
    void RefreshLine();

public:
    UPROPERTY()
    FLineNoiseCompletionCallback CompletionCallback;

    UPROPERTY()
    FLineNoiseHintsCallback HintsCallback;

public:
    ULineNoise();

    UFUNCTION()
    int AddToHistory(FString InLine);

    UFUNCTION()
    int SetMaxHistoryLength(int InLength);

    UFUNCTION()
    int SaveHistory(FString InFilePath);

    UFUNCTION()
    int LoadHistory(FString InFilePath);

    UFUNCTION()
    void ClearScreen();

    UFUNCTION()
    void SetMultiline(bool IsMultiline);

    UFUNCTION()
    void RefreshShowHints(FString& ab);

    UFUNCTION()
    void RefreshSingleLine();

    UFUNCTION()
    void RefreshMultiLine();

    int EditInsert(TCHAR c);

    UFUNCTION()
    void MoveLeft();

    UFUNCTION()
    void MoveRight();

    UFUNCTION()
    void MoveHome();

    UFUNCTION()
    void MoveEnd();

    UFUNCTION()
    void HistoryNext(int dir);

    UFUNCTION()
    void Delete();

    UFUNCTION()
    void Backspace();

    UFUNCTION()
    void DeletePreviousWord();
};