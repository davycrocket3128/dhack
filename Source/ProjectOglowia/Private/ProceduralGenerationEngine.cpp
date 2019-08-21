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
#include "PeacenetWorldStateActor.h"

void UProceduralGenerationEngine::GenerateLinks(FComputer& InOrigin) {
    // COMPUTER LINK GENERATION
    //
    // Computer Links are the game's underlying representation of the Peacenet
    // network.  They are used internally by nmap for adjacent node scanning.
    //
    // Prerequisites:
    //  - A computer should have no more than 5 links, and no less than one link.
    //  - Two computers should be linked together if they own the same
    //    Peacenet Identity.  This is an exception to the max links rule.
    //  - A player computer must have at least one hackable link.
    //  - Story computers should be linked to the player if their
    //    Story Character metadata states as such.
    // 
    // Story computers and same-identity computers are considered "special"
    // by the game and thus DO NOT count against the max-links limit.  However
    // they do count as meeting the minimum requirement of one link.
    //
    // This is so that a computer doesn't HAVE to be linked to a non-special link
    // if there are enough special links to meet the minimum link requirement.
    

    // First we'll start with same-identity links.  This only happens
    // if the computer given to us has an identity.
    if(InOrigin.SystemIdentity > -1) {
        // Iterate through all computers in the game.
        for(FComputer& Computer : this->Peacenet->SaveGame->Computers) {
            // Check if the identity matches.
            if(Computer.SystemIdentity == InOrigin.SystemIdentity) {
                // If the two computers are not linked, add a link.
                if(!this->Peacenet->SaveGame->GetLinkedSystems(InOrigin).Contains(Computer.ID)) {
                    FComputerLink NewLink;
                    NewLink.ComputerA = InOrigin.ID;
                    NewLink.ComputerB = Computer.ID;
                    this->Peacenet->SaveGame->ComputerLinks.Add(NewLink);
                }
            }
        }
    }

    // Now, we'll go through all of the story characters in the game.
    if(InOrigin.OwnerType == EComputerOwnerType::Player) {
        for(auto StoryChar : this->Peacenet->SaveGame->StoryCharacterIDs) {
            // Check if the character should link to the player.
            if(StoryChar.CharacterAsset->ShouldLinkToPlayer) {
                // Find the computer whose system identity matches the story character.
                for(FComputer& Computer : this->Peacenet->SaveGame->Computers)    {
                    if(Computer.SystemIdentity == StoryChar.Identity) {
                        // Add a link if these systems are not linked.
                        if(!this->Peacenet->SaveGame->GetLinkedSystems(InOrigin).Contains(Computer.ID)) {
                            FComputerLink NewLink;
                            NewLink.ComputerA = InOrigin.ID;
                            NewLink.ComputerB = Computer.ID;
                            this->Peacenet->SaveGame->ComputerLinks.Add(NewLink);
                        }
                    }
                }
            }
        }
    }

    // Determine how many non-special systems we need to link.
    int LinksToFulfill = 5;
    int Specials = 0;
    for(int Link : this->Peacenet->SaveGame->GetLinkedSystems(InOrigin)) {
        FComputer LinkedSys;
        int LinkedIndex = -1;
        this->Peacenet->SaveGame->GetComputerByID(Link, LinkedSys, LinkedIndex);

        if(LinkedSys.SystemIdentity == InOrigin.SystemIdentity && InOrigin.SystemIdentity > -1)  {
            Specials++;
            continue;
        }

        bool IsStory = false;
        if(InOrigin.OwnerType == EComputerOwnerType::Player) {
            for(auto StoryChar : this->Peacenet->SaveGame->StoryCharacterIDs) {
                if(StoryChar.CharacterAsset->ShouldLinkToPlayer && StoryChar.Identity == LinkedSys.SystemIdentity) {   
                    Specials++;
                    IsStory = true;
                    break;
                }
            }
        }

        if(!IsStory) LinksToFulfill--;

        if(LinksToFulfill <= 0) break;
    }

    // Generate non-special links while we still need to.
    int NextLinkId = -1;
    while(LinksToFulfill && this->GetNextLink(InOrigin, NextLinkId)) {
        // Link the fucker!
        FComputerLink NewLink;
        NewLink.ComputerA = InOrigin.ID;
        NewLink.ComputerB = NextLinkId;
        this->Peacenet->SaveGame->ComputerLinks.Add(NewLink);

        LinksToFulfill--;
    }

    // Check to make sure the computer has the minimum amount of links.
    check(this->Peacenet->SaveGame->GetLinkedSystems(InOrigin).Num() > 0);

    // Check to make sure there are no more than 5 non-special links.
    check(this->Peacenet->SaveGame->GetLinkedSystems(InOrigin).Num() - Specials <= 5);
}

