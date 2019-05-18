using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using ThePeacenet.Backend.AssetAttributes;
using ThePeacenet.Backend.Shell;

namespace ThePeacenet.Commands
{
    [Description("Lists the contents of the current working directory.")]
    [UnlockedByDefault]
    [Usage("[-a]")]
    public class Ls : Command
    {
        protected override void OnRun(string[] args)
        {
            bool listAll = GetArgument("-a").IsTrue;

            foreach(var dir in FileSystem.GetDirectories(Console.WorkingDirectory))
            {
                if (dir.StartsWith(".") && !listAll) continue;
                Console.WriteLine(dir);
            }

            foreach (var file in FileSystem.GetFiles(Console.WorkingDirectory))
            {
                if (file.StartsWith(".") && !listAll) continue;
                Console.WriteLine(file);
            }
        }
    }
}
