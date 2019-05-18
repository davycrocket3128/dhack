using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Text.RegularExpressions;
using System.Threading.Tasks;
using ThePeacenet.Backend.AssetAttributes;
using ThePeacenet.Backend.Shell;

namespace ThePeacenet.Commands
{
    [UnlockedByDefault]
    [Description("Reads the output of a command and outputs only lines of text that match the specified regular expression.")]
    [Usage("<pattern>")]
    public class Grep : Command
    {
        protected override void OnRun(string[] args)
        {
            // The pattern used for matching.
            string pattern = this.GetArgument("<pattern>").ToString();

            // Read all of the text from standard input.
            List<string> input = new List<string>();

            string line = "";
            while (Console.GetLine(out line))
                input.Add(line);

            foreach(var i in input)
            {
                if(Regex.IsMatch(i, pattern))
                {
                    Console.WriteLine(i);
                }
            }
        }
    }
}
