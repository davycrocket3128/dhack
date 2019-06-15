using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using ThePeacenet.Backend.AssetAttributes;
using ThePeacenet.Backend.AssetTypes;
using ThePeacenet.Backend.Data;
using ThePeacenet.Backend.OS;
using DocoptNet;
using System.Collections;

namespace ThePeacenet.Backend.Shell
{
    public abstract class Command : ITickable, IProcess
    {
        private bool _started = false;
        private string[] _arguments = null;
        private IConsoleContext _console = null;
        private string _name = "";

        public bool Completed { get; private set; }
        public string[] Arguments => _arguments;
        public IConsoleContext Console => ConsoleOverride ?? _console;
        public string CommandName => _name;

        private IDictionary<string, ValueObject> _argMap = new Dictionary<string, ValueObject>();

        protected internal IConsoleContext ConsoleOverride { get; set; }
        protected virtual bool IsLatent { get => false; }

        public string Name => _name;

        public string Path => Name;

        public int ProcessId => 0;

        public RamUsage RamUsage { get; internal set; }

        protected virtual void OnUpdate(float deltaSeconds) { }

        public void Update(float deltaSeconds)
        {
            if (!Completed && _started)
                OnUpdate(deltaSeconds);
        }

        public UserContext User => Console.User;

        public string GetAbsolutePath(string path)
        {
            if (path.StartsWith("/"))
                return User.FileSystem.GetAbsolutePath(path);
            return User.FileSystem.GetAbsolutePath(Console.WorkingDirectory + "/" + path);
        }

        public Argument GetArgument(string argument)
        {
            return new Argument(_argMap[argument]);
        }

        public void Run(IConsoleContext console, string[] args)
        {
            _name = args[0];
            _console = console;
            _arguments = args.Skip(1).ToArray();

            string usage = this.GetUsageString();
            if (!string.IsNullOrWhiteSpace(usage))
            {
                try
                {
                    var docopt = new Docopt();
                    _argMap = docopt.Apply(usage, _arguments, false, null, false, false);
                }
                catch
                {
                    Console.WriteLine(usage);
                    Completed = true;
                    return;
                }
            }

            OnRun(_arguments);
            _started = true;
            if (!IsLatent) Completed = true;
        }

        protected abstract void OnRun(string[] args);

        protected void Complete()
        {
            Completed = true;
        }

        public void Kill()
        {
            Complete();
        }

        private string GetUsageString()
        {
            StringBuilder sb = new StringBuilder();
            bool hasUsages = false;
            sb.Append("usage: ");
            foreach (UsageAttribute attribute in this.GetType().GetCustomAttributes(false).Where(x => x is UsageAttribute))
            {
                hasUsages = true;
                sb.AppendLine(CommandName + " " + attribute.Usage);
                sb.Append("       ");
            }

            if (!hasUsages)
                return "";

            return sb.ToString().Trim();
        }
    }

    public class Argument
    {
        private ValueObject _value = null;

        internal Argument(ValueObject value)
        {
            _value = value;
            
        }

        public int AsInt => _value.AsInt;
        public ArrayList AsList => _value.AsList;
        public bool IsFalse => _value.IsFalse;
        public bool IsInt => _value.IsInt;
        public bool IsList => _value.IsList;
        public bool IsNullOrEmpty => _value.IsNullOrEmpty;
        public bool IsString => _value.IsString;
        public bool IsTrue => _value.IsTrue;
        public object Value => _value.Value;

        public override string ToString()
        {
            return _value.ToString();
        }
    }
}
