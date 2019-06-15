using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using ThePeacenet.Backend.AssetTypes;

namespace ThePeacenet.Backend.OS
{
    internal interface IUserLand : IContentProvider
    {
        string Username { get; }
        string Hostname { get; }
        string HomeFolder { get; }
        string IdentityName { get; }
        IFileSystem FileSystem { get; }
        string EmailAddress { get; }
        bool IsAdmin { get; }
        ConsoleColor UserColor { get; }
        IEnumerable<Program> Programs { get; }

        bool Execute(string program, out IProcess process);
    }
}
