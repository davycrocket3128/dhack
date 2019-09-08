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
#include "SystemUpgrade.h"
#include "Sex.h"
#include "PeacenetIdentity.generated.h"

/**
 * Represents a type of Peacenet character identity.
 */
UENUM(BlueprintType)
enum class EIdentityType : uint8
{
	Player,
	NonPlayer,
	Story
};

/**
 * Represents a character's identity within The Peacenet.
 */
USTRUCT(BlueprintType)
struct PROJECTOGLOWIA_API FPeacenetIdentity
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	int ID;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FString CharacterName;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FString EmailAddress;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FString PreferredAlias;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	int Skill;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	int ComputerID;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float Reputation;	

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	TArray<USystemUpgrade*> UnlockedUpgrades;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	EIdentityType CharacterType;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	bool IsMissionImportant = false;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	ESex Sex = ESex::Male;
};