bool UProceduralGenerationEngine::GetNextLink(FComputer& InOrigin, int& OutLinkId) {
    // Reset link id.
    OutLinkId = -1;

    // Go through all computers in the save file.
    for(FComputer& Computer : this->Peacenet->SaveGame->Computers) {
        // Avoid the origin.
        if(Computer.ID == InOrigin.ID) {
            continue;
        }

        // Avoid same-identity computers.
        if(Computer.SystemIdentity == InOrigin.SystemIdentity && InOrigin.SystemIdentity > -1) {
            continue;
        }

        // Avoid already-linked computers.
        if(this->Peacenet->SaveGame->GetLinkedSystems(InOrigin).Contains(Computer.ID)) {
            continue;
        }

        // Avoid any computers that already have 5 non-special links.
        int NonSpecialLinks = 0;
        for(int Link : this->Peacenet->SaveGame->GetLinkedSystems(Computer)) {
            // Retrieve the computer of the link.
            FComputer LinkedSys;
            int LinkedIndex = -1;
            this->Peacenet->SaveGame->GetComputerByID(Link, LinkedSys, LinkedIndex);

            // If the system identities match it's a special link.
            if(Computer.SystemIdentity == LinkedSys.SystemIdentity && Computer.SystemIdentity > -1) {
                continue;
            }
            
            // If the computer is a player and the linked system is a story character that should be
            // linked to the player then it is a special.
            if(Computer.OwnerType == EComputerOwnerType::Player) {
                bool IsStory = false;
                for(auto StoryChar : this->Peacenet->SaveGame->StoryCharacterIDs) {
                    if(StoryChar.CharacterAsset->ShouldLinkToPlayer && StoryChar.Identity == LinkedSys.SystemIdentity) {
                        IsStory = true;
                        break;
                    }
                }
                if(IsStory) {
                    continue;
                }
            }

            // Increase non-special links count.
            NonSpecialLinks++;
        }

        if(NonSpecialLinks > 5) continue;

        // We should be fine.
        OutLinkId = Computer.ID;
        break;
    }
    return OutLinkId > -1;
}

void UProceduralGenerationEngine::UpdateStoryIdentities() {
    // Load in all of the story character assets.
    TArray<UStoryCharacter*> StoryCharacters;
    this->Peacenet->LoadAssets<UStoryCharacter>("StoryCharacter", StoryCharacters);

    // Updates all the story characters in the game. If one
    // doesn't exist, it gets created.
    for(auto Character : StoryCharacters) {
        this->UpdateStoryCharacter(Character);
    }
}

