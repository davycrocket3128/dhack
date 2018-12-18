// Copyright (c) 2018 The Peacenet & Alkaline Thunder.

#pragma once

#include "CoreMinimal.h"
#include "FPeacenetIdentity.h"
#include "UWorldDebugUtils.generated.h"

class USystemContext;

/**
 * Contains methods for debugging The Peacenet in the game's UI.
 */
UCLASS(BlueprintType)
class PROJECTOGLOWIA_API UWorldDebugUtils : public UObject
{
	GENERATED_BODY()

public:
	UFUNCTION()
	static bool RetrieveSystemContext(UObject* InContextObject, USystemContext*& OutSystemContext);

	UFUNCTION(BlueprintCallable, Category = "Debug|World Utils", meta=(DefaultToSelf="InContextObject"))
	static bool GetNPCs(UObject* InContextObject, TArray<FPeacenetIdentity>& OutCharacters);
};
