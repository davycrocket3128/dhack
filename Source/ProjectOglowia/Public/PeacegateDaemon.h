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

#include "CoreMinimal.h"
#include "Text.h"
#include "TextProperty.h"
#include "Process.h"
#include "PeacegateDaemon.generated.h"

class UDaemonManager;
class APeacenetWorldStateActor;

UCLASS(Blueprintable, BlueprintType, Abstract)
class PROJECTOGLOWIA_API UPeacegateDaemon : public UObject
{
    GENERATED_BODY()

private:
    UPROPERTY()
    bool IsGracefulStop = false;

    UPROPERTY()
    FName Name;

    UPROPERTY()
    FText FriendlyName;

    UPROPERTY()
    FText Description;

    UPROPERTY()
    UDaemonManager* DaemonManager;

    UPROPERTY()
    UProcess* DaemonProcess;

protected:
    UFUNCTION()
    APeacenetWorldStateActor* GetPeacenet();

    UFUNCTION()
    USystemContext* GetSystemContext();

    UFUNCTION()
    void StopInternal();

    // C++ events.
    virtual void NativeStart() {}
    virtual void NativeStop() {}
    virtual void NativeTick(float InDeltaTime) {}

    // Blueprint events:
    UFUNCTION(BlueprintImplementableEvent, Category = "System Daemon")
    void OnStart();

    UFUNCTION(BlueprintImplementableEvent, Category = "System Daemon")
    void OnStop();

    UFUNCTION(BlueprintImplementableEvent, Category = "System Daemon")
    void OnTick(float InDeltaTime);

public:
    // Event dispatchers.
    UFUNCTION()
    void Start();

    UFUNCTION()
    void Stop();

    UFUNCTION()
    void Restart();

    UFUNCTION()
    void Tick(float InDeltaTime);

    // Links the daemon to a daemon manager and sets the daemon metadata.  Should only be called once, by the daemon manager.
    UFUNCTION()
    void SetMetadata(UDaemonManager* InDaemonManager, FName InName, FText InFriendlyName, FText InDescription);

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "System Daemon")
    bool IsActive();
};