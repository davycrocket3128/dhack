// Copyright (c) 2018 The Peacenet & Alkaline Thunder.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "UDesktopWidget.h"
#include "UPeacenetSaveGame.h"
#include "UGameTypeAsset.h"
#include "PeacenetWorldStateActor.generated.h"

class UComputerTypeAsset;
class USystemContext;
class UPeacegateProgramAsset;
class UTerminalCommand;
class UCommandInfo;
class UWindow;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FPlayerSystemContextReadyEvent, USystemContext*, InSystemContext);

USTRUCT()
struct FManPage
{
	GENERATED_BODY()

public:
	UPROPERTY()
	FString Description;

	UPROPERTY()
	FString InternalUsage;

	UPROPERTY()
	FString FriendlyUsage;
};

UCLASS()
class PROJECTOGLOWIA_API APeacenetWorldStateActor : public AActor
{
	GENERATED_BODY()
	
private:
	UFUNCTION()
	void LoadTerminalCommands();

public:	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Meta=(ExposeOnSpawn))
	TSubclassOf<UDesktopWidget> DesktopClass;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Meta = (ExposeOnSpawn))
	UPeacenetGameTypeAsset* GameType;

	UFUNCTION()
	bool UpdateComputer(int InEntityID, FComputer& InComputer);
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Meta = (ExposeOnSpawn))
	TSubclassOf<UWindow> WindowClass;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Meta = (ExposeOnSpawn))
	UComputerTypeAsset* PlayerComputerType;

	UPROPERTY(BlueprintAssignable, Category = "Peacenet")
	FPlayerSystemContextReadyEvent PlayerSystemReady;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Meta = (ExposeOnSpawn))
	FString WorldSlot;

	UPROPERTY()
	TMap<FName, FManPage> ManPages;

	UPROPERTY()
	UPeacenetSaveGame* SaveGame;

	UPROPERTY()
	TArray<UPeacegateProgramAsset*> Programs;

	UPROPERTY()
	TMap<FName, UCommandInfo*> CommandInfo;

	// Sets default values for this actor's properties
	APeacenetWorldStateActor();

private:
	UPROPERTY()
	TArray<USystemContext*> SystemContexts;

	template<typename AssetType>
	bool LoadAssets(FName ClassName, TArray<AssetType*>& OutArray);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	

	UFUNCTION()
	bool FindProgramByName(FName InName, UPeacegateProgramAsset*& OutProgram);

	FText GetTimeOfDay();

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintCallable, Category = "Peacenet")
	void StartGame();
	
};
