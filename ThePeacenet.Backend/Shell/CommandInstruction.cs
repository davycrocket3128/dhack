using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace ThePeacenet.Backend.Shell
{
    public class CommandInstruction
    {
        public string Name { get; private set; }
        public string[] Arguments { get; private set; }
        public IConsoleContext Console { get; private set; }

        public CommandInstruction(string name, string[] args, IConsoleContext console)
        {
            Name = name;
            Arguments = args;
            Console = console;
        }
    }
}
