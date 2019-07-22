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

FPeacenetIdentity& APeacenetWorldStateActor::GetNewIdentity()
{
	// We must have a save file.
	check(this->SaveGame);

	// Figure out which IDs are taken.
	TArray<int> TakenIDs;
	for(auto& Identity : this->SaveGame->Characters)
	{
		if(!TakenIDs.Contains(Identity.ID))
			TakenIDs.Add(Identity.ID);
	}

	// Find the first ID that isn't taken, starting from 0.
	int i = 0;
	while(TakenIDs.Contains(i))
		i++;

	// Create a new identity with the new entity ID.
	FPeacenetIdentity id;
	id.ID = i;

	// Add it to the save file.
	this->SaveGame->Characters.Add(id);

	// Return a reference to that identity in save memory.
	return this->SaveGame->Characters[this->SaveGame->Characters.Num() - 1];
}

bool APeacenetWorldStateActor::GetStoryCharacterID(UStoryCharacter* InStoryCharacter, int& OutEntityID)
{
	if(this->SaveGame)
		return this->SaveGame->GetStoryCharacterID(InStoryCharacter, OutEntityID);
	return false;
}

void APeacenetWorldStateActor::SetSaveValue(FString InValueName, bool InValue)
{
	if(this->SaveGame) this->SaveGame->SetValue(InValueName, InValue);
}

FEmail& APeacenetWorldStateActor::GetNewEmailMessage()
{
	// Get a list of taken email IDs.
	TArray<int> TakenIDs;
	for(auto& Message : this->GetEmailMessages())
	{
		if(!TakenIDs.Contains(Message.ID))
			TakenIDs.Add(Message.ID);
	}

	// Find the first not-taken ID.
	int id = 0;
	while(TakenIDs.Contains(id))
		id++;

	// Create a new email message with this ID.
	FEmail mail;
	mail.ID = id;

	// Add it to the save file.
	this->SaveGame->EmailMessages.Add(mail);

	// Return a reference to the email in save memory.
	return this->SaveGame->EmailMessages[this->SaveGame->EmailMessages.Num() - 1];
}

int APeacenetWorldStateActor::GetPlayerUserID()
{
	return this->SaveGame->PlayerUserID;
}

bool APeacenetWorldStateActor::DnsResolve(FString InHost, FComputer& OutComputer, EConnectionError& OutError)
{
	// Default the error to nothing.
	OutError = EConnectionError::None;

	// Maximum resolution attempts before we abort with a timeout error.
	// This is an actual error and prevents the game from infinite-looping if
	// we keep resolving in circles.
	int MaxHops = 100;

	// Try to resolve any domain names to an IP address.
	while(this->SaveGame->DomainNameMap.Contains(InHost))
	{
		// Resolve the domain.
		InHost = this->SaveGame->DomainNameMap[InHost];
		MaxHops--;

		// Break with timeout error if max hops is less than zero.
		if(MaxHops < 0)
		{
			OutError = EConnectionError::ConnectionTimedOut;
			return false;
		}
	}

	// If, at this point, we do not have an IP address in the host, we're going to timeout.
	if(!this->SaveGame->ComputerIPMap.Contains(InHost))
	{
		OutError = EConnectionError::ConnectionTimedOut;
		return false;
	}

	// Now we can get a computer ID.
	int Entity = this->SaveGame->ComputerIPMap[InHost];

	int Index = -1;
	bool result = this->SaveGame->GetComputerByID(Entity, OutComputer, Index);

	// If we didn't find a computer then we'll timeout.
	if(!result)
	{
		OutError = EConnectionError::ConnectionTimedOut;
		return false;
	}

	// Now we have a computer.  I can see the truth.  Only returning true.
	return true;
}

bool APeacenetWorldStateActor::CharacterNameExists(FString InCharacterName)
{
	return this->SaveGame && this->SaveGame->CharacterNameExists(InCharacterName);
}

FComputer& APeacenetWorldStateActor::GetNewComputer()
{
	TArray<int> TakenIDs;
	for(auto& PC : this->SaveGame->Computers)
	{
		if(!TakenIDs.Contains(PC.ID))
			TakenIDs.Add(PC.ID);
	}

	int i = 0;
	while(TakenIDs.Contains(i))
		i++;

	FComputer pc;
	pc.ID = i;

	this->SaveGame->Computers.Add(pc);
	return this->SaveGame->Computers[this->SaveGame->Computers.Num() - 1];
}

