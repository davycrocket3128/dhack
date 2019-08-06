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

#pragma once

#include "DaemonManager.h"
#include "SystemContext.h"
#include "PeacenetWorldStateActor.h"

bool UDaemonManager::IsDaemonRunning(FName InDaemonName)
{
    for(auto Daemon : this->ActiveDaemons)
    {
        if(Daemon->GetName() == InDaemonName)
            return true;
    }
    return false;
}

void UDaemonManager::Initialize(USystemContext* InSystem, UProcess* InSystemProcess)
{
    // Check system and process validity.
    check(InSystem);
    check(InSystemProcess);

    // Make sure the system process is not dead.
    check(!InSystemProcess->IsDead());

    // Store a reference to the system!
    this->SystemContext = InSystem;
    
    // If the system process we've previously stored is valid and not dead, kill it.
    if(this->SystemProcess)
    {
        if(!this->SystemProcess->IsDead())
            this->SystemProcess->Kill();
        this->SystemProcess = nullptr;
    }

    // Store our system process.
    this->SystemProcess = InSystemProcess;

    // Grab the game instance from Peacenet World State
    this->GameInstance = Cast<UPeacenetGameInstance>(this->SystemContext->GetPeacenet()->GetWorld()->GetGameInstance());

    // Get all available daemons for the system.
    this->AvailableDaemons = this->GameInstance->GetDaemonsForSystem(this->SystemContext);

    // Start all enabled daemons.
    this->StartEnabledDaemons();
}

bool UDaemonManager::IsDaemonEnabled(FName InName)
{
    return !this->SystemContext->GetComputer().DisabledDaemons.Contains(InName);
}

void UDaemonManager::StartEnabledDaemons()
{
    for(auto Daemon : this->AvailableDaemons)
    {
        if(this->IsDaemonEnabled(Daemon.Name))
        {
            UPeacegateDaemon* DaemonInstance = this->CreateDaemon(Daemon.Name);

            ActiveDaemons.Add(DaemonInstance);

            // Start the daemon.
            this->StartDaemon(DaemonInstance);
        }
    }
}

UPeacegateDaemon* UDaemonManager::CreateDaemon(FName InName)
{
    for(auto Daemon : this->AvailableDaemons)
    {
        if(Daemon.Name == InName)
        {
            UPeacegateDaemon* Instance = NewObject<UPeacegateDaemon>(this, Daemon.DaemonClass);
            Instance->SetMetadata(this, Daemon.Name, Daemon.FriendlyName, Daemon.Description);
            return Instance;
        }
    }
    return nullptr;
}

void UDaemonManager::StartDaemon(UPeacegateDaemon* InDaemon)
{
    // The daemon will create its process on its own, all we need to do is start it.
    InDaemon->Start();
}

UProcess* UDaemonManager::Fork(FString InProcessName)
{
    return this->SystemProcess->Fork(InProcessName);
}

void UDaemonManager::DaemonStoppedGracefully(UPeacegateDaemon* InDaemon)
{
    this->ActiveDaemons.Remove(InDaemon);
}

APeacenetWorldStateActor* UDaemonManager::GetPeacenet()
{
    return this->SystemContext->GetPeacenet();
}

USystemContext* UDaemonManager::GetSystemContext()
{
    return this->SystemContext;
}

bool UDaemonManager::IsActive()
{
    return this->SystemProcess && !this->SystemProcess->IsDead();
}

void UDaemonManager::StartDaemonByName(FName InName)
{
    for(auto Daemon : this->ActiveDaemons)
    {
        if(Daemon->GetName() == InName)
            return;
    }

    for(auto DaemonInfo : this->AvailableDaemons)
    {
        if(DaemonInfo.Name == InName)
        {
            UPeacegateDaemon* Daemon = this->CreateDaemon(InName);
            ActiveDaemons.Add(Daemon);
            this->StartDaemon(Daemon);
            return;
        }
    }
}

void UDaemonManager::StopDaemonByName(FName InName)
{
    for(int i = 0; i < ActiveDaemons.Num(); i++)
    {
        UPeacegateDaemon* Daemon = ActiveDaemons[i];
        if(Daemon->GetName() == InName)
        {
            Daemon->Stop();
            return;
        }
    }
}

void UDaemonManager::RestartDaemonByName(FName InName)
{
    this->StopDaemonByName(InName);
    this->StartDaemonByName(InName);
}

void UDaemonManager::EnableDaemon(FName InDaemon)
{
    if(this->SystemContext->GetComputer().DisabledDaemons.Contains(InDaemon))
        this->SystemContext->GetComputer().DisabledDaemons.Remove(InDaemon);
}

void UDaemonManager::DisableDaemon(FName InDaemon)
{
    if(!this->SystemContext->GetComputer().DisabledDaemons.Contains(InDaemon))
        this->SystemContext->GetComputer().DisabledDaemons.Add(InDaemon);
}