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

#include "ProcessManager.h"
#include "Process.h"
#include "SystemContext.h"

void UProcessManager::Initialize(USystemContext* InSystem)
{
    // So that we have a Peacegate context.
    this->OwningSystem = InSystem;

    // Create a root process.  All processes run under this process, and if it halts, the
    // game "crashes."
    this->RootProcess = NewObject<UProcess>();
    this->RootProcess->Initialize(this, 0, "", "peacegate");

    // Make the system crash when the process exits.
    TScriptDelegate<> CrashDelegate;
    CrashDelegate.BindUFunction(this->OwningSystem, "Crash");
    this->RootProcess->OnKilled.Add(CrashDelegate);
}

int UProcessManager::GetNextProcessID()
{
    int id = this->NextProcessID;
    this->NextProcessID++;
    return id;
}

void UProcessManager::ProcessKilled()
{
    this->RootProcess->CullDeadChildren();
}

UProcess* UProcessManager::CreateProcess(FString Name)
{
    return this->CreateProcessAs(Name, this->RootProcess->UserID);
}

UProcess* UProcessManager::CreateProcessAs(FString Name, int UserID)
{
    UProcess* Process = NewObject<UProcess>();
    Process->Initialize(this, UserID, "", Name);
    Process->Parent(this->RootProcess);
    return Process;
}

TArray<UProcess*> UProcessManager::GetAllProcesses()
{
    TArray<UProcess*> Ret;

    // Tell the root process to collect itself into the array.
    this->RootProcess->CollectProcesses(Ret);

    return Ret;
}

TArray<UProcess*> UProcessManager::GetProcessesForUser(int UserID)
{
    // Get all of the processes that are currently running...
    TArray<UProcess*> AllProcesses = this->GetAllProcesses();

    // If we're root then we own all processes because we're root and root
    // can do everything.
    if(UserID == 0)
        return AllProcesses;

    // Otherwise, we only get to see processes that have our user ID.
    TArray<UProcess*> OurProcesses;
    for(auto Process : AllProcesses)
    {
        if(Process->UserID == UserID)
        {
            OurProcesses.Add(Process);
        }
    }
    return OurProcesses;
}

bool UProcessManager::KillProcess(int ProcessID, int UserID, EKillResult& OutKillResult)
{
    // Default to success so we don't have to manually set it later.
    OutKillResult = EKillResult::Success;

    // Did we kill a process?
    bool DidWeCommitMurder = false;

    // Go through all of the processes that are running to find the one with the matching ID.
    // We can't use the GetProcessesForUser() method in this case, as we need to see non-owned
    // processes so we can signal "permission denied" on attempts to kill them.
    for(auto Process : this->GetAllProcesses())
    {
        if(Process->ProcessID == ProcessID)
        {
            // We've found the process we want to kill.  Now check the user ID.
            // If we're not root, only kill if the IDs match.
            if(UserID == 0 || Process->UserID == UserID)
            {
                // Commit actual murder!
                Process->Kill();
            }
            else 
            {
                // Permission denied.
                OutKillResult = EKillResult::PermissionDenied;
            }

            // Mark that we've tried to kill, and break the loop.
            DidWeCommitMurder = true;
            break;
        }
    }

    // Process wasn't found if we didn't commit murder.
    if(!DidWeCommitMurder)
        OutKillResult = EKillResult::ProcessNotRunning;

    // Return whether or not we succeeded.
    return OutKillResult == EKillResult::Success;
}