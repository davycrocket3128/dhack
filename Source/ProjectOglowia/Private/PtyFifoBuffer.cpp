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

#include "PtyFifoBuffer.h"

int UPtyFifoBuffer::GetPosition()
{
    return this->BitstreamDream.Num();
}

int UPtyFifoBuffer::Read(TArray<TCHAR>& Buffer, int Offset, int Count)
{
    int bytesRead = 0;

    for(int i = Offset; i < Offset + Count; i++)
    {
        if(this->BitstreamDream.Num() == 0)
            break;

        Buffer[i] = this->BitstreamDream[0];
        this->BitstreamDream.RemoveAt(0);
        bytesRead++;
    }
    return bytesRead;
}

void UPtyFifoBuffer::Write(TArray<TCHAR> Buffer, int Offset, int Count)
{
    for(int i = Offset; i < Offset + Count; i++)
    {
        this->WriteChar(Buffer[i]);
    }
}

void UPtyFifoBuffer::WriteChar(TCHAR c)
{
    this->BitstreamDream.Add(c);
}