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


#include "PeacenetGameInstance.h"
#include "GameTypeAsset.h"
#include "PeacenetWorldStateActor.h"
#include "PeacenetSaveGame.h"
#include "Computer.h"
#include "CommonUtils.h"
#include "FileUtilities.h"
#include "Folder.h"
#include "Base64.h"
#include "PeacenetIdentity.h"
#include "AssetRegistry/Public/IAssetRegistry.h"
#include "AssetRegistry/Public/AssetRegistryModule.h"
#include "DaemonInfoAsset.h"
#include "TutorialDaemon.h"
#include "Kismet/GameplayStatics.h"

void UPeacenetGameInstance::RegisterDaemons() {
	// Register the tutorial daemon for player computers so Peacegate can show hints and tutorials.
	this->RegisterPeacegateDaemon(
		UTutorialDaemon::StaticClass(),
		"tutorials",
		NSLOCTEXT("DaemonNames", "TutorialDaemon", "Hints & Tutorials"),
		NSLOCTEXT("DaemonDescriptions", "TutorialDaemon", "Allows processes to show hints and tutorials on your desktop."),
		EDaemonType::Player
	);
}

void UPeacenetGameInstance::RegisterPeacegateDaemon(TSubclassOf<UPeacegateDaemon> InDaemonClass, FName Name, FText FriendlyName, FText Description, EDaemonType DaemonType) {
	// Check that a daemon with this name isn't already registered.
	check(!this->RegisteredDaemons.Contains(Name));

	// Create a new daemon info structure to hold the registered daemon.
	FDaemonInfo Info;

	// Store the information in said structure.
	Info.DaemonClass = InDaemonClass;
	Info.Name = Name;
	Info.FriendlyName = FriendlyName;
	Info.Description = Description;
	Info.DaemonType = DaemonType;

	// Register the daemon!
	this->RegisteredDaemons.Add(Name, Info);
}

void UPeacenetGameInstance::CreateWorld(FString InCharacterName, UPeacenetGameTypeAsset* InGameType) {
	check(InGameType);

	if(APeacenetWorldStateActor::HasExistingOS()) {
		UGameplayStatics::DeleteGameInSlot("PeacegateOS", 0);
	}

	// Create a new save game.
	UPeacenetSaveGame* SaveGame = NewObject<UPeacenetSaveGame>();

	// The save file needs to keep a record of the game type.
	SaveGame->GameTypeName = InGameType->Name;

	// Setting this value causes the game to generate NPCs and other procedural
	// stuff when we get to spinning up the Peacenet world state actor later on.
	SaveGame->IsNewGame = true;

	// Now we parse the player name so we have a username and hostname.
	FString Username;
	FString Hostname;
	UCommonUtils::ParseCharacterName(InCharacterName, Username, Hostname);

	// We want to create a new computer for the player.
	FComputer PlayerComputer;
	
	// Assign the computer's metadata values.
	PlayerComputer.ID = 0; // This is what the game identifies the computer as internally. IP addresses, domains, etc. map to these IDs.
	PlayerComputer.OwnerType = EComputerOwnerType::Player; // This determines how the procedural generation system interacts with this entity later on.

	// Format the computer's filesystem.
	UFileUtilities::FormatFilesystem(PlayerComputer.Filesystem);

	// Create a new folder called "etc".
	FFolder EtcFolder;
	EtcFolder.FolderID = 1;
	EtcFolder.FolderName = "etc";
	EtcFolder.ParentID = 0;
	EtcFolder.IsReadOnly = false;

	// Add a file called "hostname" inside this folder.
	FFileRecord HostnameFile;
	HostnameFile.ID = 0;
	HostnameFile.Name = "hostname";
	HostnameFile.RecordType = EFileRecordType::Text;
	HostnameFile.ContentID = 0;

	FTextFile HostnameTextFile;
	HostnameTextFile.ID = 0;
	HostnameTextFile.Content = Hostname;
	PlayerComputer.TextFiles.Add(HostnameTextFile);
	PlayerComputer.FileRecords.Add(HostnameFile);

	// Write the file to the etc folder.
	EtcFolder.FileRecords.Add(HostnameFile.ID);

	// Write the folder to the computer FS.
	PlayerComputer.Filesystem.Add(EtcFolder);

	// How's that for manual file I/O? We just set the computer's hostname...
	// BEFORE PEACEGATE WAS ACTIVATED.

	// Now we create the root user and root folder.
	// We do the same for the player user.
	FUser RootUser;
	RootUser.ID = 0;
	RootUser.Username = "root";
	RootUser.Password = "";
	RootUser.Domain = EUserDomain::Administrator;

	FUser PlayerUser;
	PlayerUser.ID = 1;
	PlayerUser.Username = Username;
	PlayerUser.Password = "";
	PlayerUser.Domain = EUserDomain::PowerUser;

	// Now Peacegate can identify as these users.
	PlayerComputer.Users.Add(RootUser);
	PlayerComputer.Users.Add(PlayerUser);
	
	// But they still need home directories.
	// Create a new folder called "etc".
	FFolder RootFolder;
	RootFolder.FolderID = 2;
	RootFolder.FolderName = "root";
	RootFolder.ParentID = 0;
	RootFolder.IsReadOnly = false;

	FFolder HomeFolder;
	HomeFolder.FolderID = 3;
	HomeFolder.FolderName = "home";
	HomeFolder.ParentID = 0;
	HomeFolder.IsReadOnly = false;

	FFolder UserFolder;
	UserFolder.FolderID = 4;
	UserFolder.FolderName = Username;
	UserFolder.ParentID = 3;
	UserFolder.IsReadOnly = false;
	HomeFolder.SubFolders.Add(UserFolder.FolderID);

	// Write the three new folders to the disk.
	PlayerComputer.Filesystem.Add(RootFolder);
	PlayerComputer.Filesystem.Add(HomeFolder);
	PlayerComputer.Filesystem.Add(UserFolder);

	// Wallpaper needs to be nullptr to prevent a crash.
	PlayerComputer.CurrentWallpaper = nullptr;
	
	// Next thing we need to do is assign these folder IDs as sub folders to the root.
	PlayerComputer.Filesystem[0].SubFolders.Add(EtcFolder.FolderID);
	PlayerComputer.Filesystem[0].SubFolders.Add(RootFolder.FolderID);
	PlayerComputer.Filesystem[0].SubFolders.Add(HomeFolder.FolderID);
	

	// Now, we add the computer to the save file.
	SaveGame->Computers.Add(PlayerComputer);

	// Set the player UID to that of the non-root user on that computer.
	// This makes Peacenet auto-login to this user account
	// when the desktop loads.
	SaveGame->PlayerUserID = PlayerUser.ID;

	// Now we create a Peacenet Identity.
	FPeacenetIdentity PlayerIdentity;

	// Set it up as a player identity so procedural generation doesn't touch it.
	PlayerIdentity.ID = 0;
	PlayerIdentity.CharacterType = EIdentityType::Player;

	// The player identity needs to own their computer for the
	// game to auto-possess it.
	PlayerIdentity.ComputerID = PlayerComputer.ID;

	// Set the name of the player.
	PlayerIdentity.CharacterName = InCharacterName;

	// Set default skill and reputation values.
	PlayerIdentity.Skill = 0;
	PlayerIdentity.Reputation = 0.f;

	// Add the character to the save file.
	SaveGame->Characters.Add(PlayerIdentity);

	// Save the game.
	UGameplayStatics::SaveGameToSlot(SaveGame, "PeacegateOS", 0);
}

