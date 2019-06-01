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
        private IProgramGuiBuilder _guiBuilder = null;
        private SaveGame _saveGame = null;
        private List<CommandAsset> _commandAssets = new List<CommandAsset>();
        private List<PlayerKernel> _kernels = new List<PlayerKernel>();
        private ItemContainer _itemContainer = null;
        private List<Exploit> _exploits = new List<Exploit>();
        private Task _itemLoadTask = null;
        private bool _hasWorldBeenStarted = false;

        public IProgramGuiBuilder GuiBuilder => _guiBuilder;
        
        public WorldState(IProgramGuiBuilder guiBuilder)
        {
            _guiBuilder = guiBuilder;
        }

        public ItemContainer Items => _itemContainer;

        public event Action<IUserLand> PlayerSystemReady;

        public bool AreAllAssetsLoaded => (_itemLoadTask != null && _itemLoadTask.IsCompleted);

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