void UProceduralGenerationEngine::UpdateStoryCharacter(UStoryCharacter* InStoryCharacter) {
    // The entity ID and identity of the current (or new)
    // character.
    int EntityID = -1;

    // Does the save file already know about this character?
    if(this->Peacenet->GetStoryCharacterID(InStoryCharacter, EntityID)) {
        // Fetch the identity from the save file.
        // Debug builds crash if this fails. The save file should report that it couldn't find
        // an entity if the entity ID is invalid.
        check(this->Peacenet->IdentityExists(EntityID));
    }

    // Are we creating a new identity or editing an existing one?
    bool create = EntityID == -1;

    // Get a new identity or existing identity memory address based on the result of the above.
    FPeacenetIdentity& Identity = (create) ? this->Peacenet->GetNewIdentity() : this->Peacenet->GetCharacterByID(EntityID);

    // If we're creating, we need to do a few things.
    if(create) {
        // This makes sure there's definitely no computer for the NPC.
        // This is done later.
        Identity.ComputerID = -1;
        
        // Make sure it's a story character.
        Identity.CharacterType = EIdentityType::Story;
    }

    // Update the character's name.
    Identity.CharacterName = InStoryCharacter->Name.ToString();

    // Preferred alias of the identity is determined by whether we want to use the name
    // as the email address.
    if(InStoryCharacter->UseNameForEmail) {
        // Use the character's name as the alias.
        Identity.PreferredAlias = Identity.CharacterName;
    } else {
        // Use the email alias value as the preferred alias.
        Identity.PreferredAlias = InStoryCharacter->EmailAlias;
    }

    // So that the identity's email doesn't keep changing providers
    // every time the save file is loaded, we're going to check if it starts
    // with the preferred alias before we choose a new email.  That way we
    // are forced to preemptively generate the username part of the email.
    FString EmailUser = Identity.PreferredAlias.ToLower().Replace(TEXT(" "), TEXT("_"));

    if(!Identity.EmailAddress.StartsWith(EmailUser)) {
        // NOW we can re-assign the email address.
        Identity.EmailAddress = EmailUser + "@" + this->ChooseEmailDomain();
    }

    // If the identity doesn't have a computer ID, we need to generate one.
    if(Identity.ComputerID == -1) {
        // Create the actual computer.
        FComputer& StoryComputer = this->GenerateComputer("tempsys", EComputerType::Personal, EComputerOwnerType::Story);

        // Assign the computer ID to the identity.
        Identity.ComputerID = StoryComputer.ID;
    }

    // Reassign the character's entity ID.
    this->Peacenet->AssignStoryCharacterID(InStoryCharacter, Identity);

    // Now we can update the story computer.
    this->UpdateStoryComputer(InStoryCharacter);
}

void UProceduralGenerationEngine::UpdateStoryComputer(UStoryCharacter* InStoryCharacter) {
    // Get the identity of the story character so we can get its computer ID.
    int StoryEntityID = 0;
    bool StoryEntityIDResult = this->Peacenet->GetStoryCharacterID(InStoryCharacter, StoryEntityID);
    check(StoryEntityIDResult);

    // Check that the story character exists already.
    check(this->Peacenet->IdentityExists(StoryEntityID));

    // Retrieve a reference to the story character data.
    FPeacenetIdentity& Identity = this->Peacenet->GetCharacterByID(StoryEntityID);

    // We have a computer ID, so we're going to create a TEMPORARY System Context to make
    // updating easier.
    USystemContext* SystemContext = NewObject<USystemContext>();
    SystemContext->Setup(Identity.ComputerID, Identity.ID, this->Peacenet);

    // Peacenet 0.1.x would do this before the system context was allocated but
    // since we can no longer access the save file we'll do this now.
    // This sort-of abstracts things.
    //
    // Oh yeah, we're updating the computer's user table.

    for(auto& User : SystemContext->GetComputer().Users) {
        if(User.ID == 1) {
            if(InStoryCharacter->UseEmailAliasAsUsername) {
                if(InStoryCharacter->UseNameForEmail) {
                    User.Username = InStoryCharacter->Name.ToString().ToLower().Replace(TEXT(" "), TEXT("_"));
                } else {
                    User.Username = InStoryCharacter->EmailAlias.ToLower().Replace(TEXT(" "), TEXT("_"));
                }
            } else {
                User.Username = InStoryCharacter->Username.ToLower().Replace(TEXT(" "), TEXT("_"));
            }
            break;
        }
    }


    // Get a filesystem with root privileges from the system context.
    UPeacegateFileSystem* RootFS = SystemContext->GetFilesystem(0);

    // Collect filesystem errors.
    EFilesystemStatusCode Status = EFilesystemStatusCode::OK;

    // Update the hostname.
    FString Host = InStoryCharacter->Hostname;
    if(InStoryCharacter->UseNameForHostname) {
        Host = InStoryCharacter->Name.ToString().ToLower().Replace(TEXT(" "), TEXT("_")) + "-pc";
    }
    RootFS->WriteText("/etc/hostname", Host);

    // Now we'll start looking at exploits.
    for(auto Exploit : InStoryCharacter->Exploits) {
        // Do we already have a file on the system that refers to this exploit?
        bool ExploitExists = false;

        // Go through all file records within the system.
        for(auto& Record : SystemContext->GetComputer().FileRecords) {
            // Only check exploit records.
            if(Record.RecordType == EFileRecordType::Exploit) {
                // Check the ID first.
                if(Record.ContentID >= 0 && Record.ContentID < this->Peacenet->GetExploits().Num()) {
                    // Get the file exploit data.
                    UExploit* FileExploit = this->Peacenet->GetExploits()[Record.ContentID];
                    
                    // If the exploit IDs match, then this exploit DOES NOT need to spawn.
                    if(FileExploit->ID == Exploit->ID) {
                        ExploitExists = true;
                        break;
                    }
                }
            }
        }

        // DO NOT SPAWN THE EXPLOIT if the exploit already exists.
        if(ExploitExists) {
            continue;
        }

        // TODO: random exploit spawn folders.
        for(int i = 0; i < this->Peacenet->GetExploits().Num(); i++) {
            if(this->Peacenet->GetExploits()[i]->ID == Exploit->ID) {
                RootFS->SetFileRecord("/root/" + Exploit->ID.ToString() + ".gsf", EFileRecordType::Exploit, i);
            }
        }
    }
}

