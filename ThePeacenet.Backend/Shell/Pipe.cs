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
        private string _redirectPath = "";
        private bool _redirectOverwrite = false;

        public UserContext User => _owner.User;

        internal void Redirect(string path, bool overwrite)
        {
            _redirectPath = path;
            _redirectOverwrite = overwrite;
        }

        internal void WriteRedirect()
        {
            if (string.IsNullOrWhiteSpace(_redirectPath)) return;

            if(_redirectOverwrite)
            {
                User.FileSystem.WriteText(_redirectPath, Log.ToString());
            }
            else
            {
                string text = User.FileSystem.ReadText(_redirectPath);
                User.FileSystem.WriteText(_redirectPath, text + Log.ToString());
            }
        }

        public string WorkingDirectory { get => _owner.WorkingDirectory; set => _owner.WorkingDirectory = value; }

        public Pipe(Pipe inputPipe, IConsoleContext owner, IConsoleContext output)
        {
            _output = output;
            _owner = owner;
            _input = inputPipe;
        }

        private StringBuilder _log = new StringBuilder();

        public StringBuilder Log => _log;

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
    }
}
