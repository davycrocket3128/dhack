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

#include "PeacegateDaemon.h"
#include "DaemonManager.h"
#include "PeacenetWorldStateActor.h"

APeacenetWorldStateActor* UPeacegateDaemon::GetPeacenet()
{
    return this->DaemonManager->GetPeacenet();
}

USystemContext* UPeacegateDaemon::GetSystemContext()
{
    return this->DaemonManager->GetSystemContext();
}

void UPeacegateDaemon::Start()
{
    if(!this->IsActive())
    {
        // Tie ourselves to the world simulation tick:
        TScriptDelegate<> WorldUpdate;
        WorldUpdate.BindUFunction(this, "Tick");
        if(!this->DaemonManager->GetPeacenet()->OnSimulationTick.Contains(WorldUpdate))
            this->DaemonManager->GetPeacenet()->OnSimulationTick.Add(WorldUpdate);

        // Create a process for ourselves.
        this->DaemonProcess = this->DaemonManager->Fork(this->Name.ToString());

        // Handle process kills.
        TScriptDelegate<> Killed;
        Killed.BindUFunction(this, "StopInternal");
        this->DaemonProcess->OnKilled.Add(Killed);

        // Allow derived class to do things when the daemon starts.
        this->NativeStart();
        this->OnStart();
    }
}

void UPeacegateDaemon::Stop()
{
    if(this->IsActive())
    {
        // Mark that the kill is graceful.
        this->IsGracefulStop = true;

        // Kill our process.
        this->DaemonProcess->Kill();
    }
}

void UPeacegateDaemon::StopInternal()
{
    // Allow derived classes to do things when we stop.
    this->NativeStop();
    this->OnStop();

    // If the shutdown was graceful then we'll detach ourselves from the daemon manager and game simulation tick,
    // and gracefully despawn.
    //
    // Otherwise we will restart.
    if(this->IsGracefulStop || !this->DaemonManager->IsActive())
    {
        // Stop any world simulation ticks of this daemon.
        TScriptDelegate<> WorldUpdate;
        WorldUpdate.BindUFunction(this, "Tick");
        this->DaemonManager->GetPeacenet()->OnSimulationTick.Remove(WorldUpdate);

        // Detach from the daemon manager.
        this->DaemonManager->DaemonStoppedGracefully(this);
        this->DaemonManager = nullptr;

        // Let Unreal GC take care of the process.
        this->DaemonProcess = nullptr;
    }
    else
    {
        // Restart the daemon.
        this->Restart();
    }

    // Reset grace state.
    this->IsGracefulStop = false;
}

void UPeacegateDaemon::Restart()
{
    // Stop the daemon if we're active...
    this->Stop();

    // Restart the daemon.
    this->Start();    
}

void UPeacegateDaemon::Tick(float InDeltaTime)
{
    // Only tick if we are active.
    if(this->IsActive())
    {
        this->NativeTick(InDeltaTime);
        this->OnTick(InDeltaTime);
    }
}

void UPeacegateDaemon::SetMetadata(UDaemonManager* InDaemonManager, FName InName, FText InFriendlyName, FText InDescription)
{
    this->DaemonManager = InDaemonManager;
    this->Name = InName;
    this->FriendlyName = InFriendlyName;
    this->Description = InDescription;
}

bool UPeacegateDaemon::IsActive()
{
    return this->DaemonProcess && !this->DaemonProcess->IsDead();
}

FName UPeacegateDaemon::GetName()
{
    return this->Name;
}