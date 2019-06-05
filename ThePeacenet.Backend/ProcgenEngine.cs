using Microsoft.Xna.Framework;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using ThePeacenet.Backend.AssetTypes;
using ThePeacenet.Backend.Data;
using ThePeacenet.Backend.FileSystem;
using ThePeacenet.Backend.OS;

namespace ThePeacenet.Backend
{
    public sealed class ProcgenEngine
    {
        private WorldState _world = null;
        private readonly List<Asset> _peacenetSites = null;
        private Random _rng = null;
        private List<ProtocolImplementation> _protocolVersions = null;
        private readonly List<Asset> _lootableFiles = null;
        private MarkovChain _maleNameGenerator = null;
        private MarkovChain _domainNameGenerator = null;
        private MarkovChain _usernameGenerator = null;
        private MarkovChain _femaleNameGenerator = null;
        private MarkovChain _lastNameGenerator = null;

        public Random RNG => _rng;

        public ProcgenEngine(WorldState world)
        {
            _world = world;
        }

        void UpdateStoryIdentities()
        {
            // Get all of the story character assets.
            var StoryCharacters = _world.Items.GetAll<StoryCharacter>();

            // Updates all the story characters in the game. If one
            // doesn't exist, it gets created.
            foreach (var Character in StoryCharacters)
            {
                UpdateStoryCharacter(Character);
            }
        }

        void UpdateStoryCharacter(StoryCharacter InStoryCharacter)
        {
            // The entity ID and identity of the current (or new)
            // character.
            Identity Identity = null;

            // Index of the identity in the save file.
            int IdentityIndex = -1;

            // Does the save file already know about this character?
            if (!_world.GetStoryCharacterID(InStoryCharacter, out int EntityID))
            {
                // We don't have an existing identity so we'll generate
                // a new one real quick and add it to the save file.
                Identity = new Identity
                {

                    // Assign the entity ID to the identity, and make the identity a story character.
                    IdentityType = IdentityType.Story
                };

                // Retrieve the index of the entity in the save file by getting the
                // length of the characters array before adding the entity.
                IdentityIndex = _world.Identities.Count();
                _world.AddIdentity(Identity);
            }

            // Update the character's name.
            Identity.Name = InStoryCharacter.Name;

            // Preferred alias of the identity is determined by whether we want to use the name
            // as the email address.
            if (InStoryCharacter.UseNameForEmail)
            {
                // Use the character's name as the alias.
                Identity.Alias = Identity.Name;
            }
            else
            {
                // Use the email alias value as the preferred alias.
                Identity.Alias = InStoryCharacter.EmailAlias;
            }

            // So that the identity's email doesn't keep changing providers
            // every time the save file is loaded, we're going to check if it starts
            // with the preferred alias before we choose a new email.  That way we
            // are forced to preemptively generate the username part of the email.
            string EmailUser = Identity.Alias.ToLower().Replace(" ", "_");

            if (!Identity.Email.StartsWith(EmailUser))
            {
                // NOW we can re-assign the email address.
                Identity.Email = EmailUser + "@" + this.ChooseEmailDomain();
            }

            // If the identity doesn't have a computer ID, we need to generate one.
            if (Identity.Computers.Count == 0)
            {
                // Create the actual computer.
                Computer StoryComputer = this.GenerateComputer("tempsys", ComputerType.Personal, IdentityType.Story);

                // Assign the computer ID to the identity.
                Identity.Computers.Add(StoryComputer.Id);
            }

            // Reassign the character's entity ID.
            _world.AssignStoryCharacterID(InStoryCharacter, EntityID);

            // Now we can update the story computer.
            this.UpdateStoryComputer(InStoryCharacter);
        }

