// Copyright (c) 2018 The Peacenet & Alkaline Thunder.

#include "PeacenetWorldStateActor.h"
#include "UWorldGenerator.h"
#include "Kismet/GameplayStatics.h"
#include "UComputerTypeAsset.h"
#include "UPeacegateProgramAsset.h"
#include "UVulnerability.h"
#include "WallpaperAsset.h"
#include "UMissionAsset.h"
#include "UMissionUnlock.h"
#include "UComputerService.h"
#include "UNativeLatentAction.h"
#include "UVulnerabilityTerminalCommand.h"
#include "CommandInfo.h"
#include "TerminalCommand.h"
#include "AssetRegistry/Public/IAssetRegistry.h"
#include "AssetRegistry/Public/AssetRegistryModule.h"
#include "Async.h"
#include "UWindow.h"

bool APeacenetWorldStateActor::UpdateComputer(int InEntityID, FComputer & InComputer, bool InSaveGame)
{
	if (!SaveGame)
	{
		return false;
	}

	for (int i = 0; i < SaveGame->Computers.Num(); i++)
	{
		FComputer OldPC = SaveGame->Computers[i];
		if (OldPC.ID == InEntityID)
		{
			SaveGame->Computers[i] = InComputer;
			if (InComputer.OwnerType == EComputerOwnerType::Player)
			{
				if (InSaveGame)
				{
					this->SaveWorld();
				}
			}
			return true;
		}
	}

	return false;
}

void APeacenetWorldStateActor::AddLatentAction(UNativeLatentAction* InAction)
{
	this->LatentActions.Add(InAction);
}

bool APeacenetWorldStateActor::UpdateCharacter(int InEntityID, FPeacenetIdentity & InCharacter)
{
	for (int i = 0; i < SaveGame->Characters.Num(); i++)
	{
		if (SaveGame->Characters[i].ID == InEntityID)
		{
			SaveGame->Characters[i] = InCharacter;
			return true;
		}
	}

	return false;
}

// Sets default values
APeacenetWorldStateActor::APeacenetWorldStateActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

bool APeacenetWorldStateActor::StartMission(UMissionAsset * InMission, USystemContext * InMissionSystem)
{
	check(InMission);
	check(InMissionSystem);
	check(!InMission->InternalID.ToString().IsEmpty());
	check(InMissionSystem->Computer.OwnerType == EComputerOwnerType::Player);

	check(!this->CurrentMissionAsset);
	check(!this->MissionContext);

	check(this->SaveGame);
	check(this->GameType);
	check(this->GameType->EnableMissions);

	if (!this->SaveGame->Missions.Contains(InMission->InternalID))
	{
		for (auto Prerequisite : InMission->Prerequisites)
		{
			if (!this->SaveGame->Missions.Contains(Prerequisite->InternalID))
			{
				return false;
			}
		}

		// if we make it this far, we can start the mission.
		this->CurrentMissionAsset = InMission;
		this->MissionContext = InMissionSystem;

		// We need a save file to go back to if the player abandons/restarts the mission.
		this->MissionStartSaveGame = DuplicateObject<UPeacenetSaveGame>(this->SaveGame, this);

		// Clear the mission unlock list.
		this->MissionUnlocks.Empty();

		// Now, we need to get a DUPLICATE ARRAY of mission actions.
		// We need to duplicate every action so that the player doesn't fuck up the mission's data asset
		// by completing objectives. The way UE4's data asset system works, that is completely possible.
		TArray<UMissionAction*> NewActionList;
		for (auto Action : InMission->Actions)
		{
			UMissionAction* DuplicateAction = DuplicateObject<UMissionAction>(Action.Action, this);
			DuplicateAction->SaveGame = this->SaveGame;
			DuplicateAction->SystemContext = this->MissionContext;
			NewActionList.Add(DuplicateAction);
		}
		this->CurrentMissionActions = NewActionList;

		if (InMissionSystem->Desktop)
		{
			InMissionSystem->Desktop->EnqueueNotification(FText::FromString("Mission start"), InMission->MissionName, nullptr);
		}

		return true;
	}

	return false;
}

