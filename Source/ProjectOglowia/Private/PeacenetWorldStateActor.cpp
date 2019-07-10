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

#include "PeacenetWorldStateActor.h"
#include "Kismet/GameplayStatics.h"
#include "PeacegateProgramAsset.h"
#include "MissionAsset.h"
#include "ComputerService.h"
#include "ChatManager.h"
#include "WallpaperAsset.h"
#include "ProceduralGenerationEngine.h"
#include "MarkovTrainingDataAsset.h"
#include "CommandInfo.h"
#include "TerminalCommand.h"
#include "ManualPageAssetBase.h"
#include "PayloadAsset.h"
#include "Async.h"
#include "Exploit.h"
#include "PeacenetGameInstance.h"
#include "MissionActor.h"
#include "SystemUpgrade.h"
#include "Window.h"

void APeacenetWorldStateActor::BroadcastMissionComplete(UMissionAsset* InMissionAsset)
{
	this->MissionCompleteEvent.Broadcast(InMissionAsset);
}

int APeacenetWorldStateActor::GetGameStat(FName InStatName)
{
	return this->SaveGame->GetGameStat(InStatName);
}

void APeacenetWorldStateActor::SetGameStat(FName InStatName, int InValue)
{
	int previousValue = this->GetGameStat(InStatName);
	this->SaveGame->SetGameStat(InStatName, InValue);

	this->SendGameEvent("GameStatChange", {
		{ "StatName", InStatName.ToString()},
		{ "PreviousValue", FString::FromInt(previousValue)},
		{ "Current", FString::FromInt(InValue)}
	});
}

void APeacenetWorldStateActor::IncreaseGameStat(FName InStatName)
{
	this->SetGameStat(InStatName, this->GetGameStat(InStatName) + 1);
}

bool APeacenetWorldStateActor::IsNewGame()
{
	return this->SaveGame->IsNewGame;
}

bool APeacenetWorldStateActor::MakeTransaction(FString InOriginCryptoAddress, FString InTargetCryptoAddress, int InAmount)
{
	bool OriginExists = this->SaveGame->CryptoWalletExists(InOriginCryptoAddress);
	bool TargetExists = this->SaveGame->CryptoWalletExists(InOriginCryptoAddress);
	
	check(OriginExists);

	if(!(OriginExists && TargetExists)) return false;

	if(!this->SaveGame->RemoveFromWallet(InOriginCryptoAddress, InAmount)) return false;

	if(!this->SaveGame->AddToWallet(InTargetCryptoAddress, InAmount)) return false;

	this->SendGameEvent("CryptoTransaction", {
		{ "From", InOriginCryptoAddress },
		{ "To", InTargetCryptoAddress },
		{ "Amount", FString::FromInt(InAmount) }
	});

	return true;
}

void APeacenetWorldStateActor::SendGameEvent(FString EventName, TMap<FString, FString> InEventData)
{
	if(this->CurrentMission)
	{
		this->CurrentMission->SendGameEvent(EventName, InEventData);
	}

	FGameEventData Data;
	Data.EventData = InEventData;
	this->GameEventSent.Broadcast(EventName, Data);
}

bool APeacenetWorldStateActor::IdentityHasSystemContext(int InIdentityID)
{
	for(auto SystemContext : this->SystemContexts)
	{
		if(SystemContext->GetCharacter().ID == InIdentityID)
		{
			return true;
		}
	}
	return false;
}

bool APeacenetWorldStateActor::ResolveSystemContext(FString InHost, USystemContext*& OutSystem, EConnectionError& OutError)
{
	// What the fuck are we connecting to and does it even exist?
	FComputer pc;
	if(!this->ResolveHost(InHost, pc, OutError))
		return false;

	// Now that we have a Computer ID, we can search existing contexts for a matching ID.
	for(auto Context : this->SystemContexts)
	{
		if(Context->GetComputer().ID == pc.ID)
		{
			OutSystem = Context;
			return true;
		}
	}

	// Okay, this is the hard issue. Creating a new context.
	// Doing this is literally a pain in my ass.
	// Like my ass is sore.
	//
	// I need a character identity.
	int Identity = -1;
	if(pc.OwnerType != EComputerOwnerType::None)
	{
		if (!this->GetOwningIdentity(pc, Identity))
		{
			OutError = EConnectionError::ConnectionTimedOut;
			return false;
		}
	}
	// Now that we have a PC and an identity, we can create a system context.
	USystemContext* NewContext = NewObject<USystemContext>(this);

	// Assign the context to the identity and computer and this Peacenet.
	NewContext->Setup(pc.ID, Identity, this);

	// Give it to the caller.
	OutSystem = NewContext;

	// Keep it from being collected.
	this->SystemContexts.Add(NewContext);

	// Done!
	return true;
}

