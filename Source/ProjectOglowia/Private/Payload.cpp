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

#include "Payload.h"
#include "NetworkedConsoleContext.h"

void UPayload::Disconnect() {
    // Kill the gsfconsole monitor process when we disconnect if it isn't dead already.
    if(LocalProcess && !LocalProcess->IsDead()) {
        LocalProcess->Kill();
    }
    
    // Only disconnect if we still have access to the local process.
    if(LocalProcess) {
        LocalProcess = nullptr;
        return this->Disconnected.Broadcast();    
    }
}

void UPayload::DeployPayload(UConsoleContext* OriginConsole, UUserContext* OriginUser, UUserContext* TargetUser, UProcess* OwningLocalProcess) {
    // Create a local process so that, when the owning process is killed, we disconnect.
    this->LocalProcess = OwningLocalProcess->Fork("gsfconsole-payload-monitor");

    // Make sure that, if the local process gets killed before we do, we disconnect.
    TScriptDelegate<> LocalProcessEnded;
    LocalProcessEnded.BindUFunction(this, "Disconnect");
    LocalProcess->OnKilled.Add(LocalProcessEnded);

    // Create a networked console context that outputs to the origin but uses the hacked user as a means of
    // gaining a Peacegate context.
    UNetworkedConsoleContext* HackerContext = NewObject<UNetworkedConsoleContext>();
    HackerContext->SetupNetworkedConsole(OriginConsole, TargetUser);

    // This console is passed to the deriving payload methods - the payload is thus run in the context of the hacked system.
    this->NativePayloadDeployed(HackerContext, OriginUser, TargetUser);
    this->OnPayloadDeployed(HackerContext, OriginUser, TargetUser);
}

UProcess* UPayload::GetLocalProcess() {
    return this->LocalProcess;
}