FRandomStream& UProceduralGenerationEngine::GetRNG() {
    return this->RNG;
}

void UProceduralGenerationEngine::PlaceLootableFiles(UUserContext* InUserContext) {
    // Only do this if the given user context is not that of a player.
    if(InUserContext->GetPeacenetIdentity().CharacterType != EIdentityType::Player) {
        // The amount of lootables we are EVER allowed to generate in an NPC.
        int MaxLootables = this->LootableFiles.Num() / 2;

        // If that ends up being 0, then we stop right there.
        if(MaxLootables > 0) {
            // The amount of lootables this NPC gets.
            int LootableCount = RNG.RandRange(0, MaxLootables);

            // Get a filesystem context for this user.
            UPeacegateFileSystem* Filesystem = InUserContext->GetFilesystem();

            // Keep going while there are lootables to generate.
            while(LootableCount) {
                // Pick a random lootable! Any lootable!
                ULootableFile* Lootable = this->LootableFiles[RNG.RandRange(0, this->LootableFiles.Num() - 1)];

                // This is where the file will go.
                FString BaseDirectory = "/";

                // Desktop or documents?
                int Dice = RNG.RandRange(0, 6);

                // Where should it go?
                switch(Lootable->SpawnLocation) {
                    case EFileSpawnLocation::Anywhere:
                        // TODO: Support picking of random folders.
                        break;
                    case EFileSpawnLocation::HomeFolder:
                        BaseDirectory = InUserContext->GetHomeDirectory();
                        break;
                    case EFileSpawnLocation::Binaries:
                        BaseDirectory = "/bin";
                        break;
                    case EFileSpawnLocation::DesktopOrDocuments:
                        if(Dice % 2 == 0) {
                            BaseDirectory = InUserContext->GetHomeDirectory() + "/Desktop";
                        } else {
                            BaseDirectory = InUserContext->GetHomeDirectory() + "/Documents";
                        }
                        break;
                    case EFileSpawnLocation::Pictures:
                        BaseDirectory = InUserContext->GetHomeDirectory() + "/Pictures";
                        break;
                    case EFileSpawnLocation::Downloads:
                        BaseDirectory = InUserContext->GetHomeDirectory() + "/Downloads";
                        break;
                    case EFileSpawnLocation::Music:
                        BaseDirectory = InUserContext->GetHomeDirectory() + "/Music";
                        break;
                    case EFileSpawnLocation::Videos:
                        BaseDirectory = InUserContext->GetHomeDirectory() + "/Videos";
                        break;
                }

                // Now that we have a base directory, get the file path.
                FString FilePath = BaseDirectory + "/" + Lootable->FileName.ToString();

                // If the file already exists, continue.
                if(Filesystem->FileExists(FilePath)) {
                    continue;
                }

                // Spawn the file.
                Lootable->FileContents->SpawnFile(FilePath, Filesystem);

                // Decrease the lootable count.
                LootableCount--;
            }
        }
    }
}

void UProceduralGenerationEngine::GenerateFirewallRules(FComputer& InComputer) {
    // Don't do this if the computer already has firewall rules!
    if(!InComputer.FirewallRules.Num()) {
        TArray<UComputerService*> Services = this->Peacenet->GetServicesFor(InComputer.ComputerType);

        // This gets the skill level of this computer's owning entity if any.
        int Skill = (this->Peacenet->IdentityExists(InComputer.SystemIdentity)) ? this->Peacenet->GetCharacterByID(InComputer.SystemIdentity).Skill : 0;

        for(int i = 0; i < Services.Num(); i++) {
            if((Services[i]->IsDefault || RNG.RandRange(0, 6) % 2) && this->HasAnyValidVersion(Services[i]))  {
                FFirewallRule Rule;
                Rule.Port = Services[i]->Port;
                Rule.Service = this->GetProtocol(Services[i], Skill);
                Rule.IsFiltered = false;
                InComputer.FirewallRules.Add(Rule);
            }
        }
    }
}