        void UpdateStoryComputer(StoryCharacter InStoryCharacter)
        {
            // Get the identity of the story character so we can get its computer ID.
            bool StoryEntityIDResult = _world.GetStoryCharacterID(InStoryCharacter, out int StoryEntityID);

            // Get the identity using that ID.
            Identity Identity = _world.GetIdentity(StoryEntityID);

            // Update the non-root user of the computer so that its username is the same
            // as what's defined in the story character.
            foreach (var ComputerID in Identity.Computers)
            {
                var Computer = _world.GetComputer(ComputerID);
                if (Computer != null)
                {
                    foreach (User User in Computer.Users)
                    {
                        if (User.Id == 1)
                        {
                            if (InStoryCharacter.UseEmailAsUsername)
                            {
                                if (InStoryCharacter.UseNameForEmail)
                                {
                                    User.Username = InStoryCharacter.Name.ToLower().Replace(" ", "_");
                                }
                                else
                                {
                                    User.Username = InStoryCharacter.EmailAlias.ToLower().Replace(" ", "_");
                                }
                            }
                            else
                            {
                                User.Username = InStoryCharacter.Username.ToLower().Replace(" ", "_");
                            }
                            break;
                        }
                    }
                    break;
                }
            }

            // We have a computer ID, so we're going to create a TEMPORARY System Context to make
            // updating easier.
            var SystemContext = _world.GetKernel(Identity.Id);

            // Get a filesystem with root privileges from the system context.
            var RootFS = SystemContext.GetUserLand(0).FileSystem;

            // Update the hostname.
            string Host = InStoryCharacter.Hostname;
            if (InStoryCharacter.UseNameForHostname)
            {
                Host = InStoryCharacter.Name.ToLower().Replace(" ", "_") + "-pc";
            }
            RootFS.WriteText("/etc/hostname", Host);

            // Now we'll start looking at exploits.
            foreach (var Exploit in InStoryCharacter.Exploits)
            {
                // Do we already have a file on the system that refers to this exploit?
                bool ExploitExists = false;

                // Go through all file records within the system.
                foreach (var Record in SystemContext.FileRecords)
                {
                    // Only check exploit records.
                    if (Record.RecordType == FileRecordType.Exploit)
                    {
                        // Check the ID first.
                        if (Record.ContentId >= 0 && Record.ContentId < _world.Items.GetAll<Exploit>().Count())
                        {
                            // Get the file exploit data.
                            var FileExploit = _world.Items.GetAll<Exploit>().ToArray()[Record.ContentId];

                            // If the exploit IDs match, then this exploit DOES NOT need to spawn.
                            if (FileExploit.Id == Exploit.Id)
                            {
                                ExploitExists = true;
                                break;
                            }
                        }
                    }
                }

                // DO NOT SPAWN THE EXPLOIT if the exploit already exists.
                if (ExploitExists) continue;

                // TODO: random exploit spawn folders.
                for (int i = 0; i < _world.Items.GetAll<Exploit>().Count(); i++)
                {
                    if (_world.Items.GetAll<Exploit>().ToArray()[i].Id == Exploit.Id)
                    {
                        RootFS.SetFileRecord("/root/" + Exploit.Id.ToString() + ".gsf", FileRecordType.Exploit, i);
                    }
                }
            }
        }

        void PlaceLootableFiles(IUserLand InUserContext)
        {
            // If the system is a player, then we stop right now.
            if (_world.Identities.Any(x => x.Name == InUserContext.IdentityName && x.IdentityType == IdentityType.Player)) return;

            // The amount of lootables we are EVER allowed to generate in an NPC.
            int MaxLootables = this._lootableFiles.Count / 2;

            // If that ends up being 0, then we stop right there.
            if (MaxLootables < 0) return;

            // The amount of lootables this NPC gets.
            int LootableCount = RNG.Next(0, MaxLootables);

            // Get a filesystem context for this user.
            var Filesystem = InUserContext.FileSystem;

            // Keep going while there are lootables to generate.
            while (LootableCount > 0)
            {
                // Pick a random lootable! Any lootable!
                dynamic Lootable = this._lootableFiles[RNG.Next(0, this._lootableFiles.Count - 1)];

                // This is where the file will go.
                string BaseDirectory = "/";

                // Desktop or documents?
                int Dice = RNG.Next(0, 6);

                // Where should it go?
                switch (Lootable.SpawnLocation)
                {
                    case FileSpawnLocation.Anywhere:
                        // TODO: Support picking of random folders.
                        break;
                    case FileSpawnLocation.HomeFolder:
                        BaseDirectory = InUserContext.HomeFolder;
                        break;
                    case FileSpawnLocation.Binaries:
                        BaseDirectory = "/bin";
                        break;
                    case FileSpawnLocation.DesktopOrDocuments:
                        if (Dice % 2 == 0)
                            BaseDirectory = InUserContext.HomeFolder + "/Desktop";
                        else
                            BaseDirectory = InUserContext.HomeFolder + "/Documents";
                        break;
                    case FileSpawnLocation.Pictures:
                        BaseDirectory = InUserContext.HomeFolder + "/Pictures";
                        break;
                    case FileSpawnLocation.Downloads:
                        BaseDirectory = InUserContext.HomeFolder + "/Downloads";
                        break;
                    case FileSpawnLocation.Music:
                        BaseDirectory = InUserContext.HomeFolder + "/Music";
                        break;
                    case FileSpawnLocation.Videos:
                        BaseDirectory = InUserContext.HomeFolder + "/Videos";
                        break;
                }

                // Now that we have a base directory, get the file path.
                string FilePath = BaseDirectory + "/" + Lootable.FileName;

                // If the file already exists, continue.
                if (Filesystem.FileExists(FilePath)) continue;

                // Spawn the file.
                Lootable.FileContents.SpawnFile(FilePath, Filesystem);

                // Decrease the lootable count.
                LootableCount--;
            }
        }

