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

void AManualCommand::NativeRunCommand(UConsoleContext* InConsole, TArray<FString> InArguments)
{
    // Get the name of the manual page to find.
    FString ManualName = this->ArgumentMap["<manualName>"]->AsString();

    // List of "likely wanted" pages in case one isn't found.
    TArray<FManualPage> LikelyPages;

    // Go through every manual page.
    for(auto& ManualPage : InConsole->GetUserContext()->GetPeacenet()->GetManualPages())
    {
        if(ManualPage.ID.ToString() == ManualName)
        {
            // We have a match.
            InConsole->WriteLine("&*&3" + ManualPage.FullName.ToString());
            for(int i = 0; i < ManualPage.FullName.ToString().Len(); i++)
            {
                InConsole->Write("=");
            }
            InConsole->WriteLine("&r&7\r\n");
            InConsole->WriteLine("&f&*Summary&r&7");
            InConsole->WriteLine(ManualPage.Summary.ToString());
            InConsole->WriteLine("");
            for(auto Metadata : ManualPage.ManualMetadata)
            {
                InConsole->WriteLine("&f&*" + Metadata.Title.ToString() + "&r&7");
                InConsole->WriteLine(Metadata.Content.ToString());
                InConsole->WriteLine("");    
            }
            this->Complete();
            return;
        }
        else
        {
            if(ManualPage.ID.ToString().ToLower().Contains(ManualName.ToLower()))
            {
                LikelyPages.Add(ManualPage);
                continue;
            }
            if(ManualPage.FullName.ToString().ToLower().Contains(ManualName.ToLower()))
            {
                LikelyPages.Add(ManualPage);
                continue;
            }

        }
    }

    InConsole->WriteLine("No manual page named &3&*" + ManualName + "&7&r was found.");

    if(!LikelyPages.Num())
    {
        this->Complete();
        return;
    }
    
    InConsole->WriteLine("However, these manual pages might have what you're looking for:\r\n");
    for(auto ManualPage : LikelyPages)
    {
        InConsole->WriteLine(" - &3&*" + ManualPage.ID.ToString() + "&r&7");
    }
    InConsole->WriteLine("");
    this->Complete();
}