void APeacenetWorldStateActor::ClearNonPlayerEntities()
{
	this->SaveGame->ClearNonPlayerEntities();
}

TArray<FString> APeacenetWorldStateActor::GetDomainNames()
{
	TArray<FString> keys;
	if(this->SaveGame)
		this->SaveGame->DomainNameMap.GetKeys(keys);
	return keys;
}

void APeacenetWorldStateActor::AssignStoryCharacterID(UStoryCharacter* InStoryCharacter, FPeacenetIdentity& InIdentity)
{
	check(this->SaveGame);
	check(InStoryCharacter);

	this->SaveGame->AssignStoryCharacterID(InStoryCharacter, InIdentity.ID);
}

FPeacenetIdentity& APeacenetWorldStateActor::GetCharacterByID(int InEntityID)
{
	int i = 0;
	FPeacenetIdentity identity;
	bool result = this->SaveGame && this->SaveGame->GetCharacterByID(InEntityID, identity, i);
	check(result);
	return this->SaveGame->Characters[i];
}

TArray<FEmail> APeacenetWorldStateActor::GetEmailMessages()
{
	check(this->SaveGame);

	return this->SaveGame->EmailMessages;
}

bool APeacenetWorldStateActor::IdentityExists(int EntityID)
{
	FPeacenetIdentity id;
	int i;
	return this->SaveGame && this->SaveGame->GetCharacterByID(EntityID, id, i);
}

FComputer& APeacenetWorldStateActor::GetPlayerComputer()
{
	check(this->SaveGame);
	check(this->SaveGame->PlayerHasComputer());

	FComputer pc;
	int i = 0;
	bool result = this->SaveGame->GetComputerByID(this->SaveGame->PlayerComputerID, pc, i);
	check(result);

	return this->SaveGame->Computers[i];
}

bool APeacenetWorldStateActor::IsMissionCompleted(UMissionAsset* InMission)
{
	return this->SaveGame && InMission && this->SaveGame->CompletedMissions.Contains(InMission);
}

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

bool APeacenetWorldStateActor::IsIPAddress(FString InIPAddress)
{
	return this->SaveGame && this->SaveGame->ComputerIPMap.Contains(InIPAddress);
}

bool APeacenetWorldStateActor::IsTrue(FString InSaveBoolean)
{
	return this->SaveGame && this->SaveGame->IsTrue(InSaveBoolean);
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

	// Peacenet 0.2.x: IP addresses do not, under any circumstances, require Peacenet Identities.
	FString IP = this->Procgen->GenerateIPAddress();
	this->SaveGame->ComputerIPMap.Add(IP, InComputer.ID);
	return IP;
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
		CoffeeIsCode->RequiredUpgrade = Program->RequiredUpgrade;
		
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
		root.Domain = EUserDomain::Administrator;
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

void APeacenetWorldStateActor::RemoveDomain(FString InDomainName)
{
	if(this->SaveGame->DomainNameMap.Contains(InDomainName))
	{
		this->SaveGame->DomainNameMap.Remove(InDomainName);
	}
}

TArray<FPeacenetIdentity> APeacenetWorldStateActor::GetCharacters()
{
	return this->SaveGame->Characters;
}

FComputer& APeacenetWorldStateActor::GetComputerByID(int InEntityID)
{
	FComputer pc;
	int i = -1;
	bool result = this->SaveGame->GetComputerByID(InEntityID, pc, i);
	check(result);
	return this->SaveGame->Computers[i];
}

void APeacenetWorldStateActor::ReplaceDomain(FString InDomain, FString NewDomain)
{
	FString OldDest = this->SaveGame->DomainNameMap[InDomain];
	this->RemoveDomain(InDomain);
	this->RegisterDomain(NewDomain, OldDest);
}

void APeacenetWorldStateActor::RegisterDomain(FString DomainName, FString DestinationHost)
{
	if(this->SaveGame->DomainNameMap.Contains(DomainName))
	{
		this->SaveGame->DomainNameMap[DomainName] = DestinationHost;
	}
	else 
	{
		this->SaveGame->DomainNameMap.Add(DomainName, DestinationHost);
	}
}

int APeacenetWorldStateActor::GetWorldSeed()
{
	return this->SaveGame->WorldSeed;
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
	actor->SaveGame->WorldSeed = FDateTime::Now().ToUnixTimestamp();
	return actor;
}