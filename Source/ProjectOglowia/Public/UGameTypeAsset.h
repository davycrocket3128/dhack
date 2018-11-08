// Copyright (c) 2018 The Peacenet & Alkaline Thunder.

#pragma once

#include "CoreMinimal.h"
#include "ECountry.h"
#include "UGameTypeAsset.generated.h"

USTRUCT(BlueprintType)
struct PROJECTOGLOWIA_API FPeacenetGameTypeInfo
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FText DefaultPlayerCharacterName;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FString Name;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FString Description;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	UTexture2D* Icon;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	bool AllowUserCustomizableWorldSpawn = false;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	ECountry SpawnCountry;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FString WorldSeed;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	bool EnableMissions = false;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	bool UnlockAllProgramsByDefault = false;
};

/**
 * An asset that describes a Peacenet local virtual machine (singleplayer game mode)
 */
UCLASS(Blueprintable)
class PROJECTOGLOWIA_API UPeacenetGameTypeAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FPeacenetGameTypeInfo Info;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool EnableTutorial = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool EnablePeacegateFirstTimeSetupIntroText = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FText> PeacegateFirstTimeSetupIntroText;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FText> PeacegatePostSetupIntroText;
};