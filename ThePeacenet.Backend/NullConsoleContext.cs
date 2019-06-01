using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using ThePeacenet.Backend.AssetTypes;
using ThePeacenet.Backend.OS;

namespace ThePeacenet.Backend
{
    public class NullConsoleContext : IConsoleContext
    {
        private readonly IUserLand owner;

        public NullConsoleContext(IUserLand owner)
        {
            this.owner = owner;

            WorkingDirectory = owner.HomeFolder;
        }

        public string WorkingDirectory { get; set; }

        public string Username => owner.Username;

        public string Hostname =>  owner.Hostname;

        public string HomeFolder => owner.HomeFolder;

        public string IdentityName => owner.IdentityName;

        public IFileSystem FileSystem => owner.FileSystem;

        public string EmailAddress => owner.EmailAddress;

        public bool IsAdmin => owner.IsAdmin;

        public ConsoleColor UserColor => owner.UserColor;

        public IEnumerable<CommandAsset> Commands => owner.Commands;

        public void Clear()
        {
        }

        public bool Execute(string program, out IProcess process)
        {
            return owner.Execute(program, out process);
        }

        public bool GetLine(out string text)
        {
            text = "";
            return false;
        }

        public void OverWrite(string text)
        {
        }

        public void OverWrite(string format, params object[] args)
        {
        }

        public void Write(string text)
        {
        }

        public void Write(string format, params object[] args)
        {
        }

        public void WriteLine(string text)
        {
        }

        public void WriteLine(string format, params object[] args)
        {
        }
    }
}
