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
#include "GameFramework/Actor.h"
#include "UDesktopWidget.h"
#include "EConnectionError.h"
#include "FManPage.h"
#include "EGovernmentAlertStatus.h"
#include "UPeacenetSaveGame.h"
#include "FGovernmentAlertInfo.h"
#include "UGameTypeAsset.h"
#include "PeacenetWorldStateActor.generated.h"

class UChatManager;
class USystemContext;
class UWallpaperAsset;
class UComputerService;
class UMarkovTrainingDataAsset;
class UProceduralGenerationEngine;
class UPeacegateProgramAsset;
class UTerminalCommand;
class UCommandInfo;
class UWindow;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FPlayerSystemContextReadyEvent, USystemContext*, InSystemContext);

UCLASS()
class PROJECTOGLOWIA_API APeacenetWorldStateActor : public AActor
{
	GENERATED_BODY()
	
public: // Constructors
	// Sets default values for this actor's properties
	APeacenetWorldStateActor();

private: // Properties
	UPROPERTY()
	UChatManager* ChatManager;

	UPROPERTY()
	TArray<UComputerService*> ComputerServices;

	UPROPERTY()
	UProceduralGenerationEngine* Procgen;

	UPROPERTY()
	TArray<USystemContext*> SystemContexts;

	UPROPERTY()
	TMap<int, FGovernmentAlertInfo> GovernmentAlertInfo;

public: //Properties
	UFUNCTION()
	bool ResolveHost(FString InHost, FComputer& OutComputer, EConnectionError& OutError);

	UPROPERTY()
	TArray<UMarkovTrainingDataAsset*> MarkovData;

	UPROPERTY()
	TArray<UWallpaperAsset*> Wallpapers;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Meta=(ExposeOnSpawn))
	TSubclassOf<UDesktopWidget> DesktopClass;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Meta = (ExposeOnSpawn))
	UPeacenetGameTypeAsset* GameType;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Meta = (ExposeOnSpawn))
	TSubclassOf<UWindow> WindowClass;

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

private: // Functions
	UFUNCTION()
	void LoadTerminalCommands();

	template<typename AssetType>
	bool LoadAssets(FName ClassName, TArray<AssetType*>& OutArray);

public:	// Functions
	FText GetTimeOfDay();

	UFUNCTION()
	bool GetOwningIdentity(FComputer& InComputer, int& OutIdentityID);

	UFUNCTION()
	USystemContext* GetSystemContext(int InIdentityID);

	UFUNCTION()
	TArray<UComputerService*> GetServicesFor(EComputerType InComputerType);

	UFUNCTION()
	void SaveWorld();

	UFUNCTION()
	FGovernmentAlertInfo GetAlertInfo(int InCharacterID);

	UFUNCTION(BlueprintCallable, Category = "Peacenet")
	void StartGame(TSubclassOf<UDesktopWidget> InDesktopClass, TSubclassOf<UWindow> InWindowClass);

	UFUNCTION()
	FString GetIPAddress(FComputer& InComputer);

	UFUNCTION()
	TArray<FPeacenetIdentity> GetAdjacentNodes(FPeacenetIdentity& InIdentity);

	UFUNCTION()
	bool FindProgramByName(FName InName, UPeacegateProgramAsset*& OutProgram);

	UFUNCTION()
	bool IsPortOpen(FString InIPAddress, int InPort);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Network Map")
	bool ScanForServices(FString InIPAddress, TArray<FFirewallRule>& OutRules);

protected: // AActor overrides
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	virtual void EndPlay(const EEndPlayReason::Type InReason) override;
	
public:	// AActor Overrides
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	
public: // Static functions
	// Used by the Ubiquity menu to see if the "Boot existing OS" screen should show.
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Peacegate")
	static bool HasExistingOS();

	UFUNCTION(BlueprintCallable, Category = "Peacegate")
	static APeacenetWorldStateActor* LoadExistingOS(const APlayerController* InPlayerController);
};