void APeacenetWorldStateActor::FailMission(const FText& InFailMessage)
{
	check(this->CurrentMission);

	this->MissionFailed.Broadcast(this->CurrentMission, InFailMessage);
}

UProceduralGenerationEngine* APeacenetWorldStateActor::GetProcgen()
{
	return this->Procgen;
}

// Sets default values
APeacenetWorldStateActor::APeacenetWorldStateActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

bool APeacenetWorldStateActor::IsInMission()
{
	return this->CurrentMission && this->CurrentMission->IsValidLowLevelFast();
}

AMissionActor* APeacenetWorldStateActor::GetMissionActor()
{
	return this->CurrentMission;
}

void APeacenetWorldStateActor::StartMission(UMissionAsset* InMission)
{
	check(InMission);
	check(!this->IsInMission());

	if(!InMission) return;
	if(this->IsInMission()) return;

	FVector Location(0.0f, 0.0f, 0.0f);
	 FRotator Rotation(0.0f, 0.0f, 0.0f);
 	FActorSpawnParameters SpawnInfo;

	this->CurrentMission = this->GetWorld()->SpawnActor<AMissionActor>(Location, Rotation, SpawnInfo);
	this->CurrentMission->Setup(this, InMission);
}

void APeacenetWorldStateActor::AbortMission()
{
	check(this->IsInMission());

	this->CurrentMission->Abort();
	this->CurrentMission = nullptr;
}

bool APeacenetWorldStateActor::IsPortOpen(FString InIPAddress, int InPort)
{
	check(this->SaveGame);
	check(this->Procgen);

	check(this->SaveGame->ComputerIPMap.Contains(InIPAddress));

	int EntityID = this->SaveGame->ComputerIPMap[InIPAddress];

	FComputer Computer;
	int ComputerIndex;
	bool result = this->SaveGame->GetComputerByID(EntityID, Computer, ComputerIndex);
	check(result);

	this->Procgen->GenerateFirewallRules(this->SaveGame->Computers[ComputerIndex]);

	for(FFirewallRule& Rule : this->SaveGame->Computers[ComputerIndex].FirewallRules)
	{
		if(Rule.Port == InPort)
			return !Rule.IsFiltered;
	}

	return false;
}

USystemContext* APeacenetWorldStateActor::GetSystemContext(int InComputerID)
{
	for(auto Context : this->SystemContexts)
	{
		if(Context->GetComputer().ID == InComputerID)
			return Context;
	}

	check(this->SaveGame);

	FComputer Computer;
	int ComputerIndex;
	bool result = this->SaveGame->GetComputerByID(InComputerID, Computer, ComputerIndex);
	check(result);

	USystemContext* NewContext = NewObject<USystemContext>(this);

	NewContext->Setup(Computer.ID, Computer.SystemIdentity, this);

	SystemContexts.Add(NewContext);

	return NewContext;
}


bool APeacenetWorldStateActor::GetOwningIdentity(FComputer& InComputer, int& OutIdentityID)
{
	check(this->SaveGame);

	for(auto& Identity : this->SaveGame->Characters)
	{
		if(Identity.ComputerID == InComputer.ID)
		{
			OutIdentityID = Identity.ID;
			return true;
		}
	}
	return false;
}

