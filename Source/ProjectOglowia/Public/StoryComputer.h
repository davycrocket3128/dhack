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
#include "Engine/DataAsset.h"
#include "StoryCharacter.h"
#include "ExplicitComputerService.h"
#include "LootableSpawnInfo.h"
#include "StoryComputer.generated.h"

/**
 * Represents a Peacegate OS computer used in the game's storyline, during missions, etc. that gets spawned
 * before all other NPC computers.
 */
UCLASS(BlueprintType, Blueprintable)
class PROJECTOGLOWIA_API UStoryComputer : public UDataAsset {
    GENERATED_BODY()

public:
    /**
     * The story character that owns this computer.  If none is specified, the computer is treated as owner-less.
     */
    UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Peacenet Identity")
    UStoryCharacter* OwningCharacter = nullptr;

    /**
     * The hostname of the computer.  Will be "localhost" if left blank, and will be Unix-ified when the game
     * spawns the computer (i.e, "My Cool Computer" will become "my-cool-computer").
     */
    UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Basic Metadata")
    FString Hostname = "";

    /**
     * The public domain name of the computer (e.x. bitphoenixsoftware.com) that can be used in place of the auto-generated static
     * IP address when hacking.  If left blank, the game will not generate a domain name for the computer.
     * 
     * The domain name will also be Unixified just like the hostname ("Cool Server Domain" becomes "cool-server-domain"), and, if no
     * top-levlel (.com, .net, .org, .gov, etc) domain is detected in the domain name, a random one will be chosen.
     */
    UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Basic Metadata")
    FString DomainName = "";

    /**
     * A list of usernames that will be spawned as Peacegate Users.  These users will NOT
     * be marked as sudoers.  If left empty, the game will yell at you in a dev build and
     * the game will refuse to spawn this computer in a release build.
     */
    UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Basic Metadata")
    TArray<FString> Users;

    /**
     * A list of exploitable Peacegate services that this computer will spawn with.  If left blank,
     * the game will pick some at random in the exact same way it does for NPC computers.  Invalid
     * services will be ignored.
     */
    UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Basic Metadata")
    TArray<FExplicitComputerService> Services;

    /**
     * A list of files to spawn on the computer.  This is useful for missions that require the
     * player to retrieve a file.
     */
    UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "File System")
    TArray<FLootableSpawnInfo> LootableFiles;
};