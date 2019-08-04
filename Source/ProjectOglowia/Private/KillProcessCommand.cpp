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

#include "KillProcessCommand.h"

// kill - Kills the specified Peacegate process by ID.
//
// Usage:
//  kill <pid>
void AKillProcessCommand::NativeRunCommand(UConsoleContext* InConsole, TArray<FString> InArguments)
{
    // Check if the specified <pid> is a number and throw an error if it isn't.
    if(!this->ArgumentMap["<pid>"]->IsNumber())
    {
        InConsole->SetForegroundColor(EConsoleColor::Magenta);
        InConsole->WriteLine(NSLOCTEXT("Kill", "InvalidProcessID", "error: pid is not valid."));
        InConsole->ResetFormatting();
        this->Complete();
        return;
    }

    // Get the process ID number.
    int pid = this->ArgumentMap["<pid>"]->AsNumber();

    // Try to kill the process.
    EProcessResult KillResult;
    if(this->GetUserContext()->KillProcess(pid, KillResult))
    {
        // Success!
        InConsole->SetForegroundColor(EConsoleColor::Green);
        InConsole->WriteLine(FText::Format(NSLOCTEXT("Kill", "Killed", "[{0}] Killed."), FText::AsNumber(pid)));
        InConsole->ResetFormatting();
        this->Complete();
        return;
    }

    // If we get this far, there was an error.
    InConsole->SetForegroundColor(EConsoleColor::Red);
    InConsole->Write(FText::Format(NSLOCTEXT("Kill", "KillErrorStart", "kill: {0}: error: "), FText::AsNumber(pid)));

    switch(KillResult)
    {
        case EProcessResult::PermissionDenied:
            InConsole->WriteLine(NSLOCTEXT("Kill", "PermissionDenied", "permission denied."));
            break;
        case EProcessResult::ProcessNotRunning:
            InConsole->WriteLine(NSLOCTEXT("Kill", "ProcessNotRunning", "process not running."));
            break;
    }

    // All done.
    InConsole->ResetFormatting();

    // Kill ourselves.
    this->Complete();
}