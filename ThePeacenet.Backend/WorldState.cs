using Microsoft.Xna.Framework;
using Microsoft.Xna.Framework.Content;
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using ThePeacenet.Backend.AssetTypes;
using ThePeacenet.Backend.Data;
using ThePeacenet.Backend.OS;
using ThePeacenet.Backend.Shell;

namespace ThePeacenet.Backend
{
    public class WorldState : ITickable
    {
        private readonly IProgramGuiBuilder _guiBuilder = null;
        private SaveGame _saveGame = null;
        private List<CommandAsset> _commandAssets = new List<CommandAsset>();
        private List<PlayerKernel> _kernels = new List<PlayerKernel>();
        private ItemContainer _itemContainer = null;
        private readonly List<Exploit> _exploits = new List<Exploit>();
        private Task _itemLoadTask = null;
        private bool _hasWorldBeenStarted = false;

        public IProgramGuiBuilder GuiBuilder => _guiBuilder;
        public IEnumerable<AdjacentNode> AdjacentNodes => _saveGame.AdjacentNodes;

        public WorldState(IProgramGuiBuilder guiBuilder)
        {
            _guiBuilder = guiBuilder;
        }

        public IEnumerable<Computer> Computers => _saveGame.Computers;
        public IEnumerable<CharacterRelationship> Relationships => _saveGame.CharacterRelationships;
        public IEnumerable<string> DomainNames => _saveGame.DomainNameMap.Keys;
        public ItemContainer Items => _itemContainer;
        public IEnumerable<Identity> Identities => _saveGame.Characters;
        public bool IsNewGame => _saveGame.IsNewGame;
        public int Seed { get => _saveGame.WorldSeed; internal set => _saveGame.WorldSeed = value; }

        internal void AssignIP(Computer computer, string ip)
        {
            _saveGame.ComputerIPMap.Add(ip, computer.Id);
        }

        internal void AddComputer(Computer computer)
        {
            _saveGame.Computers.Add(computer);
        }

        internal void RemoveRelationship(CharacterRelationship relationship)
        {
            _saveGame.CharacterRelationships.Remove(relationship);
        }

        public Computer DnsResolve(string hostname)
        {
            if(_saveGame.DomainNameMap.ContainsKey(hostname))
            {
                string ip = _saveGame.DomainNameMap[hostname];
                return DnsResolve(ip);
            }

            if(_saveGame.ComputerIPMap.ContainsKey(hostname))
            {
                return this.GetComputer(_saveGame.ComputerIPMap[hostname]);
            }

            return null;
        }

        internal void AddRelationship(CharacterRelationship relationship)
        {
            _saveGame.CharacterRelationships.Add(relationship);
        }

        public string GetIPAddress(Computer computer)
        {
            return _saveGame.ComputerIPMap.First(x => x.Value == computer.Id).Key;
        }

        internal void DnsRegister(string domain, string ip)
        {
            if(!_saveGame.DomainNameMap.ContainsKey(domain))
            {
                _saveGame.DomainNameMap.Add(domain, ip);
            }
        }

        internal void MakeAdjacent(Identity identityA, Identity identityB)
        {
            if (_saveGame.AdjacentNodes.Any(x => (x.NodeA == identityA.Id && x.NodeB == identityB.Id) || (x.NodeB == identityA.Id && x.NodeA == identityB.Id)))
                return;

            _saveGame.AdjacentNodes.Add(new AdjacentNode
            {
                NodeA = identityA.Id,
                NodeB = identityB.Id
            });
        }

        internal bool LocationTooCloseToEntity(Vector2 location, float minDistance)
        {
            return _saveGame.EntityPositions.Any(x => Vector2.Distance(location, x.Position) < minDistance);
        }

        internal void AddIdentity(Identity identity)
        {
            if (Identities.Contains(identity))
                return;

            identity.Id = Identities.Select(x => x.Id).Distinct().Max() + 1;
            _saveGame.Characters.Add(identity);
        }

        internal void SetEntityPosition(Identity identity, Vector2 location)
        {
            if(_saveGame.EntityPositions.Any(x=>x.Id == identity.Id))
            {
                _saveGame.EntityPositions.First(x => x.Id == identity.Id).Position = location;
            }
            else
            {
                var p = new EntityPosition
                {
                    Id = identity.Id,
                    Position = location
                };
                _saveGame.EntityPositions.Add(p);
            }
        }

