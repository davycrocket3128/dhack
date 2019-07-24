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
#include "PtyFifoBuffer.h"
#include "PtyStream.generated.h"

#define PTY_LINEBUFFER_SIZE 1024

#define VEOF 0
#define VEOL 1
#define VEOL2 2
#define VERASE 3
#define VKILL 5
#define VINTR 8
#define VQUIT 9
#define VSUSP 10
#define NCCS 20

#define IGNBRK 0x0001
#define BRKINT 0x0002
#define ICRNL 0x01000
#define OPOST 0x0001
#define ONLCR 0x0002
#define OXTABS 0x0004
#define ECHOKE 0x0001
#define ECHOE 0x0002
#define ECHO 0x0008
#define ECHONL 0x0010

#define ISIG 0x0080
#define ICANON 0x0100
#define IEXTEN 0x0400
#define CREAD 0x0800

USTRUCT()
struct FPtyOptions
{
    GENERATED_BODY()

public:
    uint32 OFlag;
    uint32 LFlag;
    TArray<TCHAR> C_cc;

    FPtyOptions()
    {
        C_cc.AddZeroed(20);
    }
};

UCLASS()
class PROJECTOGLOWIA_API UPtyStream : public UObject
{
    GENERATED_BODY()

private:
    TArray<TCHAR> LineBuffer;

    UPROPERTY()
    FPtyOptions Options;

    UPROPERTY()
    UPtyFifoBuffer* InputStream;

    UPROPERTY()
    UPtyFifoBuffer* OutputStream;

    UPROPERTY()
    bool IsMaster;

    UPROPERTY()
    int LineBufferPosition = 0;

private:
    void WriteOutput(TCHAR c);
    void WriteInput(TCHAR c);

public:
    UPtyStream();
    ~UPtyStream();

    UFUNCTION()
    void FlushLineBuffer();
    
    UFUNCTION()
    void RawMode(bool value);

    int Read(TArray<TCHAR>& Buffer, int Offset, int Count);
    void Write(TArray<TCHAR> Buffer, int Offset, int Count);
    void WriteChar(TCHAR c);
    bool ReadChar(TCHAR& OutChar);


private:
    UFUNCTION()
    static UPtyStream* ConstructPtyStream(FPtyOptions InOptions, UPtyFifoBuffer* InputBuffer, UPtyFifoBuffer* OutputBuffer, bool InIsMaster);

public:
    UFUNCTION()
    static void CreatePty(UPtyStream*& OutMaster, UPtyStream*& OutSlave, FPtyOptions InOptions);

public:
    UFUNCTION()
    UPtyStream* RedirectInto(UPtyFifoBuffer* InBuffer);
};