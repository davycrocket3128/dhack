using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using ThePeacenet.Backend.AssetTypes;
using ThePeacenet.Backend.Data;
using ThePeacenet.Backend.Shell;

namespace ThePeacenet.Backend.OS
{
    public class PlayerKernel : IKernel
    {
        private WorldState _worldState = null;
        private int _characterId = 0;
        private int _computerId = 0;
        private List<UserLand> _users = new List<UserLand>();
        public WorldState WorldState => _worldState;

        public Computer Computer => WorldState.GetComputer(_computerId);
        public Identity Identity => WorldState.GetIdentity(_characterId);

        public IEnumerable<CommandAsset> Commands => WorldState.GetAvailableCommands(Computer);

        public PlayerKernel(WorldState world, int identity, int computer)
        {
            _worldState = world;
            _characterId = identity;
            _computerId = computer;
        }

        public IUserLand GetUserLand(int userId)
        {
            if (_users.Any(x => x.UserID == userId))
                return _users.First(x => x.UserID == userId);

            var userLand = new UserLand(this, userId);

            _users.Add(userLand);

            return userLand;
        }

        public User GetUser(int uid)
        {
            if (uid < 0 || uid >= Computer.Users.Count) throw new ArgumentOutOfRangeException(nameof(uid));

            return Computer.Users[uid];
        }

        public bool Execute(IUserLand user, string program, out IProcess process)
        {
            var commands = Commands;

            if(commands.Any(x=>x.Id == program))
            {
                process = (IProcess)Activator.CreateInstance(commands.First(x => x.Id == program).CommandType, null);

                (process as Command).RamUsage = commands.First(x => x.Id == program).RamUsage;

                return true;
            }

            process = null;
            return false;
        }
    }

    public class UserLand : IUserLand
    {
        private PlayerKernel _kernel = null;
        private int _uid = 0;

        internal int UserID => _uid;

        public UserLand(PlayerKernel kernel, int uid)
        {
            _kernel = kernel;
            _uid = uid;
        }

        public User User => _kernel.GetUser(_uid);

        public string Username => User.Username;

        public string Hostname => "localhost";

        public string HomeFolder => (IsAdmin) ? "/root" : "/home/" + Username;

        public string IdentityName => _kernel.Identity.Name;

        public IFileSystem FileSystem => null;

        public string EmailAddress => _kernel.Identity.Email;

        public bool IsAdmin => User.UserType == UserType.Admin;

        public ConsoleColor UserColor => User.UserColor;

        public IEnumerable<CommandAsset> Commands => _kernel.Commands;

        public bool Execute(string program, out IProcess process)
        {
            return _kernel.Execute(this, program, out process);
        }
    }
}