        void GenerateFirewallRules(Computer InComputer)
        {
            // Don't do this if the computer already has firewall rules!
            if (InComputer.Services.Count > 0)
                return;

            var Services = _world.Items.GetAll<Protocol>().Where(x => x.TargetComputerType == InComputer.ComputerType).ToArray();

            // This gets the skill level of this computer's owning entity if any.
            int Skill = _world.GetSkillOf(InComputer);

            for (int i = 0; i < Services.Length; i++)
            {
                var Service = Services[i];
                if (Service.IsDefault || RNG.Next(0, 6) % 2 == 0)
                {
                    FirewallRule Rule = new FirewallRule
                    {
                        Port = Services[i].Port,
                        Service = this.GetProtocol(Services[i], Skill).Id,
                        IsFiltered = false
                    };
                    InComputer.Services.Add(Rule);
                }
            }
        }

        ProtocolImplementation GetProtocol(Protocol InService, int InSkill)
        {
            int i = 0;
            int count = 100;
            ProtocolImplementation protocol = null;

            while (count > 0)
            {
                ProtocolImplementation protocolVersion = this._protocolVersions[i];
                if (protocolVersion.Protocol != InService)
                {
                    i++;
                    if (i >= this._protocolVersions.Count)
                        i = 0;
                    continue;
                }
                if (protocolVersion.MinimumSkillLevel > InSkill)
                {
                    i++;
                    if (i >= this._protocolVersions.Count)
                        i = 0;
                    continue;
                }

                protocol = protocolVersion;

                i++;
                if (i >= this._protocolVersions.Count)
                    i = 0;

                count -= RNG.Next(0, 10);
            }

            return protocol;
        }

        string[] GetMarkovData(MarkovTrainingDataUsage InUsage)
        {
            List<string> Ret = new List<string>();
            foreach (var Markov in this._world.Items.GetAll<MarkovTrainingDataAsset>())
            {
                if (Markov.Usage == InUsage)
                {
                    Ret.AddRange(Markov.TrainingData);
                }
            }
            return Ret.ToArray();
        }

        string GenerateIPAddress()
        {
            byte Byte1, Byte2, Byte3, Byte4 = 0;

            Byte1 = (byte)RNG.Next(0, 255);

            // The other three are easy.
            Byte2 = (byte)RNG.Next(0, 255);
            Byte3 = (byte)RNG.Next(0, 255);
            Byte4 = (byte)RNG.Next(0, 255);

            // We only support IPv4 in 2025 lol.
            return $"{Byte1}.{Byte2}.{Byte3}.{Byte4}";
        }