// Loads all the terminal commands in the game
void APeacenetWorldStateActor::LoadTerminalCommands()
{
	TArray<UCommandInfo*> Commands;

	// Load command assets.
	this->LoadAssets(TEXT("CommandInfo"), Commands);

	// Help command is not processed in blueprint land.
	UCommandInfo* HelpInfo = NewObject<UCommandInfo>(this);
	HelpInfo->Info.CommandName = TEXT("help");
	HelpInfo->Info.Description = TEXT("Shows a list of commands you can run.");
	HelpInfo->Info.CommandClass = TSubclassOf<UTerminalCommand>(UHelpCommand::StaticClass());
	HelpInfo->UnlockedByDefault = true;
	Commands.Add(HelpInfo);

	for (auto Command : Commands)
	{
		this->CommandInfo.Add(Command->Info.CommandName, Command);
	
		// Compile the command info into Docopt usage strings
		FString UsageFriendly = UCommandInfo::BuildDisplayUsageString(Command->Info);
		FString UsageInternal = UCommandInfo::BuildInternalUsageString(Command->Info);

		// Create a manual page for the command
		FManPage ManPage;
		ManPage.Description = Command->Info.Description;
		ManPage.LongDescription = Command->Info.LongDescription;
		ManPage.InternalUsage = UsageInternal;
		ManPage.FriendlyUsage = UsageFriendly;

		// Save the manual page for later use
		ManPages.Add(Command->Info.CommandName, ManPage);
	}

	for (auto Program : this->Programs)
	{
		// create manual pages for the programs as well.
		FCommandInfoS Info;
		Info.CommandName = Program->ExecutableName;
		Info.Description = Program->AppLauncherItem.Description.ToString();

		FString FriendlyUsage = UCommandInfo::BuildDisplayUsageString(Info);
		FString InternalUsage = UCommandInfo::BuildInternalUsageString(Info);

		FManPage ManPage;
		ManPage.Description = Info.Description;
		ManPage.InternalUsage = InternalUsage;
		ManPage.FriendlyUsage = FriendlyUsage;

		ManPages.Add(Info.CommandName, ManPage);

		UCommandInfo* CoffeeIsCode = NewObject<UCommandInfo>();

		CoffeeIsCode->Info = Info;
		CoffeeIsCode->UnlockedByDefault = Program->IsUnlockedByDefault;
		
		this->CommandInfo.Add(Program->ExecutableName, CoffeeIsCode);
	}

	// Now we load in vulnerabilities.
	for (auto Vuln : this->Vulnerabilities)
	{
		FCommandInfoS Info;
		Info.CommandName = Vuln->InternalID;
		Info.CommandClass = UVulnerabilityTerminalCommand::StaticClass();
		Info.Description = Vuln->Name.ToString();

		Info.UsageStrings.Add("<host> [port]");

		FString FriendlyUsage = UCommandInfo::BuildDisplayUsageString(Info);
		FString InternalUsage = UCommandInfo::BuildInternalUsageString(Info);

		FManPage ManPage;
		ManPage.Description = Info.Description;
		ManPage.LongDescription = Vuln->Description.ToString();
		ManPage.InternalUsage = InternalUsage;
		ManPage.FriendlyUsage = FriendlyUsage;

		ManPages.Add(Info.CommandName, ManPage);

		UCommandInfo* VulnInfo = NewObject<UCommandInfo>(this, UVulnerabilityCommandInfo::StaticClass());
		VulnInfo->Info = Info;
		VulnInfo->UnlockedByDefault = Vuln->UnlockedByDefault;
		Cast<UVulnerabilityCommandInfo>(VulnInfo)->Vulnerability = Vuln;

		this->CommandInfo.Add(Info.CommandName, VulnInfo);
	}
}

bool APeacenetWorldStateActor::FindServiceByName(FName ServiceName, UComputerService*& OutService)
{
	for (auto Service : this->ComputerServices)
	{
		if (Service->InternalID == ServiceName)
		{
			OutService = Service;
			return true;
		}
	}

	return false;
}

