using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using ThePeacenet.Backend.AssetAttributes;
using ThePeacenet.Backend.Shell;

namespace ThePeacenet.Commands
{
    [Description("Changes the current working directory.")]
    [UnlockedByDefault]
    [Usage("<path>")]
    public class Cd : Command
    {
        protected override void OnRun(string[] args)
        {
            string path = GetAbsolutePath(GetArgument("<path>").ToString());
            
            if(FileSystem.DirectoryExists(path))
            {
                Console.WorkingDirectory = path;
            }
            else
            {
                Console.WriteLine("{0}: {1}: directory not found.", CommandName, path);
            }
        }
    }
}
