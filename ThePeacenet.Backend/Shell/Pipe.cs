using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using ThePeacenet.Backend.AssetTypes;
using ThePeacenet.Backend.OS;

namespace ThePeacenet.Backend.Shell
{
    public class Pipe : IConsoleContext
    {
        private Pipe _input = null;
        private IConsoleContext _owner = null;
        private IConsoleContext _output = null;

        public string WorkingDirectory { get => _owner.WorkingDirectory; set => _owner.WorkingDirectory = value; }

        public Pipe(Pipe inputPipe, IConsoleContext owner, IConsoleContext output)
        {
            _output = output;
            _owner = owner;
            _input = inputPipe;
        }

        private StringBuilder _log = new StringBuilder();

        public StringBuilder Log => _log;

        public string Username => _owner.Username;

        public string Hostname => _owner.Hostname;

        public string HomeFolder => _owner.HomeFolder;

        public string IdentityName => _owner.IdentityName;

        public IFileSystem FileSystem => _owner.FileSystem;

        public string EmailAddress => _owner.EmailAddress;

        public bool IsAdmin => _owner.IsAdmin;

        public ConsoleColor UserColor => _owner.UserColor;

        public IEnumerable<CommandAsset> Commands => _owner.Commands;

        public void Clear()
        {
            if(_output != null)
            {
                _output.Clear();
            }
            else
            {
                Log.Clear();
            }
        }

        public bool GetLine(out string text)
        {
            if(_input != null)
            {
                if(string.IsNullOrEmpty(_input.Log.ToString()))
                {
                    text = "";
                    return false;
                }

                if(_input.Log.ToString().Contains("\n"))
                {
                    text = _input.Log.ToString().Substring(0, _input.Log.ToString().IndexOf("\n"));
                    _input.Log.Remove(0, _input.Log.ToString().IndexOf("\n") + 1);
                    return true;
                }
                else
                {
                    text = _input.Log.ToString();
                    _input.Log.Clear();
                    return true;
                }
            }

            return _owner.GetLine(out text);
        }

        public void OverWrite(string text)
        {
            if(_output != null)
            {
                _output.OverWrite(text);
            }
            else
            {
                _log.AppendLine(text);
            }
        }

        public void OverWrite(string format, params object[] args)
        {
            OverWrite(string.Format(format, args));
        }

        public void Write(string text)
        {
            if(_output != null)
            {
                _output.Write(text);
            }
            else
            {
                _log.Append(text);
            }
        }

        public void Write(string format, params object[] args)
        {
            Write(string.Format(format, args));
        }

        public void WriteLine(string text)
        {
            Write(text + "\n");
        }

        public void WriteLine(string format, params object[] args)
        {
            WriteLine(string.Format(format, args));
        }

        public bool Execute(string program, out IProcess process)
        {
            return _owner.Execute(program, out process);
        }
    }
}
