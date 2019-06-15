using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace ThePeacenet.Backend.OS
{
    public class NetworkedConsoleContext : IConsoleContext
    {
        private UserContext _hacker = null;
        private IConsoleContext _origin = null;
        private string _working = null;

        public NetworkedConsoleContext(IConsoleContext origin, UserContext hacker)
        {
            _origin = origin;
            _hacker = hacker;

            _working = User.HomeFolder;
        }

        public UserContext User => _hacker;

        public string WorkingDirectory
        {
            get => _working;
            set {
                if(_working != value && User.FileSystem.DirectoryExists(value))
                {
                    _working = value;
                }
            }
        }

        public void Clear()
        {
            _origin.Clear();
        }

        public bool GetLine(out string text)
        {
            return _origin.GetLine(out text);
        }

        public void OverWrite(string text)
        {
            _origin.OverWrite(text);
        }

        public void OverWrite(string format, params object[] args)
        {
            _origin.OverWrite(format, args);
        }

        public void Write(string text)
        {
            _origin.Write(text);
        }

        public void Write(string format, params object[] args)
        {
            _origin.Write(format, args);
        }

        public void WriteLine(string text)
        {
            _origin.WriteLine(text);
        }

        public void WriteLine(string format, params object[] args)
        {
            _origin.WriteLine(format, args);
        }
    }
}