bool UProceduralGenerationEngine::HasAnyValidVersion(UComputerService* InComputerService) {
    for(auto Version : this->ProtocolVersions) {
        if(Version->Protocol == InComputerService) return true;
    }

    return false;
}

UProtocolVersion* UProceduralGenerationEngine::GetProtocol(UComputerService* InService, int InSkill) {
    int i = 0;
    int count = 100;
    UProtocolVersion* protocol = nullptr;

    while(count > 0) {
        UProtocolVersion* protocolVersion = this->ProtocolVersions[i];
        if(protocolVersion->Protocol != InService) {
            i++;
            if(i >= this->ProtocolVersions.Num()) {
                i=0;
            }
            continue;
        }

        protocol = protocolVersion;

        i++;
        if(i >= this->ProtocolVersions.Num()) {
            i=0;
        }
        count -= RNG.RandRange(0, 10);
    }

    return protocol;
}

TArray<FString> UProceduralGenerationEngine::GetMarkovData(EMarkovTrainingDataUsage InUsage) {
    TArray<FString> Ret;
    for(auto Markov : this->Peacenet->MarkovData) {
        if(Markov->Usage == InUsage) {
            Ret.Append(Markov->TrainingData);
        }
    }
    return Ret;
}

FString UProceduralGenerationEngine::GenerateIPAddress() {
    uint8 Byte1, Byte2, Byte3, Byte4 = 0;

    Byte1 = (uint8)RNG.RandRange(0, 255);

    // The other three are easy.
    Byte2 = (uint8)RNG.RandRange(0, 255);
    Byte3 = (uint8)RNG.RandRange(0, 255);
    Byte4 = (uint8)RNG.RandRange(0, 255);
    
    // We only support IPv4 in 2025 lol.
    return FString::FromInt(Byte1) + "." + FString::FromInt(Byte2) + "." + FString::FromInt(Byte3) + "." + FString::FromInt(Byte4);
}

void UProceduralGenerationEngine::ClearNonPlayerEntities() {
    this->Peacenet->ClearNonPlayerEntities();
}

void UProceduralGenerationEngine::Update(float InDeltaSeconds) {
    // stub
}

FString UProceduralGenerationEngine::ChooseEmailDomain() {
    TArray<FString> Emails = this->Peacenet->GetDomainNames();

    int Index = 0;

    int Counter = Emails.Num() * 10;
    while(Counter > 0) {
        FString Domain = Emails[Index];
        
        FComputer Computer;
        EConnectionError Error = EConnectionError::None;

        if(this->Peacenet->DnsResolve(Domain, Computer, Error)) {
            if(Computer.ComputerType == EComputerType::EmailServer) {
                Counter -= RNG.RandRange(5, 10);
                if(Counter <= 0) {
                    break;
                }
            }
        }

        Index++;
        if(Index >= Emails.Num()) {
            Index = 0;
        }
    }
    return Emails[Index];
}

void UProceduralGenerationEngine::GenerateNonPlayerCharacters() {
    this->ClearNonPlayerEntities();
    this->GenerateEmailServers();
    UE_LOG(LogTemp, Display, TEXT("Cleared old NPCs if any..."));

    for(int i = 0; i < 1000; i++) {
        this->GenerateNonPlayerCharacter();
    }
}

