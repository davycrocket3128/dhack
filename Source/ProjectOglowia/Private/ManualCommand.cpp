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

#include "ManualCommand.h"

void AManualCommand::NativeRunCommand(UConsoleContext* InConsole, TArray<FString> InArguments) {
    // Get the name of the manual page to find.
    FString ManualName = this->ArgumentMap["<manualPage>"]->AsString();

    // List of "likely wanted" pages in case one isn't found.
    TArray<FManualPage> LikelyPages;

    // Go through every manual page.
    for(auto& ManualPage : InConsole->GetUserContext()->GetPeacenet()->GetManualPages()) {
        if(ManualPage.ID.ToString() == ManualName) {
            // We have a match.
            InConsole->SetForegroundColor(EConsoleColor::Yellow);
            InConsole->SetBold(true);
            InConsole->WriteLine(ManualPage.FullName);
            for(int i = 0; i < ManualPage.FullName.ToString().Len(); i++) {
                InConsole->Write(FText::FromString("="));
            }
            InConsole->SetBold(false);
            InConsole->ResetForegroundColor();

            InConsole->WriteLine(FText::GetEmpty());

            InConsole->SetBold(true);
            InConsole->SetUnderline(true);
            InConsole->WriteLine(NSLOCTEXT("Manual", "Summary", "Summary"));
            InConsole->SetBold(false);
            InConsole->SetUnderline(false);

            InConsole->WriteLine(ManualPage.Summary);
            
            InConsole->WriteLine(FText::GetEmpty());
            
            for(auto Metadata : ManualPage.ManualMetadata) {
                InConsole->SetBold(true);
                InConsole->SetUnderline(true);
                InConsole->WriteLine(Metadata.Title);
                InConsole->SetBold(false);
                InConsole->SetUnderline(false);
                
                InConsole->WriteLine(Metadata.Content);
                InConsole->WriteLine(FText::GetEmpty());    
            }
            this->Complete();
            return;
        } else {
            if(ManualPage.ID.ToString().ToLower().Contains(ManualName.ToLower())) {
                LikelyPages.Add(ManualPage);
                continue;
            }
            if(ManualPage.FullName.ToString().ToLower().Contains(ManualName.ToLower())) {
                LikelyPages.Add(ManualPage);
                continue;
            }

        }
    }

    InConsole->SetBold(true);
    InConsole->SetItalic(true);
    InConsole->SetForegroundColor(EConsoleColor::Red);
    InConsole->WriteLine(FText::Format(NSLOCTEXT("Manual", "NoPagesFound", "No manual page named \"{0}\" was found."), FText::FromString(ManualName)));
    
    if(!LikelyPages.Num()) {
        InConsole->ResetFormatting();
        this->Complete();
        return;
    }
    
    InConsole->WriteLine(NSLOCTEXT("Manual", "Likelies", "However, these manual pages might have what you're looking for:\r\n"));
    InConsole->ResetFormatting();
    InConsole->SetBold(true);
    InConsole->SetForegroundColor(EConsoleColor::Yellow);
    for(auto ManualPage : LikelyPages) {
        InConsole->WriteLine(FText::Format(NSLOCTEXT("Manual", "LikelyPage", " - {0}"), FText::FromName(ManualPage.ID)));
    }
    InConsole->WriteLine(FText::GetEmpty());
    InConsole->ResetFormatting();
    this->Complete();
}