// Called when the game starts or when spawned
void APeacenetWorldStateActor::BeginPlay()
{
	Super::BeginPlay();

	// Load computer services.
	this->LoadAssets<UComputerService>("ComputerService", this->ComputerServices);

	// load all the vulnerabilities.
	this->LoadAssets<UVulnerability>("Vulnerability", this->Vulnerabilities);

	// Load wallpaper assets.
	this->LoadAssets<UWallpaperAsset>(TEXT("WallpaperAsset"), this->Wallpapers);

	// Do we have an existing OS?
	if (!HasExistingOS())
	{
		// Create a new save game object.
		this->SaveGame = NewObject<UPeacenetSaveGame>(this);
	}

	// Load all the game's programs
	LoadAssets(TEXT("PeacegateProgramAsset"), this->Programs);

	LoadAssets("ComputerTypeAsset", ComputerTypes);

	for (auto ComputerType : ComputerTypes)
	{
		if (ComputerType->IsPlayerType)
		{
			this->PlayerComputerType = ComputerType;
		}
	}

	check(PlayerComputerType);

	// Load terminal command assets, build usage strings, populate command map.
	this->LoadTerminalCommands();
}

void APeacenetWorldStateActor::EndPlay(const EEndPlayReason::Type InReason)
{
	this->SaveWorld();
}

bool APeacenetWorldStateActor::ResolveHost(FString InHost, FString& ResolvedIP, USystemContext*& ResolvedContext)
{
	// TODO: Business network resolving. Can't output a system context with that.
	// TODO: This just checks the IP address, we should really implement some in-game version of DNS.
	for(auto& Computer : this->SaveGame->Computers)
	{
		if(Computer.IPAddress == InHost)
		{
			ResolvedIP = Computer.IPAddress;
			ResolvedContext = NewObject<USystemContext>();
			ResolvedContext->Computer = Computer;
			for(auto& Character : this->SaveGame->Characters)
			{
				if(Character.ComputerID == Computer.ID)
				{
					ResolvedContext->Character = Character;
					break;
				}
			}
			ResolvedContext->Peacenet = this;
			TArray<FName> Commands;
			this->CommandInfo.GetKeys(Commands);
			for(auto Command : Commands)
			{
				if (this->CommandInfo[Command]->UnlockedByDefault)
				{
					if (!ResolvedContext->Computer.InstalledCommands.Contains(Command))
					{
						ResolvedContext->Computer.InstalledCommands.Add(Command);
					}
				}
			}
			for (auto Program : this->Programs)
			{
				if (Program->IsUnlockedByDefault && !ResolvedContext->Computer.InstalledPrograms.Contains(Program->ExecutableName))
				{
					ResolvedContext->Computer.InstalledPrograms.Add(Program->ExecutableName);
				}
			}
			this->SystemContexts.Add(ResolvedContext);
			return true;
		}
	}
	return false;
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

		// Are we in a mission?
		if (CurrentMissionAsset && MissionContext && !bIsMissionPaused)
		{
			// Do we have a latent action in progress?
			if (CurrentLatentMissionAction)
			{
				// Tick it.
				this->CurrentLatentMissionAction->Tick(DeltaTime);

				// Check if it's completed.
				if (this->CurrentLatentMissionAction->IsCompleted())
				{
					// Do something.
				}
				else if (this->CurrentLatentMissionAction->IsFailed())
				{
					// Announce that the mission's been failed.
					MissionContext->Desktop->OnMissionFailed(CurrentMissionAsset, MissionContext, CurrentLatentMissionAction->GetFailReasonText());
				
					// Stop ticking the mission.
					bIsMissionPaused = true;
				}
			}
			else
			{
				// Check if we have any actions to perform.
				if (this->CurrentMissionActions.Num())
				{
					// Pull down the first action.
					UMissionAction* NextAction = this->CurrentMissionActions[0];
					
					// Is it latent?
					if (NextAction->IsA<ULatentMissionAction>())
					{
						// We've hit a checkpoint, so back up the save file.
						this->CheckpointSaveGame = DuplicateObject<UPeacenetSaveGame>(this->SaveGame, this);

						// Do the same for mission unlocks.
						this->CheckpointMissionUnlocks = this->MissionUnlocks;

						// Now we need to create a checkpoint array of all mission actions at/after this point.
						// So we need to empty it.
						this->CheckpointMissionActions.Empty();

						// And this does the duplication.
						for (auto Action : this->CurrentMissionActions)
						{
							this->CheckpointMissionActions.Add(DuplicateObject<UMissionAction>(Action, this));
						}

						// Assign it as our current latent action.
						this->CurrentLatentMissionAction = Cast<ULatentMissionAction>(NextAction);
					}
				
					// We essentially de-queue it.
					this->CurrentMissionActions.RemoveAt(0);

					// Then we execute it.
					NextAction->ExecuteMissionAction();
				}
				else
				{
					this->CompleteMission();
				}
			}
		}

		// Save it
		SaveGame->EpochTime = TimeOfDay;
	}

	for (int i = 0; i < LatentActions.Num(); i++)
	{
		auto Action = LatentActions[0];
		if (Action->Tick(DeltaTime))
		{
			LatentActions.RemoveAt(i);
			i--;
		}
	}
}

