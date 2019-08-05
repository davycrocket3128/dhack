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
#include "Engine/DataAsset.h"
#include "TextProperty.h"
#include "DaemonType.h"
#include "PeacegateDaemon.h"
#include "DaemonInfoAsset.generated.h"

UCLASS(BlueprintType, Blueprintable, Abstract)
class PROJECTOGLOWIA_API UDaemonInfoAsset : public UDataAsset
{
    GENERATED_BODY()

public:
    // A unique name for the daemon, used for its Peacegate process name as well as
    // the <service> argument in the systemctl command.  MUST BE UNIQUE FROM OTHER DAEMONS!
    // If two daemons have the same Name, the game will debug-assert and refuse to launch.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Daemon Info")
    FName Name;

    // A "friendly" title for the daemon.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Daemon Info")
    FText FriendlyName;

    // A description to the player of what this daemon is for.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Daemon Info")
    FText Description;

    // What types of computers should this daemon ever spawn on?
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Daemon Info")
    EDaemonType DaemonType = EDaemonType::AllSystems;

    // The C++ or Blueprint Peacegate Daemon class that represents the daemon that will be spawned in on
    // all systems where this daemon is enabled.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Daemon Class")
    TSubclassOf<UPeacegateDaemon> DaemonClass;
};