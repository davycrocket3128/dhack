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

void AKillProcessCommand::NativeRunCommand(UConsoleContext* InConsole, TArray<FString> InArguments)
{
    // Get the process id/name specified by the player.
    FString ProcessIDArg = this->ArgumentMap["<pid>"]->AsString();

    // Loop through all the processes active in the system.
    for(auto Process : InConsole->GetUserContext()->GetRunningProcesses())
    {
        // If the name or ID matches, kill the process.
        if(Process.PID != this->GetProcessID() && FString::FromInt(Process.PID) == ProcessIDArg || Process.ProcessName == ProcessIDArg)
        {
            InConsole->WriteLine(FText::Format(NSLOCTEXT("Kill", "Success", "&*SUCCESS:&r Killed process &8{0}&7 (pid {1})"), FText::FromString(Process.ProcessName), FText::FromString(FString::FromInt(Process.PID))));
            InConsole->GetUserContext()->GetOwningSystem()->FinishProcess(Process.PID);
        }
    }

    // Kill ourselves.
    this->Complete();
}