bool APeacenetWorldStateActor::ResolveHost(FString InHost, FComputer& OutComputer, EConnectionError& OutError)
{
	// If it's a domain name, map it to the IP address.
	if(this->SaveGame->DomainNameMap.Contains(InHost))
	{
		return this->ResolveHost(this->SaveGame->DomainNameMap[InHost], OutComputer, OutError);
	}

	// TODO: Host -> IP (a.k.a DNS).
	if(!this->SaveGame->ComputerIPMap.Contains(InHost))
	{
		OutError = EConnectionError::ConnectionTimedOut;
		return false;
	}

	int Index = 0;
	bool result = this->SaveGame->GetComputerByID(this->SaveGame->ComputerIPMap[InHost], OutComputer, Index);
	OutError = (result) ? EConnectionError::None : EConnectionError::ConnectionTimedOut;
	return result;
}

bool APeacenetWorldStateActor::ScanForServices(FString InIPAddress, TArray<FFirewallRule>& OutRules)
{
	check(this->SaveGame);
	check(this->Procgen);

	if(!this->SaveGame->IPAddressAllocated(InIPAddress))
		return false;

	int ComputerID = this->SaveGame->ComputerIPMap[InIPAddress];

	FComputer Computer;
	int ComputerIndex;
	bool result = this->SaveGame->GetComputerByID(ComputerID, Computer, ComputerIndex);

	check(result);

	FComputer& RefComputer = this->SaveGame->Computers[ComputerIndex];

	this->Procgen->GenerateFirewallRules(RefComputer);

	OutRules = RefComputer.FirewallRules;

	return OutRules.Num();
}

void APeacenetWorldStateActor::UpdateMaps()
{
	MapsUpdated.Broadcast();
}

TArray<FPeacenetIdentity> APeacenetWorldStateActor::GetAdjacentNodes(FPeacenetIdentity& InIdentity)
{
	check(this->SaveGame);
	check(this->Procgen);

	this->Procgen->GenerateAdjacentNodes(InIdentity);

	TArray<FPeacenetIdentity> Ret;

	for(auto EntityID : this->SaveGame->GetAdjacents(InIdentity.ID))
	{
		int LinkIndex = 0;
		FPeacenetIdentity Link;
		bool result = this->SaveGame->GetCharacterByID(EntityID, Link, LinkIndex);
		check(result);
		Ret.Add(Link);
	}

	return Ret;
}

TArray<UComputerService*> APeacenetWorldStateActor::GetServicesFor(EComputerType InComputerType)
{
	TArray<UComputerService*> Ret;
	for(auto Service : this->ComputerServices)
	{
		if(Service->TargetComputerType == InComputerType)
			Ret.Add(Service);
	}
	return Ret;
}

FString APeacenetWorldStateActor::GetIPAddress(FComputer& InComputer)
{
	check(this->SaveGame);
	check(this->Procgen);

	for(auto& IP : this->SaveGame->ComputerIPMap)
	{
		if(IP.Value == InComputer.ID)
			return IP.Key;
	}

	// No IP address exists for this computer. So we'll generate one.
	// First we look through all Peacenet identities to see if one
	// owns this comuter. If one is found, we generate a public IP.
	//
	// Else we generate a private one for an enterprise net (NYI).

	for(auto& Identity: this->SaveGame->Characters)
	{
		if(Identity.ComputerID == InComputer.ID)
		{
			FString IP = this->Procgen->GenerateIPAddress();
			this->SaveGame->ComputerIPMap.Add(IP, InComputer.ID);
			return IP;
		}
	}

	return "0.0.0.0";
}

// Loads all the terminal commands in the game
void APeacenetWorldStateActor::LoadTerminalCommands()
{
	TArray<UCommandInfo*> Commands;

	// Load command assets.
	this->LoadAssets(TEXT("CommandInfo"), Commands);

	for (auto Command : Commands)
	{
		this->CommandInfo.Add(Command->ID, Command);
		}

	for (auto Program : this->Programs)
	{
		UCommandInfo* CoffeeIsCode = NewObject<UCommandInfo>();

		CoffeeIsCode->ID = Program->ID;
		CoffeeIsCode->FullName = Program->FullName;
		CoffeeIsCode->Summary = Program->Summary;
		CoffeeIsCode->UnlockedByDefault = Program->IsUnlockedByDefault;
		
		this->CommandInfo.Add(Program->ID, CoffeeIsCode);
	}
}

TArray<UExploit*> APeacenetWorldStateActor::GetExploits()
{
	return this->Exploits;
}

