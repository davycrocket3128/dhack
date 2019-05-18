using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using ThePeacenet.Backend.AssetTypes;
using ThePeacenet.Backend.Data;
using ThePeacenet.Backend.OS;

namespace ThePeacenet.Backend.Shell
{
    public abstract class Command : ITickable, IProcess, IUserLand
    {
        private string[] _arguments = null;
        private IConsoleContext _console = null;
        private string _name = "";

        public bool Completed { get; private set; }
        public string[] Arguments => _arguments;
        public IConsoleContext Console => (ConsoleOverride == null) ? _console : ConsoleOverride; 
        public string CommandName => _name;

        protected internal IConsoleContext ConsoleOverride { get; set; }
        protected virtual bool IsLatent { get => false; }

        public string Username => Console.Username;

        public string Hostname => Console.Hostname;

        public string HomeFolder => Console.HomeFolder;

        public string IdentityName => Console.IdentityName;

        public IFileSystem FileSystem => Console.FileSystem;

        public string EmailAddress => Console.EmailAddress;

        public bool IsAdmin => Console.IsAdmin;

        public ConsoleColor UserColor => Console.UserColor;

        public string Name => _name;

        public string Path => Name;

        public int ProcessId => 0;

        public RamUsage RamUsage { get; internal set; }

        public IEnumerable<CommandAsset> Commands => Console.Commands;

        public virtual void Update(float deltaSeconds) { }

        public void Run(IConsoleContext console, string[] args)
        {
            _name = args[0];
            _console = console;
            _arguments = args.Skip(1).ToArray();

            OnRun(_arguments);

            if (!IsLatent) Completed = true;
        }

        protected abstract void OnRun(string[] args);
        
        protected void Complete()
        {
            Completed = true;
        }

        public bool Execute(string program, out IProcess process)
        {
            return Console.Execute(program, out process);
        }

        public void Kill()
        {
            Complete();
        }
    }
}
