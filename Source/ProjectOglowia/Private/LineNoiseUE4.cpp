/* LineNoiseUE4.cpp -- linenouse for Unreal Engine 4/The Peacenet
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

#include "LineNoiseUE4.h"
#include "PtyStream.h"
#include "UserContext.h"
#include "ConsoleContext.h"
#include "PeacegateFileSystem.h"

#define LINENOISE_DEFAULT_HISTORY_MAX_LEN 100
#define LINENOISE_MAX_LINE 4096

ULineNoise::ULineNoise()
{
    this->HistoryMaxLen = LINENOISE_DEFAULT_HISTORY_MAX_LEN;
}

enum KEY_ACTION{
	KEY_NULL = 0,	    /* NULL */
	CTRL_A = 1,         /* Ctrl+a */
	CTRL_B = 2,         /* Ctrl-b */
	CTRL_C = 3,         /* Ctrl-c */
	CTRL_D = 4,         /* Ctrl-d */
	CTRL_E = 5,         /* Ctrl-e */
	CTRL_F = 6,         /* Ctrl-f */
	CTRL_H = 8,         /* Ctrl-h */
	TAB = 9,            /* Tab */
	CTRL_K = 11,        /* Ctrl+k */
	CTRL_L = 12,        /* Ctrl+l */
	ENTER = 13,         /* Enter */
	CTRL_N = 14,        /* Ctrl-n */
	CTRL_P = 16,        /* Ctrl-p */
	CTRL_T = 20,        /* Ctrl-t */
	CTRL_U = 21,        /* Ctrl+u */
	CTRL_W = 23,        /* Ctrl+w */
	ESC = 27,           /* Escape */
	BACKSPACE =  127    /* Backspace */
};

void ULineNoise::SetMultiline(bool IsMultiline)
{
    this->MultilineMode = IsMultiline;
}

void ULineNoise::ClearScreen()
{
    this->Console->Clear();
}

void ULineNoise::Beep()
{
    this->Console->Beep();
}

void ULineNoiseCompletions::FreeCompletions()
{
    this->CompletionsArray.Empty();
}

FString ULineNoiseCompletions::Get(int i)
{
    return this->CompletionsArray[i];
}

int ULineNoise::CompleteLine()
{
    ULineNoiseCompletions* lc = NewObject<ULineNoiseCompletions>();
    int nread, nwritten;
    TCHAR c = '\0';


    this->CompletionCallback.ExecuteIfBound(lc);

    if(lc->GetLength() == 0)
    {
        this->Beep();
    }
    else {
        int stop = 0;
        int i = 0;
        while(!stop)
        {
            if(i < lc->GetLength())
            {
                FLineNoiseState saved = this->State;
                this->State.Position = lc->Get(i).Len();
                this->State.Buffer = lc->Get(i);
                this->RefreshLine();
                this->State.Position = saved.Position;
                this->State.Buffer = saved.Buffer;
            }
            else
            {
                this->RefreshLine();
            }

            nread = this->Console->GetPty()->ReadChar(c);
            if(nread <= 0)
            {
                lc->FreeCompletions();
                return -1;
            }

            switch(c)
            {
                case 9:
                    i = (i + 1) % (lc->GetLength() + 1);
                    if(i == lc->GetLength()) this->Beep();
                    break;
                case 27:
                    if(i < lc->GetLength()) this->RefreshLine();
                    stop = 1;
                    break;
                default:
                    if(i < lc->GetLength())
                    {
                        this->State.Buffer = lc->Get(i);
                        nwritten = this->State.Buffer.Len();
                        this->State.Position = nwritten;
                    }
                    stop = 1;
                    break;
            }
        }
    }

    lc->FreeCompletions();
    return c;
}

void ULineNoiseCompletions::AddCompletion(FString InCompletion)
{
    this->CompletionsArray.Add(InCompletion);
}

void ULineNoise::RefreshShowHints(FString& ab)
{
    if(this->State.Prompt.Len() + this->State.Buffer.Len() < this->State.Cols)
    {
        int color = -1, bold = 0;
        FString hint;
        this->HintsCallback.ExecuteIfBound(this->State.Buffer, color, bold, hint);
        if(hint.Len())
        {
            int hintLen = hint.Len();
            int hintMaxLen = this->State.Cols - (this->State.Prompt.Len() + this->State.Buffer.Len());
            if(hintLen > hintMaxLen) hintLen = hintMaxLen;
            if(bold && color == -1) color = 37;
            if(color != -1 || bold)
            {
                ab += "\x1B[" + FString::FromInt(bold) + ";" + FString::FromInt(color) + "m";
            }
            ab+=hint;
            if(color != -1 || bold)
            {
                ab += "\x1B[0m";
            }            

        }
    }
}

