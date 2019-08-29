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


#include "ProceduralGenerationEngine.h"
#include "PeacenetSaveGame.h"
#include "Base64.h"
#include "FirewallRule.h"
#include "ComputerService.h"
#include "UserContext.h"
#include "LootableFile.h"
#include "ProtocolVersion.h"
#include "CommonUtils.h"
#include "WallpaperAsset.h"
#include "PeacegateFileSystem.h"
#include "StoryCharacter.h"
#include "MarkovChain.h"
#include "PeacenetSiteAsset.h"
#include "PeacegateFileSystem.h"
#include "StoryComputer.h"
#include "FileUtilities.h"
#include "PeacenetWorldStateActor.h"

void UProceduralGenerationEngine::Setup(APeacenetWorldStateActor* InPeacenet) {
    check(InPeacenet);
    check(!this->Peacenet);

    this->Peacenet = InPeacenet;

    // Load story entity data
    this->Peacenet->LoadAssets<UStoryCharacter>("StoryCharacter", this->StoryCharacters);
    this->Peacenet->LoadAssets<UStoryComputer>("StoryComputer", this->StoryComputers);

    // Markov training data:
    this->Peacenet->LoadAssets<UMarkovTrainingDataAsset>("MarkovTrainingDataAsset", this->MarkovTrainingData);

    // Protocols.
    this->Peacenet->LoadAssets<UComputerService>("ComputerService", this->ComputerServices);
    this->Peacenet->LoadAssets<UProtocolVersion>("ProtocolVersion", this->Protocols);

    // Top-level domains - hard-coded right now.
    this->TopLevelDomains = {
        ".com",
        ".net",
        ".org",
        ".mail",
        ".xyz",
        ".space",
        ".sos",
        ".os",
        ".info",
        ".abc"
    };
}

FString UProceduralGenerationEngine::PickTopLevelDomain() {
    return this->TopLevelDomains[this->Rng.RandRange(0, this->TopLevelDomains.Num() - 1)];
}

int UProceduralGenerationEngine::CreateIdentity() {
    int EntityID = 0;
    TArray<int> Taken;

    // Collect all taken entity IDs.
    for(auto Identity : this->SaveGame->Characters) {
        Taken.Add(Identity.ID);
    }
    
    // Find the lowest entity ID that's not taken.
    while(Taken.Contains(EntityID)) {
        EntityID++;
    }

    // Create the new identity.
    FPeacenetIdentity NewIdentity = FPeacenetIdentity();
    NewIdentity.ID = EntityID;

// Logging
    UE_LOG(LogTemp, Log, TEXT("Generated Peacenet Identity entity ID %s"), *FString::FromInt(NewIdentity.ID));

    // Add to the save file; return the ID.
    this->SaveGame->Characters.Add(NewIdentity);
    return NewIdentity.ID;
}

FPeacenetIdentity& UProceduralGenerationEngine::GetIdentity(int EntityID) {
    for(FPeacenetIdentity& Identity : this->SaveGame->Characters) {
        if(Identity.ID == EntityID) {
            return Identity;
        }
    }
    return this->InvalidIdentity;
}

void UProceduralGenerationEngine::UpdateStoryCharacter(UStoryCharacter* InStoryCharacter) {
    // Find the entity IF of the story character.
    int EntityID = -1;
    for(auto Map : this->SaveGame->StoryCharacterIDs) {
        if(Map.CharacterAsset == InStoryCharacter) {
            EntityID = Map.Identity;
            break;
        }
    }

    // If the entity is not valid then create a new one.
    if(EntityID == -1) {
        EntityID = this->CreateIdentity();
    }

    // Get a reference to the entity.
    FPeacenetIdentity& Identity = this->GetIdentity(EntityID);

    // Update the entity's data based on what's in the story character asset.
    Identity.CharacterName = InStoryCharacter->Name.ToString();
    Identity.CharacterType = EIdentityType::Story;
    Identity.ComputerID = -1;
    Identity.PreferredAlias = (InStoryCharacter->UseNameForEmail || !InStoryCharacter->EmailAlias.Len()) ? Identity.CharacterName : InStoryCharacter->EmailAlias;
    Identity.EmailAddress = UCommonUtils::Aliasify(Identity.PreferredAlias);

    // Assign the entity to the story character asset.
    this->SaveGame->AssignStoryCharacterID(InStoryCharacter, EntityID);
}