TArray<UPayloadAsset*> APeacenetWorldStateActor::GetAllPayloads()
{
	return this->Payloads;
}

TArray<FManualPage> APeacenetWorldStateActor::GetManualPages()
{
	return this->ManualPages;
}

TArray<USystemUpgrade*> APeacenetWorldStateActor::GetAllSystemUpgrades()
{
	return this->UserUnlockableUpgrades;
}

void APeacenetWorldStateActor::SendMissionMail(UMissionAsset* InMission)
{
	check(InMission);

	FEmail MissionEmail;
	MissionEmail.ID = this->SaveGame->EmailMessages.Num();
	MissionEmail.InReplyTo = -1;
	this->SaveGame->GetStoryCharacterID(InMission->Assigner, MissionEmail.FromEntity);
	MissionEmail.ToEntities.Add(SaveGame->GetPlayerIdentity());
	MissionEmail.Mission = InMission;
	MissionEmail.Subject = InMission->Name.ToString();
	this->SaveGame->EmailMessages.Add(MissionEmail);
	this->NewMailAdded.Broadcast();

	// Notify the player of the new email message.
	this->GetSystemContext(this->SaveGame->PlayerComputerID)->GetMailProvider()->NotifyReceivedMessage(MissionEmail.ID);
}

void APeacenetWorldStateActor::SendAvailableMissions()
{
	for(auto Mission : this->Missions)
	{
		if(this->SaveGame->CompletedMissions.Contains(Mission)) continue;

		bool IsMissingMission = false;

		for(auto RequiredMission : Mission->RequiredMissions)
		{
			if(!this->SaveGame->CompletedMissions.Contains(RequiredMission))
			{
				IsMissingMission = true;
				break;
			}
		}

		if(IsMissingMission) continue;

		bool HasMissionMail = false;

		for(auto& MailMessage : this->SaveGame->EmailMessages)
		{
			if(MailMessage.ToEntities.Contains(this->SaveGame->GetPlayerIdentity()) && MailMessage.Mission == Mission)
			{
				HasMissionMail = true;
				break;
			}
		}

		if(HasMissionMail) continue;

		this->SendMissionMail(Mission);
	}
}

void APeacenetWorldStateActor::EndMission(bool DoGameUpdate)
{
	// Tells the game we're no longer in a mission.
	this->CurrentMission = nullptr;

	if(DoGameUpdate)
	{
		// Notifies of any new missions.
		this->SendAvailableMissions();

		// Saves the game.
		this->SaveWorld();
	}
}

float APeacenetWorldStateActor::GetStealthiness(FPeacenetIdentity& InIdentity)
{
	return this->AlertManager->GetStealthStatus(InIdentity.ID).Stealthiness;
}

void APeacenetWorldStateActor::SetStealthiness(FPeacenetIdentity& InIdentity, float InValue)
{
	this->AlertManager->GetStealthStatus(InIdentity.ID).Stealthiness = InValue;
	this->AlertManager->ResetStealthIncreaseTimer(InIdentity.ID);
	this->AlertManager->GetStealthStatus(InIdentity.ID).Cooldown = 30.f;

	if(InValue <= 0.7f && !this->AlertManager->GetStealthStatus(InIdentity.ID).IsSuspicious)
	{
		this->AlertManager->GetStealthStatus(InIdentity.ID).IsSuspicious = true;
	
		this->SendGameEvent("SuspicionRaised", {
			{ "Identity", FString::FromInt(InIdentity.ID)}
		});
	}

	if(InValue <= 0.45f && !this->AlertManager->GetStealthStatus(InIdentity.ID).IsInAlert)
	{
		this->AlertManager->GetStealthStatus(InIdentity.ID).IsInAlert = true;

		this->SendGameEvent("CoverBlown", {
			{ "Identity", FString::FromInt(InIdentity.ID)}
		});
	}
}

