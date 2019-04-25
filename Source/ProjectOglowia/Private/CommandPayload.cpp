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

#include "CommandPayload.h"
#include "TerminalCommand.h"
#include "PeacenetWorldStateActor.h"

void UCommandPayload::NativePayloadDeployed(UConsoleContext* Console, UUserContext* OriginUser, UUserContext* TargetUser)
{
    check(this->Command);

    // Spawn the terminal command actor - similar to how USystemContext does it.
    FVector Location(0.0f, 0.0f, 0.0f);
	FRotator Rotation(0.0f, 0.0f, 0.0f);
 	FActorSpawnParameters SpawnInfo;

	ATerminalCommand* CommandInstance = OriginUser->GetPeacenet()->GetWorld()->SpawnActor<ATerminalCommand>(this->Command->CommandClass, Location, Rotation, SpawnInfo);
	CommandInstance->CommandInfo = this->Command;

    // Disconnect when the command finishes.
    TScriptDelegate<> DisconnectedEvent;
    DisconnectedEvent.BindUFunction(this, "Disconnect");
    CommandInstance->Completed.Add(DisconnectedEvent);

    // Run the command in the context of the hacked user.
    CommandInstance->RunCommand(Console, { this->Command->ID.ToString() });
}