        internal bool GetPosition(Identity identity, out Vector2 position)
        {
            var posData = _saveGame.EntityPositions.FirstOrDefault(x => x.Id == identity.Id);

            position = posData?.Position ?? Vector2.Zero;
            return posData != null;
        }

        internal int GetSkillOf(Computer computer)
        {
            return Identities.First(x => x.Computers.Contains(computer.Id)).Skill;
        }

        internal void AssignStoryCharacterID(StoryCharacter character, int id)
        {
            if(_saveGame.StoryCharacterIDs.ContainsKey(character.Id))
            {
                _saveGame.StoryCharacterIDs[character.Id] = id;
            }
            else
            {
                _saveGame.StoryCharacterIDs.Add(character.Id, id);
            }
        }

        public event Action<IUserLand> PlayerSystemReady;

        public bool AreAllAssetsLoaded => (_itemLoadTask != null && _itemLoadTask.IsCompleted);

        internal bool GetStoryCharacterID(StoryCharacter character, out int id)
        {
            if(_saveGame.StoryCharacterIDs.ContainsKey(character.Id))
            {
                id = _saveGame.StoryCharacterIDs[character.Id];
                return true;
            }
            id = -1;
            return false;
        }

        private void InitializeWorld()
        {
            _kernels = new List<PlayerKernel>();

            _saveGame = new SaveGame();
            _commandAssets.Clear();

            foreach (var type in ReflectionTools.GetAll<Command>())
            {
                _commandAssets.Add(CommandAsset.FromCommand(type, _itemContainer.Content));
            }

            var playerPC = new Computer
            {
                Id = 0,
                Users = new List<User>
                {
                    new User {
                         Username = "root",
                         Password = "",
                         UserType = UserType.Admin
                    },
                    new User
                    {
                        Username = "user",
                        Password = "",
                        UserType = UserType.Sudoer
                    }
                }
            };

            _saveGame.Computers.Add(playerPC);

            var playerIdentity = new Identity
            {
                Id = 0,
                Computers = new List<int>
                 {
                     playerPC.Id
                 },
                Email = "user@email.com",
                Alias = "",
                IdentityType = IdentityType.Player,
                IsMissionImportant = false,
                Name = "Player",
                Reputation = 0,
                Skill = 0
            };

            _saveGame.Characters.Add(playerIdentity);

            _saveGame.PlayerCharacterID = 0;
            _saveGame.PlayerUserID = 1;

            PlayerSystemReady?.Invoke(GetPlayerUser());
        }

        public void Initialize(ContentManager content)
        {
            _itemContainer = new ItemContainer(content);

            _itemLoadTask = _itemContainer.LoadAsync();
        }

        public void Update(float deltaSeconds)
        {
            if(!_hasWorldBeenStarted)
            {
                if (AreAllAssetsLoaded)
                {
                    InitializeWorld();
                    _hasWorldBeenStarted = true;
                }
            }
        }

        public IEnumerable<CommandAsset> GetAvailableCommands(Computer computer)
        {
            return _commandAssets.Where(x => x.UnlockedByDefault || computer.Commands.Contains(x.Id));
        }

        public Computer GetComputer(int id)
        {
            return _saveGame.Computers.First(x => x.Id == id);
        }

        public Identity GetIdentity(int id)
        {
            return _saveGame.Characters.First(x => x.Id == id);
        }

        public IKernel GetKernel(int identity)
        {
            if (_kernels.Any(x => x.Identity.Id == identity))
                return _kernels.First(x => x.Identity.Id == identity);

            if (!_saveGame.Characters.Any(x => x.Id == identity)) throw new ArgumentException("Identity not found.");

            var kernel = new PlayerKernel(this, identity, GetIdentity(identity).Computers.First());
            _kernels.Add(kernel);
            return kernel;
        }

        public IUserLand GetPlayerUser()
        {
            return GetKernel(_saveGame.PlayerCharacterID).GetUserLand(_saveGame.PlayerUserID);
        }
    }

    public interface IProgramGuiBuilder
    {
        IProcess BuildProgram(Program program);
    }
}