        void GenerateIdentityPosition(Identity Pivot, Identity Identity)
        {
            const float MIN_DIST_FROM_PIVOT = 50;
            const float MAX_DIST_FROM_PIVOT = 400;


            Vector2 Test = Vector2.Zero;
            if (_world.GetPosition(Identity, out Test))
                return;

            bool PivotResult = _world.GetPosition(Pivot, out Vector2 PivotPos);

            if (!PivotResult)
            {
                foreach (var EntityID in _world.Identities.Select(x => x.Id).Distinct())
                {
                    var entity = _world.GetIdentity(EntityID);
                    if (_world.GetPosition(entity, out Vector2 PivotSquared))
                    {
                        PivotResult = true;

                        PivotPos = Vector2.Zero;
                        do
                        {
                            PivotPos.X = PivotSquared.X + (RNG.NextFloat(MIN_DIST_FROM_PIVOT, MAX_DIST_FROM_PIVOT) - (MAX_DIST_FROM_PIVOT / 2));
                            PivotPos.Y = PivotSquared.Y + (RNG.NextFloat(MIN_DIST_FROM_PIVOT, MAX_DIST_FROM_PIVOT) - (MAX_DIST_FROM_PIVOT / 2));
                        } while (_world.LocationTooCloseToEntity(PivotPos, MIN_DIST_FROM_PIVOT));

                        _world.SetEntityPosition(Pivot, PivotPos);

                        break;
                    }
                }
            }

            Vector2 NewPos = Vector2.Zero;
            do
            {
                NewPos.X = PivotPos.X + (RNG.NextFloat(MIN_DIST_FROM_PIVOT, MAX_DIST_FROM_PIVOT) - (MAX_DIST_FROM_PIVOT / 2));
                NewPos.Y = PivotPos.Y + (RNG.NextFloat(MIN_DIST_FROM_PIVOT, MAX_DIST_FROM_PIVOT) - (MAX_DIST_FROM_PIVOT / 2));
            } while (_world.LocationTooCloseToEntity(NewPos, MIN_DIST_FROM_PIVOT));

            this._world.SetEntityPosition(Identity, NewPos);
        }

        void GenerateAdjacentNodes(Identity InIdentity)
        {
            // Don't generate any new links if there are any existing links from this NPC.
            // PATCH: Before, this would check for any links to and from the NPC, that's a problem. Now we only check for links from the NPC.
            if (_world.AdjacentNodes.Any(x => InIdentity.Id == x.NodeA))
                return;

            const int MIN_ADJACENTS = 2;
            const int MAX_ADJACENTS = 8;
            const int MAX_SKILL_DIFFERENCE = 3;


            int Adjacents = RNG.Next(MIN_ADJACENTS, MAX_ADJACENTS);

            while (Adjacents > 0)
            {
                Identity LinkedIdentity = _world.Identities.ToArray()[RNG.Next(0, _world.Identities.Count() - 1)];

                if (LinkedIdentity.Id == InIdentity.Id)
                    continue;

                if (_world.AdjacentNodes.Any(x => (x.NodeA == LinkedIdentity.Id && x.NodeB == InIdentity.Id) || (x.NodeB == LinkedIdentity.Id && x.NodeA == InIdentity.Id)))
                    continue;

                int Difference = Math.Abs(InIdentity.Skill - LinkedIdentity.Skill);
                if (Difference > MAX_SKILL_DIFFERENCE)
                    continue;

                _world.MakeAdjacent(InIdentity, LinkedIdentity);

                this.GenerateIdentityPosition(InIdentity, LinkedIdentity);

                Adjacents--;
            }
        }

        string ChooseEmailDomain()
        {
            var Emails = _world.DomainNames.ToArray();

            int Index = 0;

            int Counter = Emails.Length * 10;
            while (Counter > 0)
            {
                string Domain = Emails[Index];

                Computer PC = _world.DnsResolve(Domain);

                if (PC != null)
                {
                    if (PC.ComputerType == ComputerType.EmailServer)
                    {
                        Counter -= RNG.Next(5, 10);
                        if (Counter <= 0)
                            break;
                    }
                }

                Index++;
                if (Index >= Emails.Length)
                {
                    Index = 0;
                }
            }
            return Emails[Index];
        }

        void GenerateNonPlayerCharacters()
        {
            this.GenerateEmailServers();

            for (int i = 0; i < 1000; i++)
            {
                this.GenerateNonPlayerCharacter();
            }
        }

        void ParseCharacterName(string InCharacterName, out string OutUsername, out string OutHostname)
        {
            OutUsername = "";
            OutHostname = "";

            // No sense doing this if there's only whitespace
            if (string.IsNullOrWhiteSpace(InCharacterName))
                return;

            // Unix usernames can only be lower-case.
            string NameString = InCharacterName.ToLower();

            // this will be the username.
            string FirstName = "";
            string Rem = "";

            // These characters are valid as name chars.
            const string ValidUnixUsernameChars = "abcdefghijklmnopqrstuvwxyz0123456789_-";

            // the first char that isn't valid.
            char InvalidChar = '\0';

            // the chars in the name string
            var NameChars = NameString.ToCharArray();

            foreach (char Char in NameChars)
            {
                if (!ValidUnixUsernameChars.Contains(Char))
                {
                    InvalidChar = Char;
                    break;
                }
            }

            // Did that for loop above change us?
            if (InvalidChar != '\0')
            {
                FirstName = NameString.Substring(0, NameString.IndexOf(InvalidChar));
                Rem = NameString.Substring(NameString.IndexOf(InvalidChar) + 1);
            }
            else
            {
                FirstName = NameString;
            }

            OutUsername = FirstName;
            OutHostname = FirstName + "-pc";
        }


