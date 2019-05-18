using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using ThePeacenet.Backend.AssetAttributes;
using ThePeacenet.Backend.Shell;

namespace ThePeacenet.Commands
{
    [Usage("<file>")]
    [Description("Types out the text in the specified file.")]
    [UnlockedByDefault]
    public class Cat : Command
    {
        protected override void OnRun(string[] args)
        {
            string path = GetAbsolutePath(GetArgument("<file>").ToString());

            if(FileSystem.FileExists(path))
            {
                Console.WriteLine(FileSystem.ReadText(path));
            }
            else
            {
                Console.WriteLine("{0}: {1}: File not found.", CommandName, path);
            }
        }
    }
}
