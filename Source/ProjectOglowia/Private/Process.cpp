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

#include "Process.h"
#include "ProcessManager.h"

void UProcess::KillInternal(bool NotifyProcessManager)
{
    // Commit suicide.
    this->Dead = true;

    // Fire an event that we're dead.
    this->OnKilled.Broadcast();

    // Kill and destroy all child processes.
    while(this->ChildProcesses.Num())
    {
        this->ChildProcesses[0]->KillInternal(false);
    }

    this->OnTimeToEnd.Broadcast();

    if(NotifyProcessManager)
        this->ProcessManager->ProcessKilled();
}

void UProcess::Kill()
{
    this->KillInternal(true);
}

void UProcess::CullDeadChildren()
{
    TArray<int> IndexesToRemove;

    for(int i = 0; i < this->ChildProcesses.Num(); i++)
    {
        UProcess* Child = this->ChildProcesses[i];
        if(Child->Dead)
            IndexesToRemove.Push(i);
        Child->CullDeadChildren();
    }

    while(IndexesToRemove.Num())
    {
        int Last = IndexesToRemove.Pop();
        this->ChildProcesses.RemoveAt(Last);
    }
}

bool UProcess::Fork(FString FilePath, TArray<FString> InArguments)
{
    return false;
}

void UProcess::Initialize(UProcessManager* OwningProcessManager, int InUserID, FString InPath, FString InName)
{
    // Set up all the things we're told about.
    this->ProcessManager = OwningProcessManager;
    this->UserID = InUserID;
    this->Path = InPath;
    this->Name = InName;

    // Acquire a process ID.
    this->ProcessID = this->ProcessManager->GetNextProcessID();
}

int UProcess::GetProcessID()
{
    return this->ProcessID;
}