// Called when the game starts or when spawned
void APeacenetWorldStateActor::BeginPlay()
{
	Super::BeginPlay();

	// Spawn in the alert manager.
	FVector Location(0.0f, 0.0f, 0.0f);
	 FRotator Rotation(0.0f, 0.0f, 0.0f);
 	FActorSpawnParameters SpawnInfo;
	this->AlertManager = this->GetWorld()->SpawnActor<AAlertManager>(Location, Rotation, SpawnInfo);
	this->AlertManager->Setup(this);

	// Initialize tutorial state.
	this->TutorialState = NewObject<UTutorialPromptState>(this);

	// Load all the manual pages.
	TArray<UManualPageAssetBase*> ManualAssets;
	this->LoadAssets<UManualPageAssetBase>("ManualPageAssetBase", ManualAssets);
	for(auto ManualAsset : ManualAssets)
	{
		this->ManualPages.Add(ManualAsset->GetManualPage());
	}

	// Load all user-unlockable upgrades.
	this->UserUnlockableUpgrades.Empty();
	TArray<USystemUpgrade*> Upgrades;
	this->LoadAssets<USystemUpgrade>("SystemUpgrade", Upgrades);
	for(auto Upgrade : Upgrades)
	{
		if(Upgrade->CanUserUnlock)
		{
			this->UserUnlockableUpgrades.Add(Upgrade);
		}
	}

	// Load computer services in.
	this->LoadAssets<UComputerService>("ComputerService", this->ComputerServices);

	// Yeah, I got paid....and there was definitely a load....
	this->LoadAssets<UPayloadAsset>("PayloadAsset", this->Payloads);


	// Load all exploits.
	this->LoadAssets<UExploit>("Exploit", this->Exploits);

	// Load wallpaper assets.
	this->LoadAssets<UWallpaperAsset>(TEXT("WallpaperAsset"), this->Wallpapers);

	// Load markov training data for world gen stuff.
	this->LoadAssets<UMarkovTrainingDataAsset>("MarkovTrainingDataAsset", this->MarkovData);

	// Do we have an existing OS?
	if (!HasExistingOS())
	{
		// Create a new save game object.
		this->SaveGame = NewObject<UPeacenetSaveGame>(this);
	}

	// Load all the game's programs
	LoadAssets(TEXT("PeacegateProgramAsset"), this->Programs);

	// Load terminal command assets, build usage strings, populate command map.
	this->LoadTerminalCommands();

	// Spin up the procedural generation engine.
	this->Procgen = NewObject<UProceduralGenerationEngine>(this);

	// Load all mission assets.
	TArray<UMissionAsset*> TempMissions;

	this->LoadAssets<UMissionAsset>("MissionAsset", TempMissions);

	for(auto Mission: TempMissions)
	{
		check(Mission->Assigner);

		if(Mission->Assigner)
		{
			this->Missions.Add(Mission);
		}
	}
}

void APeacenetWorldStateActor::EndPlay(const EEndPlayReason::Type InReason)
{
	// If we're in a mission, we're going to abandon ship.
	if(this->IsInMission())
	{
		this->CurrentMission->Abort();
	}

	// Destroy the alert state.
	this->AlertManager->Destroy();

	// Destroy all system contexts.
	while(this->SystemContexts.Num())
	{
		this->SystemContexts[0]->Destroy();
		this->SystemContexts.RemoveAt(0);
	}

	this->SaveWorld();
}

// Called every frame
void APeacenetWorldStateActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	this->Procgen->Update(DeltaTime);

	// Is the save loaded?
	if (SaveGame)
	{
		// Get time of day
		float TimeOfDay = SaveGame->EpochTime;

		// Tick it.
		TimeOfDay += (DeltaTime * 6);

		// Has it been a full day yet?
		if (TimeOfDay >= SaveGame->SECONDS_DAY_LENGTH)
		{
			TimeOfDay = 0;
		}

		// Save it
		SaveGame->EpochTime = TimeOfDay;
	}
}

UTutorialPromptState* APeacenetWorldStateActor::GetTutorialState()
{
	return this->TutorialState;
}

bool APeacenetWorldStateActor::IsTutorialActive()
{
	return this->GetTutorialState()->IsPromptActive();
}

