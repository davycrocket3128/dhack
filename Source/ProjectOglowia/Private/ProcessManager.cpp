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

void UProcessManager::Initialize(USystemContext* InSystem) {
    // So that we have a Peacegate context.
    this->OwningSystem = InSystem;

    // Create a root process.  All processes run under this process, and if it halts, the
    // game "crashes."
    this->RootProcess = NewObject<UProcess>();
    this->RootProcess->Initialize(this, 0, "", "peacegate");

    // Make the system crash when the process exits.
    TScriptDelegate<> CrashDelegate;
    CrashDelegate.BindUFunction(this->OwningSystem, "RootProcessKilled");
    this->RootProcess->OnKilled.Add(CrashDelegate);
}

void UProcessManager::RootProcessKilled() {
    // Restart the root process:
    this->Initialize(this->OwningSystem);

    // Proper handling of the root process being killed should involve any
    // user-session processes being killed too, which puts Peacegate in a fucked
    // state since everything essentially dies and nothing can fork.
    //
    // Therefore, the system context should destroy all user contexts in order to
    // prevent accessing the dead user-session processes.
    //
    // This should be handled by the Crash() method.
    //
    // Cause our owning system to kernel panic (for visual effect.)
    this->OwningSystem->Crash();
}

int UProcessManager::GetNextProcessID() {
    int id = this->NextProcessID;
    this->NextProcessID++;
    return id;
}

void UProcessManager::ProcessKilled() {
    this->RootProcess->CullDeadChildren();
}

UProcess* UProcessManager::CreateProcess(FString Name) {
    return this->CreateProcessAs(Name, this->RootProcess->UserID);
}

UProcess* UProcessManager::CreateProcessAs(FString Name, int UserID) {
    UProcess* Process = NewObject<UProcess>();
    Process->Initialize(this, UserID, "", Name);
    Process->Parent(this->RootProcess);
    return Process;
}

TArray<UProcess*> UProcessManager::GetAllProcesses() {
    TArray<UProcess*> Ret;

    // Tell the root process to collect itself into the array.
    this->RootProcess->CollectProcesses(Ret);

    return Ret;
}

TArray<UProcess*> UProcessManager::GetProcessesForUser(int UserID) {
    // Get all of the processes that are currently running...
    TArray<UProcess*> AllProcesses = this->GetAllProcesses();

    // If we're root then we own all processes because we're root and root
    // can do everything.
    if(UserID == 0) {
        return AllProcesses;
    }

    // Otherwise, we only get to see processes that have our user ID.
    TArray<UProcess*> OurProcesses;
    for(auto Process : AllProcesses) {
        if(Process->UserID == UserID) {
            OurProcesses.Add(Process);
        }
    }
    return OurProcesses;
}

bool UProcessManager::KillProcess(int ProcessID, int UserID, EProcessResult& OutKillResult) {
    // The process to kill...
    UProcess* Process = nullptr;

    // Try to get the process by ID.  If we can't then we'll return false with an error.
    if(this->GetProcess(ProcessID, UserID, Process, OutKillResult)) {
        // Kill the process.
        Process->Kill();
        return true;
    }
    
    return false;
}

bool UProcessManager::GetProcess(int ProcessID, int UserID, UProcess*& OutProcess, EProcessResult& OutProcessResult) {
    // Default everything to zero.
    OutProcessResult = EProcessResult::Success;
    OutProcess = nullptr;

    // Look through all of the processes to find the right one.
    for(auto Process: this->GetAllProcesses()) {
        // Check the ID.
        if(Process->ProcessID == ProcessID) {
            // If we're root or the user id matches then we can say we found one.
            if(UserID == 0 || Process->UserID == UserID) {
                OutProcess = Process;
            } else {
                OutProcessResult = EProcessResult::PermissionDenied;
            }

            // Break the loop.
            break;
        }
    }

    // If the process is still nullptr, we have not found a process yet.
    if(!OutProcess && OutProcessResult == EProcessResult::Success) {
        OutProcessResult = EProcessResult::ProcessNotRunning;
    }

    // Return whether we succeeded.
    return OutProcessResult == EProcessResult::Success;
}

bool UProcessManager::IsActive() {
    return this->RootProcess && !this->RootProcess->IsDead();
}