int UProceduralGenerationEngine::CreateComputer() {
    int EntityID = 0;

    // Collect all taken computer IDs
    TArray<int> Taken;
    for(FComputer& Computer : this->SaveGame->Computers) {
        Taken.Add(Computer.ID);
    }

    // Find the first computer ID that isn't taken.
    while(Taken.Contains(EntityID)) {
        EntityID++;
    }

    // Create a computer with this ID
    FComputer NewPC;
    NewPC.ID = EntityID;
    NewPC.SystemIdentity = -1;
    NewPC.PeacenetSite = nullptr;
    NewPC.CurrentWallpaper = nullptr;

    // Format its filesystem.
    UFileUtilities::FormatFilesystem(NewPC.Filesystem);

    // Create a root user account.
    FUser Root;
    Root.ID = 0;
    Root.Username = "root";
    Root.Domain = EUserDomain::Administrator;

    // Add the root user to the computer.
    NewPC.Users.Add(Root);

    // Assign a public IP for the computer.
    this->SaveGame->ComputerIPMap.Add(this->GenerateIPAddress(), NewPC.ID);

    // Logging
    UE_LOG(LogTemp, Log, TEXT("Generated computer entity ID %s"), *FString::FromInt(NewPC.ID));

    // Add the computer to the save file; return its ID
    this->SaveGame->Computers.Add(NewPC);
    return NewPC.ID;
}

FComputer& UProceduralGenerationEngine::GetComputer(int EntityID) {
    for(FComputer& Computer : this->SaveGame->Computers) {
        if(Computer.ID == EntityID) {
            return Computer;
        }
    }
    return this->InvalidPC;
}

void UProceduralGenerationEngine::CreateUser(FComputer& InComputer, FString InUsername, bool Sudoer) {
    for(FUser& User : InComputer.Users) {
        if(User.Username == InUsername) {
            User.Domain = (Sudoer) ? EUserDomain::PowerUser : EUserDomain::User;
            return;
        }
    }

    FUser NewUser;
    NewUser.ID = InComputer.Users.Num();
    NewUser.Username = InUsername;
    NewUser.Domain = (Sudoer) ? EUserDomain::PowerUser : EUserDomain::User;
    InComputer.Users.Add(NewUser);
}

FString UProceduralGenerationEngine::GenerateIPAddress() {
    FString IPAddress;

    do {
        int First = this->Rng.RandRange(16, 254);
        int Second = this->Rng.RandRange(0, 255);
        int Third = this->Rng.RandRange(0, 255);
        int Fourth = this->Rng.RandRange(0, 255);

        IPAddress = FString::FromInt(First) + "." + FString::FromInt(Second) + "." + FString::FromInt(Third) + "." + FString::FromInt(Fourth);
    } while(this->SaveGame->ComputerIPMap.Contains(IPAddress));

// Logging
    UE_LOG(LogTemp, Log, TEXT("Generated IP address: %s"), *IPAddress);

    return IPAddress;
}

