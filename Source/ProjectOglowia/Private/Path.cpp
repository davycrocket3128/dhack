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

#include "Path.h"

FString UPath::GetPathSeparatorChar() {
    return "/";
}

FString UPath::Combine(TArray<FString> DirectoryNames) {
    FString Separator = UPath::GetPathSeparatorChar();
    FString PathRet = "";
    for(int i = 0; i < DirectoryNames.Num(); i++) {
        FString Name = DirectoryNames[i];
        // Split the name into a list of path names to handle a user being stupid.
        TArray<FString> SplitNames = UPath::Split(Name);
        for(int j = 0; j < SplitNames.Num(); j++) {
            FString SplitName = SplitNames[j];
            PathRet += Separator + SplitName;
        }
    }
    return PathRet;
}

TArray<FString> UPath::Split(FString InPath) {
    FString Separator = UPath::GetPathSeparatorChar();
    TArray<FString> Names;
    FString Current;
    for(int i = 0; i <= InPath.Len(); i++) {
        bool IsSeparator = false;
        TCHAR Char = '\0';
        if(i == InPath.Len()) {
            IsSeparator = true;
        } else {
            Char = InPath[i];
            IsSeparator = FString::Chr(Char) == Separator;
        }
        if(IsSeparator) {
            if(Current.TrimStartAndEnd().Len()) {
                Names.Add(Current.TrimStartAndEnd());
                Current = "";
            }
            continue;
        } else {
            Current += Char;
        }
    }
    return Names;
}