FPeacenetIdentity& UProceduralGenerationEngine::GenerateNonPlayerCharacter() {
    FPeacenetIdentity& Identity = this->Peacenet->GetNewIdentity();

    Identity.CharacterType = EIdentityType::NonPlayer;

    bool IsMale = RNG.RandRange(0, 6) % 2;

    FString CharacterName;
    do {
        if(IsMale) {
            CharacterName = MaleNameGenerator->GetMarkovString(0);
        } else {
            CharacterName = FemaleNameGenerator->GetMarkovString(0);
        }

        CharacterName = CharacterName + " " + LastNameGenerator->GetMarkovString(0);
    } while(this->Peacenet->CharacterNameExists(CharacterName));

    Identity.CharacterName = CharacterName;
    Identity.PreferredAlias = CharacterName;
    Identity.Skill = 0;

    float Reputation = RNG.GetFraction();
    bool IsBadRep = RNG.RandRange(0, 6) % 2;
    if(IsBadRep) {
        Reputation = -Reputation;
    }
    
    Identity.Reputation = Reputation;

    FString Username;
    FString Hostname;

    if(this->RNG.RandRange(0, 6) % 2) {
        UCommonUtils::ParseCharacterName(CharacterName, Username, Hostname);
    } else {
        Username = this->UsernameGenerator->GetMarkovString(0);
        Hostname = Username + "-pc";
        Identity.PreferredAlias = Username;
    }

    Identity.EmailAddress = Identity.PreferredAlias.Replace(TEXT(" "), TEXT("_")) + "@" + this->ChooseEmailDomain();

    FComputer& IdentityComputer = this->GenerateComputer(Hostname, EComputerType::Personal, EComputerOwnerType::NPC);

    FUser RootUser;
    FUser NonRootUser;
    
    RootUser.Username = "root";
    RootUser.ID = 0;
    RootUser.Domain = EUserDomain::Administrator;

    NonRootUser.ID = 1;
    NonRootUser.Username = Username;
    NonRootUser.Domain = EUserDomain::PowerUser;

    RootUser.Password = this->GeneratePassword(Identity.Skill*5);
    NonRootUser.Password = this->GeneratePassword(Identity.Skill*3);
    
    IdentityComputer.Users[0] = RootUser;
    IdentityComputer.Users[1] = NonRootUser;
    IdentityComputer.SystemIdentity = Identity.ID;

    Identity.ComputerID = IdentityComputer.ID;

    UE_LOG(LogTemp, Display, TEXT("Generated NPC: %s"), *Identity.CharacterName);

    return Identity;
}