TArray<UPeacenetGameTypeAsset*> const& UPeacenetGameInstance::GetGameTypes() const {
	return this->GameTypes;
}

UPeacenetSettings * UPeacenetGameInstance::GetSettings() {
	return this->Settings;
}

void UPeacenetGameInstance::SaveSettings() {
	// Save the settings save to disk.
	UGameplayStatics::SaveGameToSlot(this->Settings, "PeacenetSettings", 0);

	this->SettingsApplied.Broadcast(this->Settings);
}

void UPeacenetGameInstance::LoadSettings() {
	// Load it in.
	this->Settings = Cast<UPeacenetSettings>(UGameplayStatics::LoadGameFromSlot("PeacenetSettings", 0));
}

void UPeacenetGameInstance::Init() {
	// Do we have a settings save?
	if (UGameplayStatics::DoesSaveGameExist("PeacenetSettings", 0)) {
		this->LoadSettings();
	} else {
		// Create a new save.
		this->Settings = NewObject<UPeacenetSettings>();

		this->SaveSettings();
	}

		// Get the Asset Registry
	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");

	// A place to store computer type asset data
	TArray<FAssetData> Assets;

	if (!AssetRegistryModule.Get().GetAssetsByClass("PeacenetGameTypeAsset", Assets, true)) {
		check(false);
	}

	for (auto& Asset : Assets) {
		this->GameTypes.Add(Cast<UPeacenetGameTypeAsset>(Asset.GetAsset()));
	}

	// Register our own daemons.
	this->RegisterDaemons();

	// Get all daemon assets for blueprint daemons.
	TArray<FAssetData> DaemonAssets;
	AssetRegistryModule.Get().GetAssetsByClass("DaemonInfoAsset", DaemonAssets, true);

	// Loop through them and register them.
	for(auto& Asset : DaemonAssets) {
		UDaemonInfoAsset* InfoAsset = Cast<UDaemonInfoAsset>(Asset.GetAsset());

		// Register the daemon!
		this->RegisterPeacegateDaemon(InfoAsset->DaemonClass, InfoAsset->Name, InfoAsset->FriendlyName, InfoAsset->Description, InfoAsset->DaemonType);
	}

	// Load or create the Profile.
	if(UGameplayStatics::DoesSaveGameExist("Profile", 0))  {
		this->Profile = Cast<UProfile>(UGameplayStatics::LoadGameFromSlot("Profile", 0));
	} else {
		this->Profile = NewObject<UProfile>();
	}

	Super::Init();
}