void UProceduralGenerationEngine::UpdateStoryComputer(UStoryComputer* InStoryComputer) {
    // Find the computer ID for the story computer.
    int EntityID = -1;
    for(auto ComputerMap : this->SaveGame->StoryComputerIDs) {
        if(ComputerMap.StoryComputer == InStoryComputer) {
            EntityID = ComputerMap.Entity;
            break;
        }
    }

    // If it doesn't exist, create a new one.
    if(EntityID == -1) {
        EntityID = this->CreateComputer();
    }

    // Get a reference to the computer so we can update system identity, users,
    // other stuff like that.
    FComputer& Computer = this->GetComputer(EntityID);

    // Update the system identity if possible.
    Computer.SystemIdentity = -1;
    if(InStoryComputer->OwningCharacter) {
        this->SaveGame->GetStoryCharacterID(InStoryComputer->OwningCharacter, Computer.SystemIdentity);
    } 

    // Create all users that don't exist.
    for(auto Username : InStoryComputer->Users) {
        FString Unixified = UCommonUtils::Aliasify(Username);
        if(Unixified.Len()) {
            this->CreateUser(Computer, Unixified, false);
        }
    }

    // Get a system context for the computer.
    USystemContext* SystemContext = this->Peacenet->GetSystemContext(Computer.ID);

    // Get a root user context.
    UUserContext* UserContext = SystemContext->GetUserContext(0);

    // Update the hostname.
    EFilesystemStatusCode Status = EFilesystemStatusCode::OK;
    UPeacegateFileSystem* FS = UserContext->GetFilesystem();
    if(!FS->DirectoryExists("/etc")) {
        FS->CreateDirectory("/etc", Status);
    }
    FString Host = UCommonUtils::Aliasify(InStoryComputer->Hostname);
    if(Host.Len()) {
        FS->WriteText("/etc/hostname", Host);
    } else {
        FS->WriteText("/etc/hostname", "localhost");
    }

    // Assign the domain name for the computer.
    FString Domain = UCommonUtils::Aliasify(InStoryComputer->DomainName);
    if(Domain.Len()) {
        FString IP = this->Peacenet->GetIPAddress(Computer);
        if(this->SaveGame->DomainNameMap.Contains(Domain)) {
            this->SaveGame->DomainNameMap[Domain] = IP;
        } else {
            this->SaveGame->DomainNameMap.Add(Domain, IP);
        }
    }

    // Set firewall rules.

    // Spawn/update file lootables.

    // Assign the computer ID to the computer asset.
    for(auto& CompMap : this->SaveGame->StoryComputerIDs) {
        if(CompMap.StoryComputer == InStoryComputer) {
            CompMap.Entity = Computer.ID;
            return;
        }
    }
    FStoryComputerMap Map;
    Map.StoryComputer = InStoryComputer;
    Map.Entity = Computer.ID;
    this->SaveGame->StoryComputerIDs.Add(Map);
}

void UProceduralGenerationEngine::GenerateNPC() {
    // Generate an entity ID.
    int Entity = this->CreateIdentity();

    // Get a reference to the entity.
    FPeacenetIdentity& Identity = this->GetIdentity(Entity);

    // Create a computer for the entity.
    int ComputerEntity = this->CreateComputer();
    FComputer& Computer = this->GetComputer(ComputerEntity);

    // Assign the entity to the computer, and mark them both as non-player.
    Identity.CharacterType = EIdentityType::NonPlayer;
    Computer.SystemIdentity = Identity.ID;
    Computer.OwnerType = EComputerOwnerType::NPC;

    // Determine the biolgical sex of the Peacenet Identity so we know whether to generate a male or female
    // first name if the game decides that the name to generate is not uni-sex.
    Identity.Sex = this->DetermineSex();

    // Generate a unique name for this identity.
    this->GenerateHumanName(Identity);

    // Set the preferred alias and email username for the identity.
    Identity.PreferredAlias = Identity.CharacterName;
    Identity.EmailAddress = UCommonUtils::Aliasify(Identity.PreferredAlias);

    // Create a Peacegate user for the identity's Computer.
    FString FirstName = UCommonUtils::GetFirstName(FText::FromString(Identity.CharacterName)).ToString();
    this->CreateUser(Computer, UCommonUtils::Aliasify(FirstName), false);

    // Get a filesystem context for the computer to make writing files easier...
    USystemContext* SystemContext = this->Peacenet->GetSystemContext(Computer.ID);
    UPeacegateFileSystem* FS = SystemContext->GetFilesystem(0);

    // Create the /etc directory if not found, then write the hostname to /etc/hostname.
    if(!FS->DirectoryExists("/etc")) {
        EFilesystemStatusCode Status = EFilesystemStatusCode::OK;
        FS->CreateDirectory("/etc", Status);
    }
    FS->WriteText("/etc/hostname", UCommonUtils::Aliasify(FirstName) + "-pc");
}

