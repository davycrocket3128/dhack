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

#include "PtyStream.h"

UPtyStream::UPtyStream() {
    this->LineBuffer.AddZeroed(PTY_LINEBUFFER_SIZE);
    this->LineBufferPosition = 0;
}

UPtyStream::~UPtyStream() {
    this->InputStream = nullptr;
    this->OutputStream = nullptr;
    this->LineBuffer.Empty();
    this->LineBufferPosition = 0;
}

UPtyStream* UPtyStream::ConstructPtyStream(FPtyOptions InOptions, UPtyFifoBuffer* InputBuffer, UPtyFifoBuffer* OutputBuffer, bool InIsMaster) {
    UPtyStream* stream = NewObject<UPtyStream>();

    stream->IsMaster = InIsMaster;
    stream->InputStream = InputBuffer;
    stream->OutputStream = OutputBuffer;
    stream->Options = InOptions;

    return stream;
}

void UPtyStream::CreatePty(UPtyStream*& OutMaster, UPtyStream*& OutSlave, FPtyOptions InOptions) {
    UPtyFifoBuffer* input = NewObject<UPtyFifoBuffer>();
    UPtyFifoBuffer* output = NewObject<UPtyFifoBuffer>();

    OutMaster = ConstructPtyStream(InOptions, input, output, true);
    OutSlave = ConstructPtyStream(InOptions, input, output, false);
}

void UPtyStream::Write(TArray<TCHAR> Buffer, int Offset, int Count) {
    for(int i = Offset; i < Offset + Count; i++) {
        if(this->IsMaster) {
            this->WriteOutput(Buffer[i]);
        } else {
            this->WriteInput(Buffer[i]);
        }
    }
}

int UPtyStream::Read(TArray<TCHAR>& Buffer, int Offset, int Count) {
    if(!this->IsMaster) {
        return this->OutputStream->Read(Buffer, Offset, Count);
    }

    int i;
    if ((i = this->InputStream->Read(Buffer, Offset, Count)) == 0) {
        return -1;
    }
    return i;
}

void UPtyStream::FlushLineBuffer() {
    this->InputStream->Write(this->LineBuffer, 0, this->LineBufferPosition);
    this->LineBufferPosition = 0;
}

void UPtyStream::WriteOutput(TCHAR c) {
    if (c == '\n' && (this->Options.OFlag & ONLCR) != 0) {
        this->OutputStream->WriteChar('\r');
    }

    this->OutputStream->WriteChar(c);
}

void UPtyStream::WriteInput(TCHAR c) {
    if ((this->Options.LFlag & ICANON) != 0 && !this->InputStream->IsRaw()) {
        if (c == this->Options.C_cc[VERASE]) {
            if (this->LineBufferPosition > 0) {
                this->LineBufferPosition--;
            }

            this->LineBuffer[LineBufferPosition] = '\0';

            this->WriteOutput('\b');

            return;
        }

        if (c == this->Options.C_cc[VINTR]) {
            this->WriteOutput('^');
            this->WriteOutput('C');
            this->WriteOutput('\n');

            this->FlushLineBuffer();

            return;

        }

        this->LineBuffer[this->LineBufferPosition++] = c;

        if ((this->Options.LFlag & ECHO) != 0) {
            this->WriteOutput(c);
        }

        if (c == this->Options.C_cc[VEOL] || c == this->Options.C_cc[VEOL2]) {
            this->FlushLineBuffer();
        }

        return;
    }
    this->InputStream->WriteChar(c);
}

void UPtyStream::WriteChar(TCHAR c) {
    TArray<TCHAR> buf = { c };
    this->Write(buf, 0, 1);
}

bool UPtyStream::ReadChar(TCHAR& OutChar) {
    TArray<TCHAR> buf = { '\0' };
    int read = this->Read(buf, 0, 1);
    if(read < 1) {
        return false;
    }
    OutChar = buf[0];
    return true;
}

void UPtyStream::RawMode(bool value) {
    this->InputStream->RawMode(value);
    if(this->InputStream->IsRaw()) {
        this->FlushLineBuffer();
    }
}

UPtyStream* UPtyStream::RedirectInto(UPtyFifoBuffer* InBuffer) {
    UPtyStream* Redirected = NewObject<UPtyStream>();
    Redirected->OutputStream = InBuffer;
    Redirected->InputStream = this->InputStream;
    Redirected->Options = this->Options;
    Redirected->IsMaster = this->IsMaster;
    return Redirected;
}

UPtyStream* UPtyStream::Pipe(UPtyFifoBuffer* Buffer) {
    // Create a new PTY stream...
    UPtyStream* PipeStream = NewObject<UPtyStream>();

    // This stream outputs to our output stream and reads from
    // the shared buffer.
    PipeStream->OutputStream = this->OutputStream;
    PipeStream->InputStream = Buffer;

    // And we need to output to the buffer for this to work.
    this->OutputStream = Buffer;

    // Copy over our settings...
    PipeStream->Options = this->Options;
    PipeStream->IsMaster = this->IsMaster;

    // And return the pipe.
    return PipeStream;
}

UPtyStream* UPtyStream::Clone() {
    UPtyStream* Cloned = NewObject<UPtyStream>();
    Cloned->OutputStream = this->OutputStream;
    Cloned->InputStream = this->InputStream;
    Cloned->IsMaster = this->IsMaster;
    Cloned->Options = this->Options;
    return Cloned;
}