void APeacenetWorldStateActor::StartGame()
{
	AsyncTask(ENamedThreads::AnyBackgroundThreadNormalTask, [this]() {
		if (HasExistingOS())
		{
			this->SaveGame = Cast<UPeacenetSaveGame>(UGameplayStatics::LoadGameFromSlot(TEXT("PeacegateOS"), 0));

			// desktop class is stored in the save, too.
			this->DesktopClass = this->SaveGame->DesktopClass;

			// And we need a window decorator.
			this->WindowClass = this->SaveGame->WindowClass;
		}

		// Let's go through all the computers in the save file.
		for (auto& Computer : SaveGame->Computers)
		{
			// Are we owned by a player?
			if (Computer.OwnerType == EComputerOwnerType::Player)
			{
				// Go through all the programs we have loaded.
				for (auto Program : Programs)
				{
					// Is it unlocked by default and not installed?
					if (Program->IsUnlockedByDefault && !Computer.InstalledPrograms.Contains(Program->ExecutableName))
					{
						// Install it.
						Computer.InstalledPrograms.Add(Program->ExecutableName);
					}
				}

				// And now we do the same thing for terminal commands.
				TArray<FName> CommandNames;

				this->CommandInfo.GetKeys(CommandNames); //retrieve names of all commands

				for (auto CommandName : CommandNames)
				{
					// Get the command info.
					UCommandInfo* Command = this->CommandInfo[CommandName];

					// Are we unlocked by default and not installed?
					if (Command->UnlockedByDefault && !Computer.InstalledCommands.Contains(CommandName))
					{
						// Install it.
						Computer.InstalledCommands.Add(CommandName);
					}
				}

			}
		}

		AsyncTask(ENamedThreads::GameThread, [this]() 
		{
			// we need to look up the game type assets in the game.
			TArray<UPeacenetGameTypeAsset*> GameTypeAssets;

			// load them all in. Crash if we can't.
			check(this->LoadAssets<UPeacenetGameTypeAsset>(TEXT("PeacenetGameTypeAsset"), GameTypeAssets));

			// look through the assets and check if one matches the save file
			for (auto Asset : GameTypeAssets)
			{
				if (Asset->Name == SaveGame->GameTypeName)
				{
					this->GameType = Asset;
					break;
				}
			}

			// crash if we didn't find any
			check(this->GameType);

			// If we have missions enabled in this game mode then we load the missions in.
			if (this->GameType->EnableMissions)
			{
				this->LoadAssets<UMissionAsset>("MissionAsset", this->Missions);
			}

			FComputer PlayerPC = SaveGame->Computers[SaveGame->Characters[SaveGame->PlayerCharacterID].ComputerID];

			USystemContext* PlayerContext = NewObject<USystemContext>();
			
			if (!SaveGame->IsEntityKnown(SaveGame->PlayerCharacterID, EPinnedContactType::Person))
			{
				FPinnedContact PlayerContact;
				PlayerContact.ContactType = EPinnedContactType::Person;
				PlayerContact.EntityID = SaveGame->PlayerCharacterID;
				PlayerContact.IsStoryIntegral = true;
				SaveGame->PinnedContacts.Add(PlayerContact);
			}

			PlayerContext->Computer = PlayerPC;
			PlayerContext->Peacenet = this;
			PlayerContext->Character = SaveGame->Characters[SaveGame->PlayerCharacterID];

			AsyncTask(ENamedThreads::AnyBackgroundThreadNormalTask, [this, PlayerContext]()
			{
				UWorldGenerator::GenerateSystemDirectories(PlayerContext);
								
				AsyncTask(ENamedThreads::GameThread, [this, PlayerContext]()
				{
					// Update the system's files.
					PlayerContext->UpdateSystemFiles();

					APlayerController* PlayerController = UGameplayStatics::GetPlayerController(GetWorld(), 0);

					PlayerContext->Desktop = CreateWidget<UDesktopWidget, APlayerController>(PlayerController, this->DesktopClass);

					PlayerContext->Desktop->SystemContext = PlayerContext;
					PlayerContext->Desktop->UserID = SaveGame->PlayerUserID;

					SystemContexts.Add(PlayerContext);

					PlayerSystemReady.Broadcast(PlayerContext);

					// If we have missions enabled, and we have a tutorial, and it's not complete, we should start that mission.
					if (this->GameType->EnableMissions)
					{
						if (this->GameType->TutorialMission && !this->SaveGame->Missions.Contains(this->GameType->TutorialMission->InternalID))
						{
							this->StartMission(this->GameType->TutorialMission, PlayerContext);
						}
					}
				});
			});
		});
	});
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

template<typename AssetType>
bool APeacenetWorldStateActor::LoadAssets(FName ClassName, TArray<AssetType*>& OutArray)
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

bool APeacenetWorldStateActor::IsMissionActive()
{
	return this->CurrentMissionAsset;
}

bool APeacenetWorldStateActor::HasExistingOS()
{
	return UGameplayStatics::DoesSaveGameExist(TEXT("PeacegateOS"), 0);
}

APeacenetWorldStateActor* APeacenetWorldStateActor::GenerateAndCreateWorld(const APlayerController* InPlayerController, const FPeacenetWorldInfo& InWorldInfo, TSubclassOf<UDesktopWidget> InDesktop, UPeacenetGameTypeAsset* InGameType, TSubclassOf<UWindow> InWindowDecorator, FShowLoadingScreenEvent InShowLoadingScreen)
{
	// This function is responsible for initially generating a Peacenet world.
	// We take in an FPeacenetWorldInfo structure by-ref so we know how to spawn
	// the player's computer and Peacenet identity.
	//
	// We take in a Player Controller so that we have a world context object.
	// Eventually, we won't take in a raw UE4 Player Controller - we'll take in
	// our own Player Controller, allowing us to directly call back into
	// the game's Blueprint land, automatically telling the controller
	// to possess a new Peacegate OS pawn when the world generator finishes.

	// But, that can wait until I've had my fucking coffee... on Boxing Day of 2025.
	// - Alkaline

	// We're going to first delete the save file if it exists.
	if (HasExistingOS())
	{
		UGameplayStatics::DeleteGameInSlot(TEXT("PeacegateOS"), 0);
	}

	// For now, get the current world:
	UWorld* CurrentWorld = InPlayerController->GetWorld();

	// For now, that's all we need that player controller for. We can now spawn the Peacenet world.
	auto NewPeacenet = CurrentWorld->SpawnActor<APeacenetWorldStateActor>();

	// Import the desktop and game type
	NewPeacenet->DesktopClass = InDesktop;
	NewPeacenet->GameType = InGameType;
	NewPeacenet->WindowClass = InWindowDecorator;

	// Now we can create a world seed. The world seed will be generated from the combined string of the player's full name, username and hostname. This creates a unique world for each player.
	FString CombinedPlayerName = InWorldInfo.PlayerName.ToString() + TEXT("_") + InWorldInfo.PlayerUsername.ToString() + TEXT("_") + InWorldInfo.PlayerHostname.ToString();

	// We can use a CRC memory hash function to create the seed.
	TArray<TCHAR> SeedChars = CombinedPlayerName.GetCharArray();
	int Seed = FCrc::MemCrc32(SeedChars.GetData(), sizeof(TCHAR) * SeedChars.Num());

	// Suitable seed for a random number generator. So, let Peacenet create one with it.
	FRandomStream WorldGenerator = UWorldGenerator::GetRandomNumberGenerator(Seed);

	// Create a new computer for the player.
	FComputer PlayerComputer;

	// Set its entity metadata.
	PlayerComputer.ID = 0;
	PlayerComputer.ComputerType = TEXT("c_desktop"); // TODO: shouldn't be hardcoded.
	PlayerComputer.OwnerType = EComputerOwnerType::Player;

	// Create a unix root user.
	FUser RootUser;
	RootUser.ID = 0;
	RootUser.Username = FText::FromString(TEXT("root"));
	RootUser.Password = FText::FromString(TEXT(""));
	RootUser.Domain = EUserDomain::Administrator;

	// Create a non-root user.
	FUser PlayerUser;
	PlayerUser.ID = 1;
	PlayerUser.Username = InWorldInfo.PlayerUsername;
	PlayerUser.Password = InWorldInfo.PlayerPassword;
	PlayerUser.Domain = EUserDomain::PowerUser;

	// Assign users to the new computer.
	PlayerComputer.Users.Add(RootUser);
	PlayerComputer.Users.Add(PlayerUser);

	// Now we create the Peacenet Identity for the player.
	FPeacenetIdentity PlayerIdentity;
	PlayerIdentity.ID = 0;
	PlayerIdentity.Country = InGameType->SpawnCountry;
	PlayerIdentity.NodePosition = FVector2D::ZeroVector;
	PlayerIdentity.CharacterType = EIdentityType::Player;
	PlayerIdentity.CharacterName = InWorldInfo.PlayerName;
	PlayerIdentity.Skill = 0;
	PlayerIdentity.Reputation = 0.f;
	PlayerIdentity.ComputerID = PlayerComputer.ID;

	// World generator can generate the computer's filesystem.
	UWorldGenerator::CreateFilesystem(PlayerIdentity, PlayerComputer, WorldGenerator, InWorldInfo.PlayerHostname.ToString());


	// Note: The save file would have been loaded as soon as the actor spawned - BeginPlay gets called during UWorld::SpawnActor.
	// So, at this point, we've had a save file created and ready for us for a few CPU cycles now...
	UPeacenetSaveGame* WorldSave = NewPeacenet->SaveGame;

	// Tell the game what user and computer to possess.
	WorldSave->PlayerCharacterID = PlayerIdentity.ID;
	WorldSave->PlayerUserID = PlayerUser.ID;

	// We can then add our player's computer to the save file.
	WorldSave->Computers.Add(PlayerComputer);

	// Add the character
	WorldSave->Characters.Add(PlayerIdentity);

	// And we can generate non-story NPCs.
	auto Status = UWorldGenerator::GenerateCharacters(NewPeacenet, WorldGenerator, WorldSave);
	InShowLoadingScreen.Execute(Status);
	NewPeacenet->WorldGeneratorStatus = Status;

	// Saves the game when the world finishes generating.
	TScriptDelegate<> SaveDelegate;
	SaveDelegate.BindUFunction(NewPeacenet, TEXT("SaveWorld"));

	Status->WorldGenerationCompleted.Add(SaveDelegate);

	// Give our new Peacenet back to the Blueprint land or whoever else happened to call us.
	return NewPeacenet;
}

void APeacenetWorldStateActor::CompleteMission()
{
	check(CurrentMissionAsset);
	check(CurrentMissionActions.Num() == 0);

	check(MissionContext);
	
	if (MissionContext->Desktop)
	{
		MissionContext->Desktop->OnMissionCompleted(CurrentMissionAsset, MissionContext, MissionUnlocks);
	}

	SaveGame->Missions.Add(CurrentMissionAsset->InternalID);

	CurrentMissionAsset = nullptr;
	MissionContext = nullptr;

	CurrentLatentMissionAction = nullptr;

	CheckpointSaveGame = nullptr;
	MissionStartSaveGame = nullptr;
	CheckpointMissionActions.Empty();
	CheckpointMissionUnlocks.Empty();
	MissionUnlocks.Empty();

	

	this->SaveWorld();
}

void APeacenetWorldStateActor::ResynchronizeSystemContexts()
{
	for (auto Context : SystemContexts)
	{
		int ComputerID = Context->Computer.ID;
		int CharacterID = Context->Character.ID;

		for (auto Computer : SaveGame->Computers)
		{
			if (Computer.ID == ComputerID)
			{
				Context->Computer = Computer;
			}
		}

		for (auto Character : SaveGame->Characters)
		{
			if (Character.ID == CharacterID)
			{
				Context->Character = Character;
			}
		}
	}
}

void APeacenetWorldStateActor::SaveWorld()
{
	// Never save the game during a mission.
	if (this->CurrentMissionAsset)
		return;

	// update game type, window decorator and desktop class
	SaveGame->DesktopClass = this->DesktopClass;
	SaveGame->GameTypeName = this->GameType->Name;
	SaveGame->WindowClass = this->WindowClass;

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