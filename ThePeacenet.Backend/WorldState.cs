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
        private List<CommandAsset> _commandAssets = new List<CommandAsset>();
        private List<PlayerKernel> _kernels = new List<PlayerKernel>();
        private ItemContainer _itemContainer = null;
        private readonly List<Exploit> _exploits = new List<Exploit>();
        private Task _itemLoadTask = null;
        private bool _hasWorldBeenStarted = false;
        private ProcgenEngine _procgen = null;
        private SaveManager _saveManager = null;

        public IProgramGuiBuilder GuiBuilder => _guiBuilder;
        public IEnumerable<AdjacentNode> AdjacentNodes => CurrentSave.AdjacentNodes;
        public string GameDataPath => Path.Combine(Environment.GetFolderPath(Environment.SpecialFolder.LocalApplicationData), "Bit Phoenix Software", "The Peacenet");

        public event EventHandler PreloadFinished;

        public WorldState(IProgramGuiBuilder guiBuilder)
        {
            _guiBuilder = guiBuilder;
            _procgen = new ProcgenEngine(this);
            _saveManager = new SaveManager(this);
        }

        public IEnumerable<Computer> Computers => CurrentSave.Computers;
        public IEnumerable<CharacterRelationship> Relationships => CurrentSave.CharacterRelationships;
        public IEnumerable<string> DomainNames => CurrentSave.DomainNameMap.Keys;
        public ItemContainer Items => _itemContainer;
        public IEnumerable<Identity> Identities => CurrentSave.Characters;
        public bool IsNewGame => CurrentSave.IsNewGame;
        public int Seed { get => CurrentSave.WorldSeed; internal set => CurrentSave.WorldSeed = value; }

        internal SaveGame CurrentSave => _saveManager.CurrentSave;

        public IEnumerable<Identity> GetAdjacentNodes(Identity identity)
        {
            throw new NotImplementedException();
        }

        internal void AssignIP(Computer computer, string ip)
        {
            CurrentSave.ComputerIPMap.Add(ip, computer.Id);
        }

        internal void AddComputer(Computer computer)
        {
            CurrentSave.Computers.Add(computer);
        }

        internal void RemoveRelationship(CharacterRelationship relationship)
        {
            CurrentSave.CharacterRelationships.Remove(relationship);
        }

        public Computer DnsResolve(string hostname)
        {
            if(CurrentSave.DomainNameMap.ContainsKey(hostname))
            {
                string ip = CurrentSave.DomainNameMap[hostname];
                return DnsResolve(ip);
            }

            if(CurrentSave.ComputerIPMap.ContainsKey(hostname))
            {
                return this.GetComputer(CurrentSave.ComputerIPMap[hostname]);
            }

            return null;
        }

        internal void AddRelationship(CharacterRelationship relationship)
        {
            CurrentSave.CharacterRelationships.Add(relationship);
        }

        public string GetIPAddress(Computer computer)
        {
            return CurrentSave.ComputerIPMap.First(x => x.Value == computer.Id).Key;
        }

        internal void DnsRegister(string domain, string ip)
        {
            if(!CurrentSave.DomainNameMap.ContainsKey(domain))
            {
                CurrentSave.DomainNameMap.Add(domain, ip);
            }
        }

        internal void MakeAdjacent(Identity identityA, Identity identityB)
        {
            if (CurrentSave.AdjacentNodes.Any(x => (x.NodeA == identityA.Id && x.NodeB == identityB.Id) || (x.NodeB == identityA.Id && x.NodeA == identityB.Id)))
                return;

            CurrentSave.AdjacentNodes.Add(new AdjacentNode
            {
                NodeA = identityA.Id,
                NodeB = identityB.Id
            });
        }

        internal bool LocationTooCloseToEntity(Vector2 location, float minDistance)
        {
            return CurrentSave.EntityPositions.Any(x => Vector2.Distance(location, x.Position) < minDistance);
        }

        internal void AddIdentity(Identity identity)
        {
            if (Identities.Contains(identity))
                return;

            identity.Id = Identities.Select(x => x.Id).Distinct().Max() + 1;
            CurrentSave.Characters.Add(identity);
        }

        internal void SetEntityPosition(Identity identity, Vector2 location)
        {
            if(CurrentSave.EntityPositions.Any(x=>x.Id == identity.Id))
            {
                CurrentSave.EntityPositions.First(x => x.Id == identity.Id).Position = location;
            }
            else
            {
                var p = new EntityPosition
                {
                    Id = identity.Id,
                    Position = location
                };
                CurrentSave.EntityPositions.Add(p);
            }
        }

        internal bool GetPosition(Identity identity, out Vector2 position)
        {
            var posData = CurrentSave.EntityPositions.FirstOrDefault(x => x.Id == identity.Id);

            position = posData?.Position ?? Vector2.Zero;
            return posData != null;
        }

        internal int GetSkillOf(Computer computer)
        {
            return Identities.First(x => x.Computers.Contains(computer.Id)).Skill;
        }

        internal void AssignStoryCharacterID(StoryCharacter character, int id)
        {
            if(CurrentSave.StoryCharacterIDs.ContainsKey(character.Id))
            {
                CurrentSave.StoryCharacterIDs[character.Id] = id;
            }
            else
            {
                CurrentSave.StoryCharacterIDs.Add(character.Id, id);
            }
        }

        public event Action<UserContext> PlayerSystemReady;

        public bool AreAllAssetsLoaded => (_itemLoadTask != null && _itemLoadTask.IsCompleted);

        internal bool GetStoryCharacterID(StoryCharacter character, out int id)
        {
            if(CurrentSave.StoryCharacterIDs.ContainsKey(character.Id))
            {
                id = CurrentSave.StoryCharacterIDs[character.Id];
                return true;
            }
            id = -1;
            return false;
        }

        public IEnumerable<SaveInfo> AvailableAgents => _saveManager.AvailableAgents;
        public bool IsInGame => CurrentSave != null;

        public void Save()
        {
            if (!IsInGame) throw new InvalidOperationException("There's no save file currently loaded.");

            _saveManager.Save();
        }

        public void StartGame(SaveInfo info)
        {
            _saveManager.Load(info);

            InitializeWorld();
        }

        private void InitializeWorld()
        {
            _kernels = new List<PlayerKernel>();

            _commandAssets.Clear();

            foreach (var type in ReflectionTools.GetAll<Command>())
            {
                _commandAssets.Add(CommandAsset.FromCommand(type, _itemContainer.Content));
            }

            // Start first few phases of world generation.
            _procgen.Initialize();

            PlayerSystemReady?.Invoke(GetPlayerUser());
        }

        internal IKernel GetKernel(Computer computer)
        {
            if (_kernels.Any(x => x.Computer.Id == computer.Id))
                return _kernels.First(x => x.Computer.Id == computer.Id);

            Identity identity = null;
            if(computer.OwnerType != IdentityType.None)
            {
                identity = this.Identities.First(x => x.Computers.Contains(computer.Id));
            }

            var kernel = new PlayerKernel(this, identity?.Id ?? -1, computer.Id);
            _kernels.Add(kernel);
            return kernel;
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
                    PreloadFinished?.Invoke(this, EventArgs.Empty);
                    _hasWorldBeenStarted = true;
                }
            }

            foreach (var kernel in _kernels)
                kernel.Update(deltaSeconds);
        }

        public void StartNewGame(string playerName)
        {
            _saveManager.NewGame(playerName);

            InitializeWorld();
        }

        public IEnumerable<CommandAsset> GetAvailableCommands(Computer computer)
        {
            return _commandAssets.Where(x => x.UnlockedByDefault || computer.Commands.Contains(x.Id));
        }

        public Computer GetComputer(int id)
        {
            return CurrentSave.Computers.First(x => x.Id == id);
        }

        public Identity GetIdentity(int id)
        {
            return CurrentSave.Characters.First(x => x.Id == id);
        }

        internal IKernel GetKernel(int identity)
        {
            if (_kernels.Any(x => x.Identity.Id == identity))
                return _kernels.First(x => x.Identity.Id == identity);

            if (!CurrentSave.Characters.Any(x => x.Id == identity)) throw new ArgumentException("Identity not found.");

            var kernel = new PlayerKernel(this, identity, GetIdentity(identity).Computers.First());
            _kernels.Add(kernel);
            return kernel;
        }

        public UserContext GetPlayerUser()
        {
            return GetKernel(CurrentSave.PlayerCharacterID).SystemContext.GetUserContext(CurrentSave.PlayerUserID);
        }
    }

    public interface IProgramGuiBuilder
    {
        IProcess BuildProgram(Program program);
    }
}
