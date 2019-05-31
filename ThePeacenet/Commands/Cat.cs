using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using ThePeacenet.Backend.AssetAttributes;
using ThePeacenet.Backend.Shell;

namespace ThePeacenet.Commands
{
    /// <summary>
    /// In-game Terminal Command that takes in a file path, reads the content of the file and prints it to the Terminal.
    /// The usage for the command is <c>cat <file></c>.
    /// </summary>
    /// <remarks>
    ///     <para>This class is a <see cref="ThePeacenet.Backend.Shell.Command" />.  As such, you should never need to directly instantiate it within the code.  The Peacenet will automatically load all <see cref="Command" />s when the game is run.
    /// </remarks>
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