FString UProceduralGenerationEngine::GeneratePassword(int InLength) {
    FString Chars = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890`~!@#$%^&*()_+-=[]{}\\|'\":;?.,<>";

    FString Ret;
    for(int i = 0; i < InLength; i++) {
        TCHAR Char = Chars[RNG.RandRange(0, Chars.Len()-1)];
        Ret.AppendChar(Char);
    }

    return Ret;
}

void UProceduralGenerationEngine::Initialize(APeacenetWorldStateActor* InPeacenet) {
    check(!this->Peacenet);
    check(InPeacenet);

    this->Peacenet = InPeacenet;

    // Load all the peacenet site assets in the game, checking for duplicate domain names,
    // and valid website widgets.
    TArray<UPeacenetSiteAsset*> TempSites;
    TArray<FString> Domains;

    // Load the assets into the game.
    this->Peacenet->LoadAssets<UPeacenetSiteAsset>("PeacenetSiteAsset", TempSites);

    // Loop through them to check validity.
    for(auto Site : TempSites) {
        // Skip invalid site widgets and empty domain names.
        if(!Site->PeacenetSite) {
            continue;
        }
        if(!Site->DomainName.Len()) {
            continue;
        }

        // Debug assert: two assets with the same domain name.
        check(!Domains.Contains(Site->DomainName));

        // Release sanity check: skip duplicate domain names.
        if(Domains.Contains(Site->DomainName)) {
            continue;
        }

        // Add the domain name to known list.
        Domains.Add(Site->DomainName);

        // Fully load the site.
        this->PeacenetSites.Add(Site);
    }


    // Recall when we set the world seed in the save file?
    // This is where we need it.
    this->RNG = FRandomStream(this->Peacenet->GetWorldSeed());

    // Now that we have an RNG, we can begin creating markov chains!
    this->MaleNameGenerator = NewObject<UMarkovChain>(this);
    this->FemaleNameGenerator = NewObject<UMarkovChain>(this);
    this->LastNameGenerator = NewObject<UMarkovChain>(this);
    this->DomainGenerator = NewObject<UMarkovChain>(this);

    // Create mixed markov input for usernames and hostnames.
    TArray<FString> UsernameData = this->GetMarkovData(EMarkovTrainingDataUsage::Usernames);
    UsernameData.Append(this->GetMarkovData(EMarkovTrainingDataUsage::Hostnames));

    // Set them all up with the data they need.
    this->MaleNameGenerator->Init(this->GetMarkovData(EMarkovTrainingDataUsage::MaleFirstNames), 3, RNG);
    this->FemaleNameGenerator->Init(this->GetMarkovData(EMarkovTrainingDataUsage::FemaleFirstNames), 3, RNG);
    this->LastNameGenerator->Init(this->GetMarkovData(EMarkovTrainingDataUsage::LastNames), 3, RNG);
    this->DomainGenerator->Init(UsernameData, 3, this->RNG);

    // get username generator.
    this->UsernameGenerator = NewObject<UMarkovChain>(this);
    
    // initialize it.
    this->UsernameGenerator->Init(UsernameData, 3, RNG);

    if(this->Peacenet->IsNewGame()) {
        // PASS 1: GENERATE NPC IDENTITIES.
        this->GenerateNonPlayerCharacters();

        // PASS 2: GENERATE STORY CHARACTERS
        this->UpdateStoryIdentities();

        // PASS 3: SPAWN PEACENET SITES
        this->SpawnPeacenetSites();
    }

    // Array of temporary file assets.
    TArray<ULootableFile*> TempFiles;

    // Populate it with ALL lootable files in the game.
    this->Peacenet->LoadAssets<ULootableFile>("LootableFile", TempFiles);

    // Filter out any files that have bad contents.
    for(auto File : TempFiles) {
        if(!File->FileContents) {
            continue;
        }

        // Add it to the REAL lootables list.
        this->LootableFiles.Add(File);
    }

    // Now we do that but for all the protocol versions in the game.
    TArray<UProtocolVersion*> Protocols;
    this->Peacenet->LoadAssets<UProtocolVersion>("ProtocolVersion", Protocols);

    for(auto Protocol : Protocols) {
        if(!Protocol->Protocol) {
            continue;
        }

        this->ProtocolVersions.Add(Protocol);
    }
}

void UProceduralGenerationEngine::SpawnPeacenetSites() {
    // Loop through all the Peacenet sites that we know of.
    for(auto Site : this->PeacenetSites) {
        // If the domain name, for some UNGODLY fucking reason, was taken
        // by a procgen'd system, we're going to give that system a new domain.
        //
        // This may result in NPCs ending up with invalid email addresses so we'll
        // fix those too.
        if(this->Peacenet->GetDomainNames().Contains(Site->DomainName)) {
            // This generates a new domain which is guaranteed not to be taken.
            FString NewDomain;
            do {
                NewDomain = this->DomainGenerator->GetMarkovString(0).ToLower() + ".com";
            } while(NewDomain == Site->DomainName || this->Peacenet->GetDomainNames().Contains(NewDomain));

            // Remove the old domain and add the new domain.
            this->Peacenet->ReplaceDomain(Site->DomainName, NewDomain);
            
            // Get the entity ID of the IP address so we can check the computer to see
            // if it's an email server.  If that's the case, then we need to reset
            // all NPCs in the game with the old domain as an email address.
            FComputer ComputerTemp;
            EConnectionError Error = EConnectionError::None;
            bool result = this->Peacenet->DnsResolve(NewDomain, ComputerTemp, Error);
            check(result);

            FComputer& Computer = this->Peacenet->GetComputerByID(ComputerTemp.ID);

            if(Computer.ComputerType == EComputerType::EmailServer) {
                // Go through all characters in the save file.
                for(auto& Character : this->Peacenet->GetCharacters()) {
                    // If the character's email address ends with the old domain we need to
                    // set it to the new domain.
                    if(Character.EmailAddress.EndsWith(Site->DomainName)) {
                        FString Alias = Character.CharacterName;
                        if(Character.PreferredAlias.Len()) {
                            Alias = Character.PreferredAlias;
                        }

                        Character.EmailAddress = Alias.ToLower().Replace(TEXT(" "), TEXT("_")) + "@" + NewDomain;
                    }
                }
            }
        }

        // That ordeal should be done now...
        // The next step is to generate a computer for the Peacenet site.
        FComputer& SiteComputer = this->GenerateComputer(Site->DomainName, EComputerType::PeacenetSite, EComputerOwnerType::None);

        // Assign the peacenet site asset to the computer.
        SiteComputer.PeacenetSite = Site;

        // Generate an IP address for the computer.
        FString IPAddress = this->Peacenet->GetIPAddress(SiteComputer);

        // Register the new domain.
        this->Peacenet->RegisterDomain(Site->DomainName, IPAddress);
    }
}

void UProceduralGenerationEngine::GenerateEmailServers() {
    const int MIN_EMAIL_SERVERS = 10;
    const int MAX_EMAIL_SERVERS = 25;

    int ServersToGenerate = this->RNG.RandRange(MIN_EMAIL_SERVERS, MAX_EMAIL_SERVERS);

    while(ServersToGenerate > 0) {
        FString NewHostname = this->DomainGenerator->GetMarkovString(0).ToLower() + ".com";
        while(this->Peacenet->GetDomainNames().Contains(NewHostname)) {
            NewHostname = this->DomainGenerator->GetMarkovString(0).ToLower() + ".com";
        }

        FComputer& Server = this->GenerateComputer(NewHostname, EComputerType::EmailServer, EComputerOwnerType::None);

        FString IPAddress = this->Peacenet->GetIPAddress(Server);

        check(IPAddress.Len());

        this->Peacenet->RegisterDomain(NewHostname, IPAddress);

        ServersToGenerate--;
    }
}

FComputer& UProceduralGenerationEngine::GenerateComputer(FString InHostname, EComputerType InComputerType, EComputerOwnerType InOwnerType) {
    FComputer& Ret = this->Peacenet->GetNewComputer();

    // Set up the core metadata.
    Ret.OwnerType = InOwnerType;
    Ret.ComputerType = InComputerType;

    // Get a random wallpaper.
    UWallpaperAsset* Wallpaper = this->Peacenet->Wallpapers[RNG.RandRange(0, this->Peacenet->Wallpapers.Num()-1)];
    Ret.UnlockedWallpapers.Add(Wallpaper->InternalID);
    Ret.CurrentWallpaper = Wallpaper->WallpaperTexture;

    // Create the barebones filesystem.
    FFolder Root;
    Root.FolderID = 0;
    Root.FolderName = "";
    Root.ParentID = -1;

    FFolder RootHome;
    RootHome.FolderID = 1;
    RootHome.FolderName = "root";
    RootHome.ParentID = 0;

    FFolder UserHome;
    UserHome.FolderID = 2;
    UserHome.FolderName = "home";
    UserHome.ParentID = 0;

    FFolder Etc;
    Etc.FolderID = 3;
    Etc.FolderName = "etc";
    Etc.ParentID = 0;

    // Write the hostname to a file.
    FFileRecord HostnameFile;
    HostnameFile.ID = 0;
    HostnameFile.Name = "hostname";
    HostnameFile.RecordType = EFileRecordType::Text;
    HostnameFile.ContentID = 0;

    FTextFile HostnameText;
    HostnameText.ID = 0;
    HostnameText.Content = InHostname;
    Ret.TextFiles.Add(HostnameText);
    Ret.FileRecords.Add(HostnameFile);

    // Write the file in /etc.
    Etc.FileRecords.Add(HostnameFile.ID);

    // Link up the three folders to the root.
    Root.SubFolders.Add(RootHome.FolderID);
    Root.SubFolders.Add(Etc.FolderID);
    Root.SubFolders.Add(UserHome.FolderID);
    
    // Add all the folders to the computer's disk.
    Ret.Filesystem.Add(Root);
    Ret.Filesystem.Add(Etc);
    Ret.Filesystem.Add(RootHome);
    Ret.Filesystem.Add(UserHome);
    
    // Create a root user for the system.
    FUser RootUser;
    RootUser.ID = 0;
    RootUser.Domain = EUserDomain::Administrator;
    RootUser.Username = "root";

    // Create a non-root user for the system.
    FUser NonRoot;
    NonRoot.ID = 1;
    NonRoot.Domain = EUserDomain::PowerUser;
    NonRoot.Username = "user";

    // Add the two users to the computer.
    Ret.Users.Add(RootUser);
    Ret.Users.Add(NonRoot);

    FString IPAddress = this->Peacenet->GetIPAddress(Ret);
    
    UE_LOG(LogTemp, Display, TEXT("Computer generated..."));

    // Return it.
    return Ret;
}