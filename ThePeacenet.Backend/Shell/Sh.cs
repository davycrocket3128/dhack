using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using ThePeacenet.Backend.AssetAttributes;
using ThePeacenet.Backend.OS;

namespace ThePeacenet.Backend.Shell
{
    [UnlockedByDefault]
    [Description("The Peacegate command shell.")]
    public class Sh : Shell
    {
        protected override bool AllowPipes => true;
        protected override bool AllowRedirection => true;

        protected override string ShellPrompt
        {
            get
            {
                string shebang = "$";
                if (IsAdmin) shebang = "#";

                return $"{Username}@{Hostname}:{Console.WorkingDirectory.Replace(HomeFolder, "~")}{shebang} ";
            }
        }

        protected override Command GetCommand(string name)
        {
            IProcess process = null;

            if(Execute(name, out process) && process is Command)
            {
                return process as Command;
            }
            return null;
        }

        protected override bool RunSpecialCommand(string name, string[] args)
        {
            if(name == "help")
            {
                Console.WriteLine("Built-In Commands");
                Console.WriteLine("-----------------");
                Console.WriteLine(@"
exit:   Exits the shell.
clear:  Clears the screen of all text.
echo:   Writes the given text to the screen.
");

                return false; // Pass execution to the true help command.
            }
            return base.RunSpecialCommand(name, args);
        }

        protected override void OnRun(string[] args)
        {
        }
    }
}
