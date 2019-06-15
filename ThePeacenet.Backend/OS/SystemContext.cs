using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using ThePeacenet.Backend.AssetTypes;
using ThePeacenet.Backend.Data;

namespace ThePeacenet.Backend.OS
{
    public sealed class SystemContext
    {
        private Identity _internalIdentity;
        private MailProvider _mailProvider;
        private string _currentHostname = null;
        private Dictionary<int, IFileSystem> _registeredFilesystems;
        private List<IProcess> _processes;
        private List<UserContext> _hackers;
        private Dictionary<int, UserContext> _users;
        private IKernel _kernel = null;

        private void UpdateInternalIdentity()
        {
            if (_internalIdentity == null)
                _internalIdentity = new Identity();
            this._internalIdentity.Id = -1;
            this._internalIdentity.Name = this.Hostname;
            this._internalIdentity.Alias = this.Hostname;
            this._internalIdentity.Email = this.Hostname + "@" + this.IPAddress;
            this._internalIdentity.IdentityType = IdentityType.None;
            this._internalIdentity.Skill = 0;
            this._internalIdentity.Reputation = 0;

            if (!_internalIdentity.Computers.Contains(Computer.Id))
                _internalIdentity.Computers.Add(Computer.Id);
        }

        public UserContext GetHackerContext(int InUserID, UserContext HackingUser)
        {
            throw new NotImplementedException();
        }

        private string UpdateHostname()
        {
            var fs = this.GetFilesystem(0);
            if(fs.FileExists("/etc/hostname"))
            {
                string hostname = fs.ReadText("/etc/hostname");
                if (hostname.Contains('\n'))
                    hostname = hostname.Substring(0, hostname.IndexOf('\n')).Trim();

                return _currentHostname = hostname;
            }
            return _currentHostname = "localhost";
        }

        public MailProvider MailProvider => _mailProvider;
        public WorldState Peacenet => _kernel.WorldState;
        public string EmailAddress => Identity.Email;
        public Identity Identity => _kernel.Identity;
        public Computer Computer => _kernel.Computer;
        public string Hostname => _currentHostname ?? UpdateHostname();
        public IEnumerable<Program> InstalledPrograms => Peacenet.Items.GetAll<Program>();
        public IEnumerable<IProcess> RunningProcesses => _processes;
        public IEnumerable<CommandAsset> InstalledCommands => _kernel.Commands;
        public IEnumerable<Exploit> Exploits => Peacenet.Items.GetAll<Exploit>();
        public string IPAddress => Peacenet.GetIPAddress(Computer);
        public IEnumerable<PayloadAsset> Payloads => Peacenet.Items.GetAll<PayloadAsset>();

        public IEnumerable<ServiceInfo> Services
        {
            get
            {
                if (Computer.Services.Count == 0)
                    Peacenet.GenerateServices(Computer);

                foreach (var svc in Computer.Services.Where(x => !x.IsCrashed))
                    yield return new ServiceInfo(this, svc);
            }
        }

        internal SystemContext(IKernel kernel)
        {
            _kernel = kernel;
            _users = new Dictionary<int, UserContext>();
            _registeredFilesystems = new Dictionary<int, IFileSystem>();
            Computer.FixUserIds();
        }

        public void Update(float deltaSeconds)
        {

        }

        public void AppendLog(string InLogText)
        {
            throw new NotImplementedException();
        }

        bool UsernameExists(string InUsername)
        {
            return Computer.Users.Any(x => x.Username == InUsername);
        }

        public SystemContext ConnectTo(string host)
        {
            return _kernel.ConnectTo(host).SystemContext;
        }

        public IEnumerable<AdjacentNodeInfo> ScanForAdjacentNodes()
        {
            List<AdjacentNodeInfo> nodes = new List<AdjacentNodeInfo>();

            foreach(var identity in Peacenet.GetAdjacentNodes(Identity))
            {
                var node = new AdjacentNodeInfo
                {
                    Name = identity.Name,
                    LinkA = this.Identity.Id,
                    LinkB = identity.Id
                };
                nodes.Add(node);
            }

            return nodes;
        }

        public bool ExecuteAs(int uid, string InCommand, out IProcess process)
        {
            return _kernel.Execute(_kernel.GetUserLand(uid), InCommand, out process);
        }

        public void UpdateSystemFiles()
        {
            throw new NotImplementedException();
        }

        public UserContext GetUserContext(int InUserID)
        {
            if (_users.ContainsKey(InUserID))
                return _users[InUserID];
            var user = new UserContext(this, InUserID);
            _users.Add(InUserID, user);
            return user;
        }

        public IFileSystem GetFilesystem(int UserID)
        {
            if (_registeredFilesystems.ContainsKey(UserID))
                return _registeredFilesystems[UserID];

            var fs = _kernel.GetUserLand(UserID).FileSystem;
            _registeredFilesystems.Add(UserID, fs);
            return fs;
        }

    public int GetUserIdFromUsername(string InUsername)
        {
            if (!UsernameExists(InUsername)) throw new InvalidOperationException(string.Format("{0} does not exist as a user on this system.", InUsername));
            return Computer.Users.First(x => x.Username == InUsername).Id;
        }

        public User GetUserInfo(int InUserID)
        {
            return Computer.Users.FirstOrDefault(x => x.Id == InUserID);
        }

    public string GetUsername(int InUserID)
        {
            return GetUserInfo(InUserID)?.Username ?? string.Empty;
        }

        public string GetUserHomeDirectory(int UserID)
        {
            var user = GetUserInfo(UserID);
            if (user == null) return string.Empty;
            if (user.UserType == UserType.Admin)
                return "/root";
            return "/home/" + user.Username;
        }
    }
}
