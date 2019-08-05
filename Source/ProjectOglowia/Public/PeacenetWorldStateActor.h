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
#include "DesktopWidget.h"
#include "ConnectionError.h"
#include "AssetRegistry/Public/IAssetRegistry.h"
#include "ManualPage.h"
#include "AssetRegistry/Public/AssetRegistryModule.h"
#include "PeacenetSaveGame.h"
#include "TutorialPromptState.h"
#include "AlertManager.h"
#include "PeacenetWorldStateActor.generated.h"

class AMissionActor;
class UChatManager;
class USystemContext;
class UWallpaperAsset;
class UComputerService;
class UMarkovTrainingDataAsset;
class UProceduralGenerationEngine;
class UPeacegateProgramAsset;
class UTerminalCommand;
class UExploit;
class UCommandInfo;
class UPayloadAsset;
class UWindow;
class UMissionAsset;
class USystemUpgrade;

// Because literally anything that requires the world state to be friends with it is fucking clingy
// as fuck.
#define mystalkeris friend
#define myotherstalkeris friend


USTRUCT(BlueprintType)
struct FGameEventData
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadOnly)
	TMap<FString, FString> EventData;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FPlayerSystemContextReadyEvent, USystemContext*, InSystemContext);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FPeacenetMapUpdateEvent);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FMailMessageSendEvent);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FMissionFailureEvent, AMissionActor*, MissionState, const FText&, FailMessage);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FGameEventSent, FString, InEventName, FGameEventData, InEventData);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMissionCompleteEvent, UMissionAsset*, InMission);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FQuitRequestEvent);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FUpdateKnownHostsEvent);

UCLASS()
class PROJECTOGLOWIA_API APeacenetWorldStateActor : public AActor
{
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FWorldSimulationTickEvent, float, InDeltaTime);

	mystalkeris AMissionActor; // AMissionActor's more clingy to the fucking save file than I was to Nick Ensor.  That's...annoying....
	myotherstalkeris UMissionTask; // For the love of Kaylin...
	myotherstalkeris UProceduralGenerationEngine;

	GENERATED_BODY()
	
public: // Constructors
	// Sets default values for this actor's properties
	APeacenetWorldStateActor();

	UPROPERTY()
	FWorldSimulationTickEvent OnSimulationTick;

	UPROPERTY(BlueprintAssignable)
	FUpdateKnownHostsEvent UpdateKnownHosts;

	// All of the system upgrades that are available in the upgrade shop (formerly shiftorium in shiftos.)
	UPROPERTY()
	TArray<USystemUpgrade*> UserUnlockableUpgrades;

	UPROPERTY()
	FMissionCompleteEvent MissionCompleteEvent;

	UFUNCTION()
	void BroadcastMissionComplete(UMissionAsset* InMissionAsset);

	UFUNCTION()
	void SetGameStat(FName InStatName, int InValue);

	UFUNCTION()
	int GetGameStat(FName InStatName);

	UFUNCTION()
	void IncreaseGameStat(FName InStatName);

	UFUNCTION()
	float GetStealthiness(FPeacenetIdentity& InIdentity);

	UFUNCTION()
	void SetStealthiness(FPeacenetIdentity& InIdentity, float InValue);

private: // Properties
	UPROPERTY()
	UPeacenetSaveGame* SaveGame;

	UPROPERTY()
	AAlertManager* AlertManager;
	
	UPROPERTY()
	UTutorialPromptState* TutorialState;
	
	UPROPERTY()
	AMissionActor* CurrentMission;

	UPROPERTY()
	TArray<UExploit*> Exploits;

	UPROPERTY()
	TArray<UPayloadAsset*> Payloads;

	UPROPERTY()
	UChatManager* ChatManager;

	UPROPERTY()
	TArray<UComputerService*> ComputerServices;

	UPROPERTY()
	TArray<FManualPage> ManualPages;

	UPROPERTY()
	UProceduralGenerationEngine* Procgen;

	UPROPERTY()
	TArray<USystemContext*> SystemContexts;

public: //Properties
	UPROPERTY(BlueprintAssignable)
	FMissionFailureEvent MissionFailed;

	UPROPERTY(BlueprintAssignable)
	FQuitRequestEvent GameQuitRequested;

	UPROPERTY()
	FMailMessageSendEvent NewMailAdded;

	UPROPERTY()
	TArray<UMissionAsset*> Missions;

	UFUNCTION()
	void SendMissionMail(UMissionAsset* InMission);

	UPROPERTY()
	FPeacenetMapUpdateEvent MapsUpdated;

	UFUNCTION()
	void UpdateMaps();

	UFUNCTION()
	void SilentlyFailMission();

	UFUNCTION()
	FPeacenetIdentity& GetNewIdentity();

	UFUNCTION()
	FComputer& GetPlayerComputer();

	UFUNCTION()
	bool IdentityExists(int EntityID);

	UFUNCTION()
	FPeacenetIdentity& GetCharacterByID(int EntityID);

	UFUNCTION()
	TArray<FEmail> GetEmailMessages();

	UFUNCTION()
	FEmail& GetNewEmailMessage();

	UFUNCTION()
	void QuitGame();

	UFUNCTION()
	bool GetStoryCharacterID(UStoryCharacter* InStoryCharacter, int& OutEntityID);

	UFUNCTION(BlueprintCallable, BlueprintPure)
	bool IsMissionCompleted(UMissionAsset* InMission);

	UFUNCTION(BlueprintCallable)
	void SetSaveValue(FString InValueName, bool InValue);

	UFUNCTION(BlueprintCallable, BlueprintPure)
	TArray<USystemUpgrade*> GetAllSystemUpgrades();

	UFUNCTION(BlueprintCallable, BlueprintPure)
	UTutorialPromptState* GetTutorialState();

	UFUNCTION(BlueprintCallable, BlueprintPure)
	bool IsTutorialActive();

	UFUNCTION()
	void EndMission(bool DoGameUpdate);

	UFUNCTION()
	bool ResolveHost(FString InHost, FComputer& OutComputer, EConnectionError& OutError);

	UPROPERTY()
	TArray<UMarkovTrainingDataAsset*> MarkovData;

	UPROPERTY()
	TArray<UWallpaperAsset*> Wallpapers;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Meta=(ExposeOnSpawn))
	TSubclassOf<UDesktopWidget> DesktopClass;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Meta = (ExposeOnSpawn))
	TSubclassOf<UWindow> WindowClass;

	UPROPERTY(BlueprintAssignable, Category = "Peacenet")
	FPlayerSystemContextReadyEvent PlayerSystemReady;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Meta = (ExposeOnSpawn))
	FString WorldSlot;

	UPROPERTY()
	TArray<UPeacegateProgramAsset*> Programs;

	UPROPERTY()
	TMap<FName, UCommandInfo*> CommandInfo;

	UPROPERTY(BlueprintAssignable)
	FGameEventSent GameEventSent;

