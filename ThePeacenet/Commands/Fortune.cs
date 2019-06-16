using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using ThePeacenet.Backend.AssetAttributes;
using ThePeacenet.Backend.Shell;

namespace ThePeacenet.Commands
{
    [Description("Prints a fortune, like you might find in a fortune cookie.")]
    [UnlockedByDefault]
    [Usage("none")]
    public class Fortune : Command
    {
        protected override void OnRun(string[] args)
        {
            // Honestly I have absolutely no idea how we should go about implementing this command, so for now, this is your fortune.
            Console.WriteLine("Those that work with haste are probably a waste.");
        }
    }
}
