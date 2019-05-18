using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace ThePeacenet.Backend.OS
{
    public interface IUserLand : IContentProvider
    {
        string Username { get; }
        string Hostname { get; }
        string HomeFolder { get; }
        string IdentityName { get; }
        IFileSystem FileSystem { get; }
        string EmailAddress { get; }
        bool IsAdmin { get; }
        ConsoleColor UserColor { get; }

        bool Execute(string program, out IProcess process);
    }
}