void UProceduralGenerationEngine::GenerateHumanName(FPeacenetIdentity& Identity) {
    do {
        FString First = "";
        FString Last = "";

        // TODO: Unisex names
        // The first name depends on the sex of the identity.
        if(Identity.Sex == ESex::Male) {
            First = this->MaleNameGenerator->GetMarkovString(0);
        } else {
            First = this->FemaleNameGenerator->GetMarkovString(0);
        }

        // Last name doesn't matter
        Last = this->LastNameGenerator->GetMarkovString(0);

        // Set the character name.
        Identity.CharacterName = First + " " + Last;
    } while(this->UsedHumanNames.Contains(Identity.CharacterName));

    // Prevent this exact name from being generated again.
    this->UsedHumanNames.Add(Identity.CharacterName);

    // Logging.
    UE_LOG(LogTemp, Log, TEXT("Identity %d is now named %s."), Identity.ID, *Identity.CharacterName);
}

void UProceduralGenerationEngine::Update(float DeltaTime) {
    check(this->Peacenet);

    // Update all story characters, then all story computers, but only do one per frame.
    if(StoryCharactersToUpdate.Num()) {
        int Index = StoryCharactersToUpdate.Pop();
        this->UpdateStoryCharacter(this->StoryCharacters[Index]);
    } else if(StoryComputersToUpdate.Num()) {
        int Index = StoryComputersToUpdate.Pop();
        this->UpdateStoryComputer(this->StoryComputers[Index]);
    } else if(ComputersNeedingIPAddresses.Num()) {
        // Generate an IP address for any computer that needs one.
        int ComputerID = this->ComputersNeedingIPAddresses.Pop();
        FString IPAddress = this->GenerateIPAddress();
        this->SaveGame->ComputerIPMap.Add(IPAddress, ComputerID);
    } else if(this->EmailServersToGenerate > 0) {
        this->GenerateEmailServer();
        this->EmailServersToGenerate--;
    } else {
        // Generate domain names for email servers.
        if(this->EmailServersNeedingDomains.Num()) {
            int Entity = this->EmailServersNeedingDomains.Pop();
            this->SetDomainName(Entity);
        }

        // Send mission emails if we've just updated.
        if(this->JustUpdated) {
            this->Peacenet->SendAvailableMissions();
            this->JustUpdated = false;
        }
        
        // Generate any non-player identities that are left.
        if(this->NonPlayerIdentitiesToGenerate > 0) {
            this->GenerateNPC();
            this->NonPlayerIdentitiesToGenerate--;
        }
    }
}

void UProceduralGenerationEngine::GenerateEmailServer() {
    // Generate a computer ID.
    int Entity = this->CreateComputer();

    // Get the computer as a reference.
    FComputer& Computer = this->GetComputer(Entity);

    // Assign the computer type as an Email Server.
    Computer.ComputerType = EComputerType::EmailServer;
    Computer.SystemIdentity = -1;
    Computer.OwnerType = EComputerOwnerType::None;

    // Queue this server to receive a domain name.
    this->EmailServersNeedingDomains.Add(Computer.ID);
}