bool UPeacenetGameInstance::HasOldSaveFile() {
	// If there is no profile data yet there is still a Peacegate state file, then we have an old pre-0.3.x save file we can
	// convert if the player chooses to.
	return APeacenetWorldStateActor::HasExistingOS() && !this->Profile->ProfileData.Num();
}

void UPeacenetGameInstance::LoadAndConvertOldSave() {
	if(this->HasOldSaveFile())  {
		// Old saves are on PeacegateOS (user 0).
		UPeacenetSaveGame* OldSave = Cast<UPeacenetSaveGame>(UGameplayStatics::LoadGameFromSlot("PeacegateOS", 0));

		// We can create a profile slot out of the player's computer data.  Password will just be "oldsave".
		FComputer OldPC;
		int OldIndex;
		bool result = OldSave->GetComputerByID(OldSave->PlayerComputerID, OldPC, OldIndex);
		check(result);

		if(result) {
			// Create new profile data.
			FProfileData NewProfile;
			NewProfile.Username = OldPC.Users[OldPC.Users.Num() - 1].Username; // give it the actual username
			NewProfile.Password = "oldsave"; // I wasn't fucking kidding about the password.
			NewProfile.Created = FDateTime::Now();
			NewProfile.LastPlayed = NewProfile.Created;

			// Add it to the profile list.
			this->Profile->ProfileData.Add(NewProfile);
		}
	}
}

bool UPeacenetGameInstance::LoadGame(APlayerController* InPlayerController, FString Username, FString Password, APeacenetWorldStateActor*& WorldState) {
	for(int i = 0; i < this->Profile->ProfileData.Num(); i++) {
		FProfileData& Data = this->Profile->ProfileData[i];
		if(Data.Username == Username && Data.Password == Password) {
			Data.LastPlayed = FDateTime::Now();
			WorldState = APeacenetWorldStateActor::LoadExistingOS(InPlayerController, Data.SlotId);
			return true;
		}
	}
	return false;
}

TArray<FProfileData> UPeacenetGameInstance::GetProfiles() {
	return this->Profile->ProfileData;
}

bool UPeacenetGameInstance::GetMostRecentCredentials(FString& Username, FString& Password)
{
	// Fuck it.  For this function I'm using Cassian-style curly braces and anyone who interjects can go
	// fuck themselves.  - Michael
	if(!this->Profile->ProfileData.Num()) {
		return false;
	}

	int RecentIndex = -1;
	FDateTime Min = FDateTime::MinValue();

	for(int i = 0; i < this->Profile->ProfileData.Num(); i++) {
		FDateTime LastPlayed = this->Profile->ProfileData[i].LastPlayed;
		if(LastPlayed > Min) {
			RecentIndex = i;
			Min = LastPlayed;
		}
	}

	Username = this->Profile->ProfileData[RecentIndex].Username;
	Password = this->Profile->ProfileData[RecentIndex].Password;
	return true;
}

void UPeacenetGameInstance::Shutdown() {
	// Save the Profile.
	UGameplayStatics::SaveGameToSlot(this->Profile, "Profile", 0);

	// Unreal Engine's about to shut down.	
	this->SaveSettings();

	// AND NOW WE'RE GOING TO ABANDON SHIP.
	this->GameTypes.Empty();
	this->Settings = nullptr;

	// KILLLLLLLL EVERYTHINGGGGGGGGG
	Super::Shutdown();
}

TArray<FDaemonInfo> UPeacenetGameInstance::GetDaemonsForSystem(USystemContext* InSystemContext) {
	TArray<FDaemonInfo> Ret;

	for(auto RegisteredDaemon : this->RegisteredDaemons) {
		if(RegisteredDaemon.Value.DaemonType == EDaemonType::AllSystems) {
			Ret.Add(RegisteredDaemon.Value);
		} else {
			if(RegisteredDaemon.Value.DaemonType == EDaemonType::Player) {
				if(InSystemContext->GetComputer().OwnerType == EComputerOwnerType::Player) {
					Ret.Add(RegisteredDaemon.Value);
				}
				continue;
			} else {
				if(InSystemContext->GetComputer().OwnerType != EComputerOwnerType::Player) {
					Ret.Add(RegisteredDaemon.Value);
				}
				continue;
			}
		}
	}

	return Ret;
}