void APeacenetWorldStateActor::StartGame(TSubclassOf<UDesktopWidget> InDesktopClass, TSubclassOf<UWindow> InWindowClass)
{
	check(HasExistingOS() || this->SaveGame);

	if(!this->SaveGame)
		this->SaveGame = Cast<UPeacenetSaveGame>(UGameplayStatics::LoadGameFromSlot("PeacegateOS", 0));

	this->DesktopClass = InDesktopClass;
	this->WindowClass = InWindowClass;

	UPeacenetGameInstance* GameInstance = Cast<UPeacenetGameInstance>(GetGameInstance());

	// Create a new computer for the player if none is available.
	if(!this->SaveGame->PlayerHasComputer())
	{
		FComputer pc;
		pc.ID = 0;
		pc.OwnerType = EComputerOwnerType::Player;

		pc.CurrentWallpaper = nullptr;

		FUser root;
		root.Username = "root";
		root.ID = 0;
		pc.Users.Add(root);

		SaveGame->Computers.Add(pc);
		SaveGame->PlayerComputerID = pc.ID;
	}

	Procgen->Initialize(this);

	// Create a system context.
	USystemContext* PlayerSystemContext = NewObject<USystemContext>(this);

	// Link it to the character and computer.
	PlayerSystemContext->Setup(this->SaveGame->PlayerComputerID, -1, this);

	// Set up the desktop.
	PlayerSystemContext->SetupDesktop(0);

	// Keep the player system context loaded.
	this->SystemContexts.Add(PlayerSystemContext);

	this->PlayerSystemReady.Broadcast(PlayerSystemContext);

	// This allows the game to notify the player of any new missions.
	if(this->SaveGame->PlayerHasIdentity())
		this->SendAvailableMissions();
}

bool APeacenetWorldStateActor::FindProgramByName(FName InName, UPeacegateProgramAsset *& OutProgram)
{
	for (auto Program : this->Programs)
	{
		if (Program->ID == InName)
		{
			OutProgram = Program;
			return true;
		}
	}

	return false;
}

FText APeacenetWorldStateActor::GetTimeOfDay()
{
	if (!SaveGame)
		return FText();

	float TOD = SaveGame->EpochTime;

	float Seconds = FMath::Fmod(TOD, 60.f);
	float Minutes = FMath::Fmod(TOD / 60.f, 60.f);
	float Hours = FMath::Fmod((TOD / 60.f) / 60.f, 24.f);

	FString MinutesString = FString::FromInt((int)Minutes);
	if (Minutes < 10.f)
	{
		MinutesString = TEXT("0") + MinutesString;
	}

	FString HoursString = FString::FromInt((int)Hours);

	return FText::FromString(HoursString + TEXT(":") + MinutesString);
}

bool APeacenetWorldStateActor::HasExistingOS()
{
	return UGameplayStatics::DoesSaveGameExist(TEXT("PeacegateOS"), 0);
}

void APeacenetWorldStateActor::SaveWorld()
{
	// If we're in a mission, back the fuck out.
	if(this->IsInMission()) return;

	// Actually save the game.
	UGameplayStatics::SaveGameToSlot(this->SaveGame, TEXT("PeacegateOS"), 0);
}

APeacenetWorldStateActor* APeacenetWorldStateActor::LoadExistingOS(const APlayerController* InPlayerController)
{
	check(HasExistingOS());

	UWorld* World = InPlayerController->GetWorld();

	auto ExistingPeacenet = World->SpawnActor<APeacenetWorldStateActor>();

	return ExistingPeacenet;
}

APeacenetWorldStateActor* APeacenetWorldStateActor::BootOS(const APlayerController* InPlayerController, bool InDeleteExistingSaveFile)
{
	// If we have an existing OS then we'll load that in if, and only if, we aren't deleting the existing OS.
	if(APeacenetWorldStateActor::HasExistingOS())
	{
		if(InDeleteExistingSaveFile)
		{
			UGameplayStatics::DeleteGameInSlot(TEXT("PeacegateOS"), 0);
		}
		else {
			return APeacenetWorldStateActor::LoadExistingOS(InPlayerController);
		}
	}

	// Spawn a new Peacenet actor.
	UWorld* world = InPlayerController->GetWorld();
	APeacenetWorldStateActor* actor = world->SpawnActor<APeacenetWorldStateActor>();
	actor->SaveGame = NewObject<UPeacenetSaveGame>();
	return actor;
}