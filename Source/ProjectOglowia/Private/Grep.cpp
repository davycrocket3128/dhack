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

#include "Grep.h"
#include "Regex.h"

void AGrep::NativeRunCommand(UConsoleContext* InConsole, TArray<FString> InArguments) {
    FString PatternText = this->ArgumentMap["<pattern>"]->AsString();

    FRegexPattern Pattern(PatternText);

    FString Line;
    while(InConsole->GetLine(Line)) {
        FRegexMatcher Matcher(Pattern, Line);

        TArray<int> indexes;
        while(Matcher.FindNext()) {
            indexes.Add(Matcher.GetMatchBeginning());
            indexes.Add(Matcher.GetMatchEnding());
        }

        if(!indexes.Num()) continue;

        bool isMatching = false;
        for(int i = 0; i < Line.Len(); i++) {
            if(indexes.Num()) {
                if(indexes[0] == i) {
                    if(!isMatching) {
                        Line.InsertAt(i, "&c&*");
                    } else {
                        Line.InsertAt(i+1, "&7&r");
                    }
                    isMatching = !isMatching;
                }

                indexes.RemoveAt(0);
            }
        }

        InConsole->WriteLine(FText::FromString(Line));
    }

    this->Complete();
}