void ULineNoise::RefreshSingleLine()
{
    FString seq;
    int plen = this->State.Prompt.Len();
    UConsoleContext* fd = this->Console;
    FString buf = this->State.Buffer;
    int bufp = 0;
    int len = this->State.Buffer.Len();
    int pos = this->State.Position;
    

    while((plen + pos) >= this->State.Cols)
    {
        bufp++;
        len--;
        pos--;
    }
    while(plen+len > this->State.Cols)
        len--;

    seq += "\r";
    seq += this->State.Prompt;
    for(int i = bufp; i < bufp + len; i++)
    {
        seq += buf[i];
    }

    this->RefreshShowHints(seq);

    seq += "\x1B[0K\r\x1B[" + FString::FromInt(plen + pos) + "C";

    fd->Write(FText::FromString(seq), 0.f);
}

void ULineNoise::RefreshMultiLine()
{
    FString seq;
    int plen = this->State.Prompt.Len();
    int rows = (plen+this->State.Buffer.Len()+this->State.Cols-1)/this->State.Cols;
    int rpos = (plen+this->State.OldPosition+this->State.Cols)/this->State.Cols;
    int rpos2;
    int col;
    int oldRows = this->State.MaxRows;
    UConsoleContext* fd = this->Console;
    int j;

    if(rows > oldRows) this->State.MaxRows = rows;

    if(oldRows - rpos > 0)
    {
        seq += "\x1B[" + FString::FromInt(oldRows - rpos) + "B";
    }

    for(j = 0; j < oldRows-1; j++)
    {
        seq += "\x1B[0K";
        seq += "\x1B[1A";
    }

    seq += "\r\x1B[0K";

    seq += this->State.Prompt;

    seq += this->State.Buffer;

    this->RefreshShowHints(seq);

        if (this->State.Position &&
        this->State.Position == this->State.Buffer.Len() &&
        (this->State.Position + plen) % this->State.Cols == 0)
    {
        seq += "\n\r";
        rows++;
        if(rows>this->State.MaxRows) this->State.MaxRows = rows;
    }

    rpos2 = (plen+this->State.Position+this->State.Cols)/this->State.Cols;
    if(rows-rpos2>0)
    {
        seq += "\x1B[" + FString::FromInt(rows - rpos2) + "A";
    }

    col = (plen+(int)this->State.Position) % (int)this->State.Cols;
    seq += "\r";
    if(col)
    {
        seq += "\x1B[" + FString::FromInt(col) + "C";
    }

    this->State.OldPosition = this->State.Position;
    fd->Write(FText::FromString(seq));
}

void ULineNoise::RefreshLine()
{
    if(this->MultilineMode)
    {
        this->RefreshMultiLine();
    }
    else {
        this->RefreshSingleLine();
    }
}

int ULineNoise::EditInsert(TCHAR c)
{
    if(this->State.Position == this->State.Buffer.Len())
    {
        this->State.Buffer += c;
        this->State.Position++;
        if(!MultilineMode && this->State.Prompt.Len() + this->State.Buffer.Len() < this->State.Cols && !HintsCallback.IsBound())
        {
            this->Console->Write(FText::FromString(FString::Chr(c)));
        }
        else
        {
            this->RefreshLine();
        }
    }
    else {
        this->State.Buffer.InsertAt(this->State.Position, FString::Chr(c));
        this->State.Position++;
        this->RefreshLine();
    }
    return 0;
}

void ULineNoise::MoveLeft()
{
    if(this->State.Position > 0)
    {
        this->State.Position--;
        this->RefreshLine();
    }
}

void ULineNoise::MoveRight()
{
    if(this->State.Position < this->State.Buffer.Len())
    {
        this->State.Position++;
        this->RefreshLine();
    }
}

void ULineNoise::MoveHome()
{
    if(this->State.Position != 0)
    {
        this->State.Position = 0;
        this->RefreshLine();
    }
}

void ULineNoise::MoveEnd()
{
    if(this->State.Position != this->State.Buffer.Len())
    {
        this->State.Position = this->State.Buffer.Len();
        this->RefreshLine();
    }
}

#define LINENOISE_HISTORY_NEXT 0
#define LINENOISE_HISTORY_PREV 1

void ULineNoise::HistoryNext(int dir)
{
    if(this->History.Num() > 1)
    {
        this->History[this->History.Num() - 1 - this->HistoryIndex] = this->State.Buffer;
        this->HistoryIndex += (dir == LINENOISE_HISTORY_PREV) ? 1 : -1;
        if(this->HistoryIndex < 0) 
        {
            this->HistoryIndex = 0;
            return;
        }
        else if(this->HistoryIndex >= this->History.Num())
        {
            this->HistoryIndex = this->History.Num() - 1;
            return;
        }
        this->State.Buffer = this->History[this->History.Num() - 1 - this->HistoryIndex];

        this->State.Position = this->State.Buffer.Len();
        this->RefreshLine();
    }
}

void ULineNoise::Delete()
{
    if(this->State.Buffer.Len() > 0 && this->State.Position < this->State.Buffer.Len())
    {
        this->State.Buffer.RemoveAt(this->State.Position, 1);
        this->RefreshLine();
    }

}

void ULineNoise::Backspace()
{
    if(this->State.Buffer.Len() > 0 && this->State.Position > 0)
    {
        this->State.Position--;
        this->State.Buffer.RemoveAt(this->State.Position, 1);
        this->RefreshLine();
    }
}