        Identity GenerateNonPlayerCharacter()
        {
            Identity Identity = new Identity
            {
                Id = _world.Identities.Select(x => x.Id).Distinct().Max() + 1,
                IdentityType = IdentityType.NPC
            };

            bool IsMale = RNG.Next(0, 6) % 2 == 0;

            string CharacterName = "";
            do
            {
                if (IsMale)
                {
                    CharacterName = _maleNameGenerator.GetMarkovString(0);
                }
                else
                {
                    CharacterName = _femaleNameGenerator.GetMarkovString(0);
                }

                CharacterName += " " + _lastNameGenerator.GetMarkovString(0);
            } while (_world.Identities.Any(x => string.Equals(x.Name, CharacterName, StringComparison.OrdinalIgnoreCase)));

            Identity.Name = CharacterName;
            Identity.Alias = CharacterName;
            Identity.Skill = RNG.Next(1, 15);

            float Reputation = (float)RNG.NextDouble();
            bool IsBadRep = RNG.Next(0, 6) % 2 == 0;
            if (IsBadRep)
                Reputation = -Reputation;

            Identity.Reputation = Reputation;

            string Username = "";
            string Hostname = "";

            if (RNG.Next(0, 6) % 2 == 0)
            {
                this.ParseCharacterName(CharacterName, out Username, out Hostname);
            }
            else
            {
                Username = _usernameGenerator.GetMarkovString(0);
                Hostname = Username + "-pc";
                Identity.Alias = Username;
            }

            Identity.Email = Identity.Alias.Replace(" ", "_") + "@" + this.ChooseEmailDomain();

            Computer IdentityComputer = this.GenerateComputer(Hostname, ComputerType.Personal, IdentityType.NPC);

            User RootUser = new User();
            User NonRootUser = new User();

            RootUser.Username = "root";
            RootUser.Id = 0;

            NonRootUser.Id = 1;
            NonRootUser.Username = Username;

            RootUser.Password = this.GeneratePassword(Identity.Skill * 5);
            NonRootUser.Password = this.GeneratePassword(Identity.Skill * 3);

            IdentityComputer.Users[0] = RootUser;
            IdentityComputer.Users[1] = NonRootUser;

            Identity.Computers.Add(IdentityComputer.Id);

            _world.AddIdentity(Identity);

            return Identity;
        }

