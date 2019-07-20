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

#include "HelpCommand.h"
#include "CommandInfo.h"
#include "UserContext.h"
#include "PeacegateProgramAsset.h"

void AHelpCommand::NativeRunCommand(UConsoleContext* InConsole, TArray<FString> InArguments)
{
    // Get all installed commands and programs.
    TArray<UPeacegateProgramAsset*> Programs = InConsole->GetUserContext()->GetInstalledPrograms();
    TArray<UCommandInfo*> Commands = InConsole->GetUserContext()->GetInstalledCommands();

    // user interface bullshitfucks
    InConsole->ResetFormatting();
    InConsole->SetBold(true);
    InConsole->SetItalic(true);
    InConsole->WriteLine(NSLOCTEXT("Help", "HelpTitle", "Help Command"));
    InConsole->WriteLine(NSLOCTEXT("Help", "HelpUnderline", "--------------\n"));
    InConsole->ResetFormatting();

    // All the descriptions of each command name.
    TMap<FString, FString> NameMap;

    // Maximum length of each name.
    int NameLength = 0;

    for(auto Program : Programs)
    {
        // Add it to the list of shit to display.
        NameMap.Add(Program->ID.ToString(), Program->Summary.ToString());

        // Update the length if we need to.
        if(NameLength < Program->ID.ToString().Len())
            NameLength = Program->ID.ToString().Len();
    }
    
    for(auto Command : Commands)
    {
        // Add it to the list of shit to display.
        NameMap.Add(Command->ID.ToString(), Command->Summary.ToString());

        // Update the length if we need to.
        if(NameLength < Command->ID.ToString().Len())
            NameLength = Command->ID.ToString().Len();
    }

    // Get all the added names.
    TArray<FString> Keys;
    NameMap.GetKeys(Keys);

    // Loop through them.
    for(auto& Key : Keys)
    {
        InConsole->SetColors(EConsoleColor::Cyan, EConsoleColor::Black);
        InConsole->Write(FText::FromString(Key));
        InConsole->ResetFormatting();
        int Spaces = (NameLength - Key.Len()) + 1;
        for(int i = 0; i < Spaces; i++)
            InConsole->Write(FText::FromString(" "));
        InConsole->SetItalic(true);
        InConsole->WriteLine(FText::FromString(NameMap[Key]));
        InConsole->ResetFormatting();
    }
    
    this->Complete();
}