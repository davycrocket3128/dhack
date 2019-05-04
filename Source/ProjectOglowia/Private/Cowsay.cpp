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

#include "Cowsay.h"
#include "PiperContext.h"

void ACowsay::NativeRunCommand(UConsoleContext* InConsole, TArray<FString> InArguments)
{
    // This is the text that will go in the cow's speech bubble.
    FString SpeechText = "";
    
    // Read from the console for cow speech.
    FString SpeechLine = "";
    while(InConsole->GetLine(SpeechLine))
    {
        SpeechText = SpeechText + SpeechLine + "\r\n";
    }

    // Trim excess whitespace.
    SpeechText = SpeechText.TrimStartAndEnd();

    // If the speech text is empty then we'll read from command-line arguments instead.
    if(!SpeechText.Len())
    {
        for(auto& Arg : InArguments)
        {
            SpeechText = SpeechText + Arg + " ";
        }
    }

    // Trim excess whitespace again.
    SpeechText = SpeechText.TrimStartAndEnd();

    // Make the cow.
    FString Cow = this->MakeSpeech(SpeechText, this->GetCow());

    // Write it to the console.
    InConsole->WriteLine(Cow);

    // We're done.
    this->Complete();
}

FString ACowsay::GetCow()
{
    return "\\  ^__^\r\n \\ (oo)\\_______\r\n   (__)\\       )\\/\\\r\n       ||----w |\r\n       ||     ||";
}

TArray<FString> ACowsay::SplitLines(FString InText, int InWrap)
{
    TArray<FString> Lines;
    FString Current = "";

    for(int i = 0; i < InText.Len(); i++)
    {
        TCHAR Char = InText[i];

        switch(Char)
        {
            case '\0':
            case '\b':
            case '\t':
            case '\v':
            case '\r':
                continue;
            case '\n':
                Lines.Add(Current);
                Current = "";
                continue;
            default:
                Current += FString::Chr(Char);
                break;
        }

        if(Current.Len() >= InWrap && InWrap > 0)
        {
            Lines.Add(Current);
            Current = "";
        }
    }

    if(Current.Len() > 0)
    {
        Lines.Add(Current);
        Current = "";
    }

    return Lines;
}

FString ACowsay::MakeSpeech(FString InSpeech, FString InCow)
{
    // Split the cow into individual lines of text.
    TArray<FString> cowlines = this->SplitLines(InCow, 0);

    // FString is really the UE4 version of a C# StringBuilder anyway lol.
    FString sb = "";

    // Wow.  UE4's libraries are so similar to C#'s!
    int length = FMath::Min(InSpeech.Len(), 30);
    
    // Add the top of the speech bubble.
    sb += " _";

    for(int i = 0; i < length; i++)
    {
        sb += "_";
    }
    
    sb += "_ \r\n";

    // Now we split the speech into lines.
    TArray<FString> lines = this->SplitLines(InSpeech, length);
    
    // And go through each line.
    for(int i = 0; i < lines.Num(); i++)
    {
        TCHAR begin = '|';
        TCHAR end = '|';
        
        if(i == 0)
        {
            if(lines.Num() == 1)
            {
                begin = '<';
                end = '>';
            }
            else
            {
                begin = '/';
                end = '\\';
            }
        }
        else if(i == lines.Num() - 1)
        {
            begin = '\\';
            end = '/';
        }
        
        FString line = lines[i];
        
        int lineLength = line.Len();
        int pad = length - lineLength;
        
        sb += FString::Chr(begin) + " " + line;
        for(int j = 0; j < pad; j++)
        {
            sb += " ";
        }
        sb += " " + FString::Chr(end) + "\r\n";
   }

    sb += " -";
    for(int i = 0; i < length; i++)
    {
        sb += "-";
    }
    sb += "- \r\n";

    for(auto& cowline : cowlines)
    {
        for(int i = 0; i < length + 4; i++)
        {
            sb += " ";
        }
        sb += cowline + "\r\n";
    }

    return sb;
}