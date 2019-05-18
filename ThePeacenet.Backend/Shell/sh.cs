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

                return $"{Username}@{Hostname}:{HomeFolder}{shebang} ";
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

        protected override void OnRun(string[] args)
        {
        }
    }
}
