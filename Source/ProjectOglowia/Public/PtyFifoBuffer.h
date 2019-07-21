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
 *******************************7*************************************************/

#pragma once

#include "CoreMinimal.h"
#include "PtyFifoBuffer.generated.h"

UCLASS()
class PROJECTOGLOWIA_API UPtyFifoBuffer : public UObject
{
    GENERATED_BODY()

private:
    // Contains the actual binary data of the stream.
    TArray<TCHAR> BitstreamDream;

    UPROPERTY()
    int Pos = 0;

    UPROPERTY()
    bool Raw = false;

public:
    UFUNCTION()
    int GetPosition();

    int Read(TArray<TCHAR>& Buffer, int Offset, int Count);
    void Write(TArray<TCHAR> Buffer, int Offset, int Count);
    void WriteChar(TCHAR c);

    UFUNCTION()
    FString DumpToString();

    UFUNCTION()
    bool IsRaw();

    UFUNCTION()
    void RawMode(bool value);
};