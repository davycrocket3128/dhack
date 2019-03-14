// Copyright (c) 2018 The Peacenet & Alkaline Thunder.

#include "PeacenetWorldStateActor.h"
#include "Kismet/GameplayStatics.h"
#include "UPeacegateProgramAsset.h"
#include "UComputerService.h"
#include "UHelpCommand.h"
#include "UChatManager.h"
#include "WallpaperAsset.h"
#include "UProceduralGenerationEngine.h"
#include "UMarkovTrainingDataAsset.h"
#include "CommandInfo.h"
#include "TerminalCommand.h"
#include "ManualPageAssetBase.h"
#include "PayloadAsset.h"
#include "Async.h"
#include "Exploit.h"
#include "UPeacenetGameInstance.h"
#include "UWindow.h"

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
	if (!this->GetOwningIdentity(pc, Identity))
	{
		OutError = EConnectionError::ConnectionTimedOut;
		return false;
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

USystemContext* APeacenetWorldStateActor::GetSystemContext(int InIdentityID)
{
	for(auto Context : this->SystemContexts)
	{
		if(Context->GetCharacter().ID == InIdentityID)
			return Context;
	}

	check(this->SaveGame);

	FPeacenetIdentity Identity;
	int IdentityIndex;
	bool result = this->SaveGame->GetCharacterByID(InIdentityID, Identity, IdentityIndex);
	check(result);

	FComputer Computer;
	int ComputerIndex;
	result = this->SaveGame->GetComputerByID(Identity.ComputerID, Computer, ComputerIndex);
	check(result);

	USystemContext* NewContext = NewObject<USystemContext>(this);

	NewContext->Setup(Computer.ID, Identity.ID, this);

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
			FString IP = this->Procgen->GenerateIPAddress(Identity.Country);
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
		this->CommandInfo.Add(Command->Info.CommandName, Command);
		}

	for (auto Program : this->Programs)
	{
		// create manual pages for the programs as well.
		FCommandInfoS Info;
		Info.CommandName = Program->ExecutableName;
		Info.Description = Program->AppLauncherItem.Description.ToString();

		UCommandInfo* CoffeeIsCode = NewObject<UCommandInfo>();

		CoffeeIsCode->Info = Info;
		CoffeeIsCode->UnlockedByDefault = Program->IsUnlockedByDefault;
		
		this->CommandInfo.Add(Program->ExecutableName, CoffeeIsCode);
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

// Called when the game starts or when spawned
void APeacenetWorldStateActor::BeginPlay()
{
	Super::BeginPlay();

	// Load all the manual pages.
	TArray<UManualPageAssetBase*> ManualAssets;
	this->LoadAssets<UManualPageAssetBase>("ManualPageAssetBase", ManualAssets);
	for(auto ManualAsset : ManualAssets)
	{
		this->ManualPages.Add(ManualAsset->GetManualPage());
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
}

void APeacenetWorldStateActor::EndPlay(const EEndPlayReason::Type InReason)
{
	this->SaveWorld();
}

// Called every frame
void APeacenetWorldStateActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

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

FGovernmentAlertInfo APeacenetWorldStateActor::GetAlertInfo(int InCharacterId)
{
	check(this->SaveGame);

	FPeacenetIdentity Character;
	int CharacterIndex;
	bool result = this->SaveGame->GetCharacterByID(InCharacterId, Character, CharacterIndex);
	check(result);

	if(!GovernmentAlertInfo.Contains(Character.ID))
	{
		FGovernmentAlertInfo Info;
		Info.Status = EGovernmentAlertStatus::NoAlert;
		Info.AlertLevel = 0.f;
		this->GovernmentAlertInfo.Add(Character.ID, Info);
	}

	return GovernmentAlertInfo[Character.ID];
}

void APeacenetWorldStateActor::StartGame(TSubclassOf<UDesktopWidget> InDesktopClass, TSubclassOf<UWindow> InWindowClass)
{
	check(HasExistingOS() || this->SaveGame);

	if(!this->SaveGame)
		this->SaveGame = Cast<UPeacenetSaveGame>(UGameplayStatics::LoadGameFromSlot("PeacegateOS", 0));

	this->DesktopClass = InDesktopClass;
	this->WindowClass = InWindowClass;

	UPeacenetGameInstance* GameInstance = Cast<UPeacenetGameInstance>(GetGameInstance());

	for(auto GameTypeAsset : GameInstance->GameTypes)
	{
		if(GameTypeAsset->Name == this->SaveGame->GameTypeName)
		{
			this->GameType = GameTypeAsset;
			break;
		}
	}

	check(GameType);

	this->Procgen->Initialize(this);

	// Find the player character.
	FPeacenetIdentity Character;
	int CharacterIndex = -1;
	bool result = this->SaveGame->GetCharacterByID(this->SaveGame->PlayerCharacterID, Character, CharacterIndex);
	check(result);

	// Create a system context.
	USystemContext* PlayerSystemContext = NewObject<USystemContext>(this);

	// Link it to the character and computer.
	PlayerSystemContext->Setup(Character.ComputerID, Character.ID, this);

	// Set up the desktop.
	PlayerSystemContext->SetupDesktop(this->SaveGame->PlayerUserID);

	this->PlayerSystemReady.Broadcast(PlayerSystemContext);
}

bool APeacenetWorldStateActor::FindProgramByName(FName InName, UPeacegateProgramAsset *& OutProgram)
{
	for (auto Program : this->Programs)
	{
		if (Program->ExecutableName == InName)
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
	// update game type, window decorator and desktop class
	SaveGame->GameTypeName = this->GameType->Name;

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