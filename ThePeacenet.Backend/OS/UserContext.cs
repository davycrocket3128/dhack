using System.Collections.Generic;
using ThePeacenet.Backend.AssetTypes;

namespace ThePeacenet.Backend.OS
{
    public sealed class UserContext
    {
        private SystemContext _system = null;
        private int _uid = 0;

        public UserContext(SystemContext system, int uid)
        {
            _uid = uid;
            _system = system;
        }

        public IFileSystem FileSystem => _system.GetFilesystem(_uid);
        public IEnumerable<CommandAsset> Commands => _system.InstalledCommands;
        public IEnumerable<Program> Programs => _system.InstalledPrograms;
        public string HomeFolder => _system.GetUserHomeDirectory(_uid);
        public string Hostname => _system.Hostname;
        public bool IsAdmin => _system.GetUserInfo(_uid).UserType == Data.UserType.Admin;
        public string Username => _system.GetUsername(_uid);

        public bool Execute(string command, out IProcess process)
        {
            return _system.ExecuteAs(_uid, command, out process);
        }
    }
}