        string GeneratePassword(int InLength)
        {
            string Chars = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890`~!@#$%^&*()_+-=[]{}\\|'\":;?.,<>";

            string Ret = "";
            for (int i = 0; i < InLength; i++)
            {
                char Char = Chars[RNG.Next(0, Chars.Length - 1)];
                Ret += Char;
            }

            return Ret;
        }

        void Initialize()
        {
            List<string> Domains = new List<string>();

            // Initialize the world seed if the game is new.
            if (_world.IsNewGame)
            {
                // Get the player character.
                Identity Character = _world.Identities.First(x => x.IdentityType == IdentityType.Player);

                // This creates a hash out of the character name which we can seed the RNG with.
                // Thus making the player's name dictate how the world generates.
                int Hash = Character.Name.GetHashCode();

                // Store the seed in the save file in case we need it. WHICH WE FUCKING WILL LET ME TELL YOU.
                _world.Seed = Hash;
            }

            // Recall when we set the world seed in the save file?
            // This is where we need it.
            _rng = new Random(_world.Seed);

            // Create mixed markov input for usernames and hostnames.
            var UsernameData = this.GetMarkovData(MarkovTrainingDataUsage.Usernames).ToList();
            UsernameData.AddRange(this.GetMarkovData(MarkovTrainingDataUsage.Hostnames));

            // Now that we have an RNG, we can begin creating markov chains!
            this._maleNameGenerator = new MarkovChain(this.GetMarkovData(MarkovTrainingDataUsage.MaleFirstNames), 3, RNG);
            this._femaleNameGenerator = new MarkovChain(this.GetMarkovData(MarkovTrainingDataUsage.FemaleFirstNames), 3, RNG);
            this._lastNameGenerator = new MarkovChain(this.GetMarkovData(MarkovTrainingDataUsage.LastNames), 3, RNG);
            this._domainNameGenerator = new MarkovChain(UsernameData.ToArray(), 3, RNG);

            // get username generator.
            this._usernameGenerator = new MarkovChain(UsernameData.ToArray(), 3, RNG);


            if (_world.IsNewGame)
            {
                // PASS 1: GENERATE NPC IDENTITIES.
                this.GenerateNonPlayerCharacters();

                // PASS 2: GENERATE STORY CHARACTERS
                this.UpdateStoryIdentities();

                // PASS 4: GENERATE CHARACTER RELATIONSHIPS
                this.GenerateCharacterRelationships();
            }

            _protocolVersions = _world.Items.GetAll<ProtocolImplementation>().ToList();
        }

        void GenerateEmailServers()
        {
            const int MIN_EMAIL_SERVERS = 10;
            const int MAX_EMAIL_SERVERS = 25;

            int ServersToGenerate = this.RNG.Next(MIN_EMAIL_SERVERS, MAX_EMAIL_SERVERS);

            while (ServersToGenerate > 0)
            {
                string NewHostname = this._domainNameGenerator.GetMarkovString(0).ToLower() + ".com";
                while (_world.DomainNames.Contains(NewHostname))
                {
                    NewHostname = this._domainNameGenerator.GetMarkovString(0).ToLower() + ".com";
                }

                Computer Server = this.GenerateComputer(NewHostname, ComputerType.EmailServer, IdentityType.None);

                string IPAddress = _world.GetIPAddress(Server);

                _world.DnsRegister(NewHostname, IPAddress);

                ServersToGenerate--;
            }
        }

        void GenerateCharacterRelationships()
        {
            // We will need to remove all relationships that are between any character and a non-player.
            List<CharacterRelationship> RelationshipsToRemove = new List<CharacterRelationship>();
            foreach (var Relationship in _world.Relationships)
            {
                var First = _world.GetIdentity(Relationship.FirstId);
                var Second = _world.GetIdentity(Relationship.SecondId);


                if (First.IdentityType != IdentityType.Player || Second.IdentityType != IdentityType.Player)
                {
                    RelationshipsToRemove.Add(Relationship);
                }
            }

            int RelationshipsRemoved = 0;
            while (RelationshipsToRemove.Count > 0)
            {
                _world.RemoveRelationship(RelationshipsToRemove.First());
                RelationshipsRemoved++;
                RelationshipsToRemove.RemoveAt(0);
            }

            List<Identity> GoodReps = _world.Identities.Where(x => x.IdentityType != IdentityType.Player).Where(x => x.Reputation >= 0).ToList();
            List<Identity> BadReps = _world.Identities.Where(x => x.IdentityType != IdentityType.Player).Where(x => x.Reputation < 0).ToList();

            // Second pass goes through every NPC, looks at their reputation, and chooses relationships from the correct list.
            foreach (var First in _world.Identities.Where(x => x.IdentityType != IdentityType.Player))
            {
                bool Bad = First.Reputation < 0;

                bool MakeEnemy = RNG.Next(0, 6) % 2 == 0;

                Identity Second = null;

                do
                {
                    if (MakeEnemy)
                    {
                        if (Bad)
                            Second = GoodReps[RNG.Next(0, GoodReps.Count - 1)];
                        else
                            Second = BadReps[RNG.Next(0, BadReps.Count - 1)];
                    }
                    else
                    {
                        if (Bad)
                            Second = BadReps[RNG.Next(0, BadReps.Count - 1)];
                        else
                            Second = GoodReps[RNG.Next(0, GoodReps.Count - 1)];
                    }
                } while (_world.Relationships.Any(x => x.FirstId == First.Id && x.SecondId == Second.Id));

                CharacterRelationship Relationship = new CharacterRelationship
                {
                    FirstId = First.Id,
                    SecondId = Second.Id
                };

                if (MakeEnemy)
                {
                    Relationship.RelationshipType = RelationshipType.Enemy;
                }
                else
                {
                    Relationship.RelationshipType = RelationshipType.Friend;
                }

                _world.AddRelationship(Relationship);
            }
        }


        Computer GenerateComputer(string InHostname, ComputerType InComputerType, IdentityType InOwnerType)
        {
            Computer Ret = new Computer
            {

                // Set up the core metadata.
                Id = _world.Computers.Select(x => x.Id).Distinct().Max() + 1,

                OwnerType = InOwnerType,

                ComputerType = InComputerType
            };

            // Create the barebones filesystem.
            Folder Root = new Folder
            {
                Id = 0,
                Name = "",
                Parent = -1
            };

            Folder RootHome = new Folder
            {
                Id = 1,
                Name = "root",
                Parent = 0
            };

            Folder UserHome = new Folder
            {
                Id = 2,
                Name = "home",
                Parent = 0
            };

            Folder Etc = new Folder
            {
                Id = 3,
                Name = "etc",
                Parent = 0
            };

            // Write the hostname to a file.
            FileRecord HostnameFile = new FileRecord
            {
                Id = 0,
                Name = "hostname",
                RecordType = FileRecordType.Text,
                ContentId = 0
            };

            TextFile HostnameText = new TextFile
            {
                Id = 0,
                Content = InHostname
            };
            Ret.TextFiles.Add(HostnameText);
            Ret.Files.Add(HostnameFile);

            // Write the file in /etc.
            Etc.Files.Add(HostnameFile.Id);

            // Link up the three folders to the root.
            Root.SubFolders.Add(RootHome.Id);
            Root.SubFolders.Add(Etc.Id);
            Root.SubFolders.Add(UserHome.Id);

            // Add all the folders to the computer's disk.
            Ret.Folders.Add(Root);
            Ret.Folders.Add(Etc);
            Ret.Folders.Add(RootHome);
            Ret.Folders.Add(UserHome);

            // Create a root user for the system.
            User RootUser = new User
            {
                Id = 0,
                Username = "root"
            };

            // Create a non-root user for the system.
            User NonRoot = new User
            {
                Id = 1,
                Username = "user"
            };

            // Add the two users to the computer.
            Ret.Users.Add(RootUser);
            Ret.Users.Add(NonRoot);

            // Add the computer to the save file.
            _world.AddComputer(Ret);

            string IPAddress = "";
            do
            {
                IPAddress = this.GenerateIPAddress();
            } while (_world.DnsResolve(IPAddress) != null);

            _world.AssignIP(Ret, IPAddress);

            // Return it.
            return Ret;
        }
    }

    public struct MarkovSource
    {
        private string Data;
        private char[] Chars;

        public void SetCount(int InCount)
        {
            Chars = new char[InCount];
        }

        public void Rotate(char nextChar)
        {
#if DEBUG
            if (Chars.Length < 1) throw new InvalidOperationException("debug assert: chars > 0 == false.");
#endif

            for (int i = 0; i < Chars.Length - 1; i++)
            {
                Chars[i] = Chars[i + 1];
            }
            Chars[Chars.Length - 1] = nextChar;
            Data = ToString();
        }

        public bool IsLessThan(MarkovSource OtherSource)
        {
            int i = 0;
            for (i = 0; i < Chars.Length - 1; i++)
            {
                if (Chars[i] != OtherSource.Chars[i]) break;
            }
            return Chars[i] < OtherSource.Chars[i];
        }

        public bool IsStartSource()
        {
            foreach (var Char in Chars)
            {
                if (Char != '\0') return false;
            }
            return true;
        }

        public static bool operator ==(MarkovSource a, MarkovSource b)
        {
            return a.Chars == b.Chars;
        }

        public static bool operator !=(MarkovSource a, MarkovSource b)
        {
            return a.Chars != b.Chars;
        }

        public override string ToString()
        {
            string Ret = "";

            for (int i = 0; i < this.Chars.Length; i++)
            {
                Ret += Chars[i];
            }

            return Ret;
        }

        public char[] GetChars()
        {
            return this.Chars;
        }
    }

    public class MarkovChain
    {
        Dictionary<MarkovSource, Dictionary<char, int>> MarkovMap;
        Random Random;
        readonly int SourceCount = 0;

        char GetNext(MarkovSource InSource)
        {
            if (!MarkovMap.ContainsKey(InSource))
                return '\0';

            var Map = this.MarkovMap[InSource];

            int Total = 0;

            char[] Keys = Map.Keys.ToArray();

            for (int i = 0; i < Keys.Length; i++)
            {
                Total += Map[Keys[i]];
            }

            int Rng = this.Random.Next(0, Total);

            for (int i = 0; i < Keys.Length; i++)
            {
                Rng -= Map[Keys[i]];
                if (Rng < 0)
                {
                    return Keys[i];
                }
            }

            return '\0';
        }

        MarkovSource RandomSource()
        {
            MarkovSource[] Keys = this.MarkovMap.Keys.ToArray();

            return Keys[Random.Next(0, Keys.Length)];
        }

        bool IsDeadEnd(MarkovSource InSource, int Depth)
        {
            if (Depth <= 0)
                return false;

            if (!MarkovMap.ContainsKey(InSource))
                return true;

            var Map = this.MarkovMap[InSource];

            char[] Keys = Map.Keys.ToArray();

            if (Keys.Length == 1)
                if (Keys[0] == '\0')
                    return true;

            MarkovSource TempSource = InSource;

            for (int i = 0; i < Keys.Length; ++i)
            {
                TempSource = InSource;
                TempSource.Rotate(Keys[i]);
                if (!IsDeadEnd(TempSource, Depth - 1)) return false;
            }

            return true;
        }

        char GetNextCharGuarantee(MarkovSource InSource, int InSteps)
        {
#if DEBUG
            if(IsDeadEnd(InSource, InSteps)) throw new InvalidOperationException("Debug assert.");
#endif

            Dictionary<char, int> Temp = new Dictionary<char, int>();
            var Map = MarkovMap[InSource];

            var Keys = Map.Keys.ToArray();

            if (Keys.Length == 1)
                return Keys[0];

#if DEBUG
            if(KeyCount < 1) throw new InvalidOperationException("Debug assert.");
#endif

            int Total = 0;
            for (int i = 0; i < Keys.Length; ++i)
            {
                MarkovSource TempSource = InSource;
                TempSource.Rotate(Keys[i]);
                if (!IsDeadEnd(TempSource, InSteps))
                {
                    if (Temp.ContainsKey(Keys[i]))
                        Temp[Keys[i]] = Map[Keys[i]];
                    else
                        Temp.Add(Keys[i], Map[Keys[i]]);
                    Total += Map[Keys[i]];
                }
            }

            int Rng = Random.Next(Total);

            char[] TempKeys = Temp.Keys.ToArray();

            for (int i = 0; i < TempKeys.Length; i++)
            {
                Rng -= Temp[TempKeys[i]];
                if (Rng < 0)
                {
                    return TempKeys[i];
                }
            }

#if DEBUG
            if(Rng >= 0) throw new InvalidOperationException("Debug assert.");
#endif

            return '\0';
        }

        public MarkovChain(string[] InArray, int N, Random InRng)
        {
            this.Random = InRng;

            foreach (string ArrayString in InArray)
            {
                MarkovSource Source = new MarkovSource();
                Source.SetCount(N);

                foreach (char Char in ArrayString)
                {
                    if (Char == '\0')
                        break;

                    if (!MarkovMap.ContainsKey(Source))
                        MarkovMap.Add(Source, new Dictionary<char, int>());

                    if (!MarkovMap[Source].ContainsKey(Char))
                        MarkovMap[Source].Add(Char, 0);

                    MarkovMap[Source][Char]++;
                    Source.Rotate(Char);
                }
                if (!MarkovMap.ContainsKey(Source))
                    MarkovMap.Add(Source, new Dictionary<char, int>());

                if (!MarkovMap[Source].ContainsKey('\0'))
                    MarkovMap[Source].Add('\0', 0);

                MarkovMap[Source]['\0']++;
            }

            SourceCount = N;
        }

        public string GetMarkovString(int InLength)
        {
            string Out = "";

            MarkovSource src = new MarkovSource();
            src.SetCount(SourceCount);

            if (InLength < 1)
            {
                char tmp = GetNext(src);
                while (tmp != '\0')
                {
                    Out += tmp;
                    src.Rotate(tmp);
                    tmp = GetNext(src);
                }
                return Out;
            }
            for (int i = 0; i < InLength; ++i)
            {
                int x = (i > InLength - SourceCount ? InLength - i : SourceCount);
                char tmp = GetNextCharGuarantee(src, x);
                Out += tmp;
                src.Rotate(tmp);
            }
            return Out;
        }
    }

public static class RandomExtensions
{
    public static float NextFloat(this Random rng, float min, float max)
    {
        var range = (max - min);
        var value = (float)rng.NextDouble() * range;
        return value + min;
    }
}
}
