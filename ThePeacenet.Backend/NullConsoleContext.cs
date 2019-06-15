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
        private readonly UserContext owner;

        public UserContext User => owner;

        public NullConsoleContext(UserContext owner)
        {
            this.owner = owner;
        }

        public string WorkingDirectory { get; set; }

        public void Clear()
        {
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
