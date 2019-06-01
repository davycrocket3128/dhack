using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using ThePeacenet.Backend.Data;

namespace ThePeacenet.Backend.OS
{
    public interface IProcess
    {
        string Name { get; }
        string Path { get; }
        RamUsage RamUsage { get; }

        void Run(IConsoleContext console, string[] arguments);
        void Kill();
    }
}