void UProceduralGenerationEngine::ResetState() {
    // Define some hardcoded constants for the world.
    const int MAX_NPC_IDENTITIES = 1000;
    const int MAX_EMAIL_SERVERS = 20;
    const int MARKOV_HUMAN_NAME_READABILITY = 4;
    const int HOSTNAME_READABILITY = 3;

    // Determine the world's seed and re-initialize the random number stream.
    if(this->SaveGame->IsNewGame) {
        this->SaveGame->WorldSeed = (int)FDateTime::Now().ToUnixTimestamp();
        this->SaveGame->IsNewGame = false;
    }
    this->Rng = FRandomStream(this->SaveGame->WorldSeed);

    // Determine which story characters need to spawn.
    this->StoryCharactersToUpdate.Empty();
    for(int i = 0; i < this->StoryCharacters.Num(); i++) {
        this->StoryCharactersToUpdate.Push(i);
    }

    // Determine which story computers need to spawn.
    this->StoryComputersToUpdate.Empty();
    for(int i = 0; i < this->StoryComputers.Num(); i++) {
        this->StoryComputersToUpdate.Push(i);
    }

    // Dtermine how many identities need to spawn.
    this->NonPlayerIdentitiesToGenerate = MAX_NPC_IDENTITIES;
    for(FPeacenetIdentity& Identity : this->SaveGame->Characters) {
        if(Identity.CharacterType == EIdentityType::NonPlayer) {
            this->NonPlayerIdentitiesToGenerate--;
            if(this->NonPlayerIdentitiesToGenerate <= 0) {
                break;
            }
        }
    }

    // MARKOV DATA FOR IDENTITIES
    // TODO: Unisex names
    TArray<FString> MaleFirstNames;
    TArray<FString> LastNames;
    TArray<FString> FemaleFirstNames;
    TArray<FString> Hostnames;

    // Sift through the metric fuckton of markov training data in this game
    for(auto TrainingData : this->MarkovTrainingData) {
        if(TrainingData->Usage == EMarkovTrainingDataUsage::FemaleFirstNames) {
            FemaleFirstNames.Append(TrainingData->TrainingData);
        } else if(TrainingData->Usage == EMarkovTrainingDataUsage::MaleFirstNames) {
            MaleFirstNames.Append(TrainingData->TrainingData);
        } else if(TrainingData->Usage == EMarkovTrainingDataUsage::LastNames) {
            LastNames.Append(TrainingData->TrainingData);
        } else if(TrainingData->Usage == EMarkovTrainingDataUsage::Hostnames) {
            Hostnames.Append(TrainingData->TrainingData);
        }
    }

    // Create markov chains with our current RNG and training data
    this->MaleNameGenerator = NewObject<UMarkovChain>(this);
    this->FemaleNameGenerator = NewObject<UMarkovChain>(this);
    this->LastNameGenerator = NewObject<UMarkovChain>(this);
    this->DomainNameGenerator = NewObject<UMarkovChain>(this);
    this->MaleNameGenerator->Init(MaleFirstNames, MARKOV_HUMAN_NAME_READABILITY, this->Rng);
    this->FemaleNameGenerator->Init(FemaleFirstNames, MARKOV_HUMAN_NAME_READABILITY, this->Rng);
    this->LastNameGenerator->Init(LastNames, MARKOV_HUMAN_NAME_READABILITY, this->Rng);
    this->DomainNameGenerator->Init(Hostnames, HOSTNAME_READABILITY, this->Rng);

    // Clear all used human names.
    this->UsedHumanNames.Empty();

    // Determine which computers need IP addresses.
    this->ComputersNeedingIPAddresses.Empty();
    TArray<int> IPs;
    for(auto& IP : this->SaveGame->ComputerIPMap) {
        IPs.Add(IP.Value);
    }
    for(auto& Computer : this->SaveGame->Computers) {
        if(!IPs.Contains(Computer.ID)) {
            this->ComputersNeedingIPAddresses.Push(Computer.ID);
        }
    }

    // Determine how many email servers we need to generate.
    this->EmailServersToGenerate = MAX_EMAIL_SERVERS;
    this->EmailServersNeedingDomains.Empty();
    for(FComputer& Computer : this->SaveGame->Computers) {
        if(Computer.ComputerType == EComputerType::EmailServer) {
            this->EmailServersToGenerate--;
            for(auto& IPAddress : this->SaveGame->ComputerIPMap) {
                if(IPAddress.Value == Computer.ID) {
                    bool HasDomain = false;
                    for(auto& Domain : this->SaveGame->DomainNameMap) {
                        HasDomain = true;
                        break;
                    }
                    if(!HasDomain) {
                        this->EmailServersNeedingDomains.Add(Computer.ID);
                    }
                    break;
                }
            }
        }
    }
}

void UProceduralGenerationEngine::GiveSaveGame(UPeacenetSaveGame* InSaveGame) {
    if(this->SaveGame != InSaveGame) {
        this->SaveGame = InSaveGame;
        this->ResetState();
        this->JustUpdated = true;
    }
}

bool UProceduralGenerationEngine::IsDoneGeneratingStoryCharacters() {
    return !this->StoryCharactersToUpdate.Num();
}

ESex UProceduralGenerationEngine::DetermineSex() {
    int DiceRoll = this->Rng.RandRange(1, 6);
    if(DiceRoll % 2) {
        return ESex::Male;
    } else {
        return ESex::Female;
    }
}