private: // Functions
	UFUNCTION()
	void LoadTerminalCommands();

public:
	template<typename AssetType>
	bool LoadAssets(FName ClassName, TArray<AssetType*>& OutArray);

private:
	FString ReverseDns(int ComputerID);

public:	// Functions
	FText GetTimeOfDay();

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Peacenet")
	bool IsNewGame();

	UFUNCTION()
	void FailMission(const FText& InFailMessage);

	UFUNCTION()
	int GetPlayerUserID();

	UFUNCTION()
	TArray<FPeacenetIdentity> GetCharacters();

	UFUNCTION()
	bool IsTrue(FString InSaveBoolean);

	UFUNCTION()
	bool IsIPAddress(FString InIPAddress);

	UFUNCTION()
	FComputer& GetComputerByID(int InEntityID);

	UFUNCTION()
	void ClearNonPlayerEntities();

	UFUNCTION()
	void RemoveDomain(FString InDomain);

	UFUNCTION()
	void ReplaceDomain(FString InDomain, FString NewDomain);

	UFUNCTION()
	void RegisterDomain(FString DomainName, FString DestinationHost);

	UFUNCTION()
	int GetWorldSeed();

	UFUNCTION()
	FComputer& GetNewComputer();

	UFUNCTION()
	bool CharacterNameExists(FString InCharacterName);

	UFUNCTION()
	TArray<FString> GetDomainNames();

	UFUNCTION()
	void AssignStoryCharacterID(UStoryCharacter* InStoryCharacter, FPeacenetIdentity& InIdentity);

	UFUNCTION()
	bool DnsResolve(FString InHost, FComputer& OutComputer, EConnectionError& OutError);

	UFUNCTION()
	TArray<UExploit*> GetExploits();

	UFUNCTION()
	bool IsInMission();

    UFUNCTION()
    void SendGameEvent(FString EventName, TMap<FString, FString> InEventData);

	UFUNCTION()
	void AbortMission();

	UFUNCTION()
	AMissionActor* GetMissionActor();

	UFUNCTION()
	void StartMission(UMissionAsset* InMission);

	UFUNCTION()
	UProceduralGenerationEngine* GetProcgen();

	UFUNCTION()
	bool GetOwningIdentity(FComputer& InComputer, int& OutIdentityID);

	UFUNCTION()
	void SendAvailableMissions();

	UFUNCTION()
	TArray<UPayloadAsset*> GetAllPayloads();

	UFUNCTION()
	USystemContext* GetSystemContext(int InComputerID);

	UFUNCTION()
	TArray<UComputerService*> GetServicesFor(EComputerType InComputerType);

	UFUNCTION()
	TArray<FString> GetLinkedHosts(USystemContext* InSystemContext);

	UFUNCTION()
	TArray<FString> GetPlayerKnownHosts();

	UFUNCTION()
	void SaveWorld();

	UFUNCTION(BlueprintCallable, Category = "Peacenet")
	void StartGame(TSubclassOf<UDesktopWidget> InDesktopClass, TSubclassOf<UWindow> InWindowClass);

	UFUNCTION()
	FString GetIPAddress(FComputer& InComputer);

	UFUNCTION()
	bool FindProgramByName(FName InName, UPeacegateProgramAsset*& OutProgram);

	UFUNCTION()
	bool IsPortOpen(FString InIPAddress, int InPort);

	UFUNCTION()
	bool IdentityHasSystemContext(int InIdentityID);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Network Map")
	bool ScanForServices(FString InIPAddress, TArray<FFirewallRule>& OutRules);

	UFUNCTION(BlueprintCallable)
	TArray<FManualPage> GetManualPages();

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
	static APeacenetWorldStateActor* BootOS(const APlayerController* InPlayerController, bool InDeleteExistingSaveFile = false);

	UFUNCTION(BlueprintCallable, Category = "Peacegate")
	static APeacenetWorldStateActor* LoadExistingOS(const APlayerController* InPlayerController);
};

template<typename AssetType>
inline bool APeacenetWorldStateActor::LoadAssets(FName ClassName, TArray<AssetType*>& OutArray)
{
	// Get the Asset Registry
	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");

	// A place to store computer type asset data
	TArray<FAssetData> Assets;

	if (!AssetRegistryModule.Get().GetAssetsByClass(ClassName, Assets, true))
		return false;

	for (auto& Asset : Assets)
	{
		OutArray.Add((AssetType*)Asset.GetAsset());
	}

	return true;
}

