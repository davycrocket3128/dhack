using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using ThePeacenet.Backend.AssetAttributes;
using ThePeacenet.Backend.Shell;

namespace ThePeacenet.Commands
{
    [Description("Create a directory.")]
    [UnlockedByDefault]
    [Usage("<path>")]
    public class Mkdir : Command
    {
        protected override void OnRun(string[] args)
        {
            string path = GetAbsolutePath(GetArgument("<path>").ToString());

            if(User.FileSystem.DirectoryExists(path))
            {
                Console.WriteLine("{0}: {1}: Directory exists.", CommandName, path);
            }
            else if(User.FileSystem.FileExists(path))
            {
                Console.WriteLine("{0}: {1}: File exists.", CommandName, path);
            }
            else
            {
                User.FileSystem.CreateDirectory(path);
            }
        }
    }
}