void ULineNoise::DeletePreviousWord()
{
    int old_pos = this->State.Position;
    int diff;

    while (this->State.Position > 0 && this->State.Buffer[this->State.Position-1] == ' ')
        this->State.Position--;
    while (this->State.Position > 0 && this->State.Buffer[this->State.Position-1] != ' ')
        this->State.Position--;
    diff = old_pos - this->State.Position;
    this->State.Buffer.RemoveAt(this->State.Position, diff);
    this->RefreshLine();
}

void ULineNoise::SetConsole(UConsoleContext* InConsole, FString Prompt)
{
    this->State = FLineNoiseState();
    this->State.Prompt = Prompt;

    this->Console = InConsole;

    this->State.Cols = this->Console->GetColumns();

    this->HistoryIndex = 0;

    this->AddToHistory("");

    this->Console->GetPty()->RawMode(true);

    this->Console->Write(FText::FromString("\r" + this->State.Prompt));
}

bool ULineNoise::GetLine(FString& Line)
{
    TCHAR c;
    while(this->Console->GetPty()->ReadChar(c))
    {
        int diff = 0;
        if(c == 9 && this->CompletionCallback.IsBound())
        {
            c = this->CompleteLine();
            if(c == 0) continue;
        }
        switch(c)
        {
            case ENTER:
                this->History.RemoveAt(this->History.Num() - 1);
                if(this->MultilineMode) this->MoveEnd();

                if(this->HintsCallback.IsBound())
                {
                    FLineNoiseHintsCallback cb = this->HintsCallback;
                    this->HintsCallback.Unbind();
                    this->RefreshLine();
                    this->HintsCallback = cb;
                }

                Line = this->State.Buffer;
                return true;
            case CTRL_C:
                return true;
            case BACKSPACE:
            case 8:
                this->Backspace();
                break;
            case CTRL_D:
                if(this->State.Buffer.Len() > 0)
                {
                    this->Delete();
                }
                else {
                    this->History.RemoveAt(this->History.Num()-1);
                    return true;
                }
                break;
            case CTRL_T:
                if(this->State.Position > 0 && this->State.Position < this->State.Buffer.Len())
                {
                    TCHAR aux = this->State.Buffer[this->State.Position - 1];
                    this->State.Buffer.RemoveAt(this->State.Position - 1);
                    this->State.Buffer.InsertAt(this->State.Position, aux);
                    if(this->State.Position != this->State.Buffer.Len() - 1) this->State.Position++;
                    this->RefreshLine();
                }
                break;
            case CTRL_B:
                this->MoveLeft();
                break;
            case CTRL_F:
                this->MoveRight();
                break;
            case CTRL_P:
                this->HistoryNext(LINENOISE_HISTORY_PREV);
                break;
            case CTRL_N:
                this->HistoryNext(LINENOISE_HISTORY_NEXT);
                break;
            case ESC:
                // TODO: Handle escape sequences for delete key, etc.
                break;

            default:
                this->EditInsert(c);
                break;
            
            case CTRL_U:
                this->State.Buffer = "";
                this->State.Position = 0;
                this->RefreshLine();
                break;
            case CTRL_K:
                diff = this->State.Buffer.Len() - this->State.Position;
                if(diff > 0)
                {
                    this->State.Buffer.RemoveAt(this->State.Position, diff);
                    this->RefreshLine();
                }
                break;
            case CTRL_A:
                this->MoveHome();
                break;
            case CTRL_E:
                this->MoveEnd();
                break;
            case CTRL_L:
                this->ClearScreen();
                this->RefreshLine();
                break;
            case CTRL_W:
                this->DeletePreviousWord();
                break;
        }
    }
    return false;
}

int ULineNoise::AddToHistory(FString Line)
{
    if(this->HistoryMaxLen <= 0) return 0;

    if(this->History.Contains(Line)) return 0;

    if(this->History.Num() == this->HistoryMaxLen)
    {
        this->History.RemoveAt(0);
    }

    this->History.Add(Line);
    return 1;
}

UFUNCTION()
int ULineNoise::SetMaxHistoryLength(int len)
{
    if(len < 1) return 1;
    while(this->History.Num() > len)
    {
        this->History.RemoveAt(0);
    }
    this->HistoryMaxLen = len;
    return 0;
}

int ULineNoise::SaveHistory(FString InFilePath)
{
    FString hist;
    for(FString h : this->History)
        hist += h + "\r\n";
    this->Console->GetUserContext()->GetFilesystem()->WriteText(InFilePath, hist);
    return 0;
}

int ULineNoise::LoadHistory(FString InFilePath)
{
    EFilesystemStatusCode status;
    FString hist;
    if(!this->Console->GetUserContext()->GetFilesystem()->ReadText(InFilePath, hist, status)) return 1;

    hist.ParseIntoArray(this->History, TEXT("\r\n"));
    return 0;
}

int ULineNoiseCompletions::GetLength()
{
    return this->CompletionsArray.Num();
}