void UProceduralGenerationEngine::SpawnServices(int ComputerID) {
    // Get a reference to the computer with this ID.
    FComputer& Computer = this->GetComputer(ComputerID);

    // Since story computers can explicitly define services to spawn, we need to figure out if this is a
    // story computer and spawn those if possible.
    for(auto& StoryComputer : this->SaveGame->StoryComputerIDs) {
        if(StoryComputer.Entity == Computer.ID) {
            TArray<FFirewallRule> FirewallRules;
            for(auto Protocol : StoryComputer.StoryComputer->Services) {
                if(Protocol.ProtocolVersion) {
                    // Spawn the service right here since the story computer asset knows whether the firewall should filter
                    // it
                    FFirewallRule FirewallRule;
                    FirewallRule.IsCrashed = false;
                    FirewallRule.IsFiltered = Protocol.Filtered;
                    FirewallRule.Service = Protocol.ProtocolVersion;
                    FirewallRule.Port = FirewallRule.Service->Protocol->Port;
                    FirewallRules.Add(FirewallRule);
                }
            }

            // If we ended up spawning firewall rules then we'll overwrite those found in the computer.
            if(FirewallRules.Num()) {
                Computer.FirewallRules = FirewallRules;
            }
        }
    }

    // If we still don't have any protocols then, either the computer is an NPC (non-story) computer, or it's a 
    // story computer but the creator of the asset didn't define any services.
    if(!Computer.FirewallRules.Num()) {
        // These are the protocol types we want to spawn.
        TArray<UComputerService*> ProtocolTypesToSpawn;

        // How many unique ones do we want to spawn?
        // TODO: Service spawn conditions.
        int AmountToSpawn = this->Rng.RandRange(1, this->ComputerServices.Num());

        // Pick service types at random.
        while(AmountToSpawn) {
            int i = this->Rng.RandRange(0, this->ComputerServices.Num() - 1);
            if(!ProtocolTypesToSpawn.Contains(this->ComputerServices[i])) {
                ProtocolTypesToSpawn.Add(this->ComputerServices[i]);
                AmountToSpawn--;
            }
        }

        // Now we pick protocol implementations for each service.
        for(auto ServiceType : ProtocolTypesToSpawn) {
            int i = 0;
            int ChooserCounter = this->Protocols.Num() * 10;
            UProtocolVersion* Protocol = nullptr;
            do {
                UProtocolVersion* Candidate = this->Protocols[i];
                if(Candidate->Protocol == ServiceType) {
                    Protocol = Candidate;
                    ChooserCounter -= this->Rng.RandRange(1, 5);
                }
                i++;
                if(i >= this->Protocols.Num()) {
                    // If a candidate has not yet been chosen yet we have hit the end of the list, break
                    // out of this fucking loop because 0.3.0 is supposed to fix that infinite loop bug.
                    if(!Protocol) {
                        break;
                    }
                    i = 0;
                }
            } while (!Protocol && ChooserCounter > 0);
            
            // Spawn the firewall rule! But only if we chose a protocol.
            if(Protocol) {
                FFirewallRule FirewallRule;
                FirewallRule.IsCrashed = false;
                FirewallRule.IsFiltered = this->Rng.RandRange(1, 6) % 2;
                FirewallRule.Service = Protocol;
                FirewallRule.Port = FirewallRule.Service->Protocol->Port;
                Computer.FirewallRules.Add(FirewallRule);
            }
        }
    }
}

void UProceduralGenerationEngine::SetDomainName(int Entity) {
    // Retrieve a reference to the computer.
    FComputer& Computer = this->GetComputer(Entity);

    // Retrieve the IP address of the computer so we can map a domain name
    // to it.
    FString IPAddress = "";
    for(auto& IPMap : this->SaveGame->ComputerIPMap) {
        if(IPMap.Value == Computer.ID) {
            IPAddress = IPMap.Key;
            break;
        }
    }

    // If the computer does not have an IP address we'll generate one.
    if(!IPAddress.Len()) {
        IPAddress = this->GenerateIPAddress();
        this->SaveGame->ComputerIPMap.Add(IPAddress, Computer.ID);
    }

    // Generate a unique domain name for the computer.
    FString DomainName = "";
    do {
        DomainName = UCommonUtils::Aliasify(this->DomainNameGenerator->GetMarkovString(0)) + this->PickTopLevelDomain();
    } while(this->SaveGame->DomainNameMap.Contains(DomainName));
}