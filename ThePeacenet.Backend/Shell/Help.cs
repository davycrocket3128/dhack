using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using ThePeacenet.Backend.AssetAttributes;

namespace ThePeacenet.Backend.Shell
{
    [UnlockedByDefault]
    [Description("Shows the help screen.")]
    public class Help : Command
    {
        protected override void OnRun(string[] args)
        {
            // Get all installed commands and programs.
            //TArray<UPeacegateProgramAsset*> Programs = InConsole->GetUserContext()->GetOwningSystem()->GetInstalledPrograms();
            
            // user interface bullshitfucks
            Console.WriteLine("Help Command");
            Console.WriteLine("--------------\n");

            // All the descriptions of each command name.
            Dictionary<string, string> NameMap = new Dictionary<string, string>();

            // Maximum length of each name.
            int NameLength = 0;

            /*for (auto Program : Programs)
            {
                // Add it to the list of shit to display.
                NameMap.Add(Program->ID.ToString(), Program->Summary.ToString());

                // Update the length if we need to.
                if (NameLength < Program->ID.ToString().Len())
                    NameLength = Program->ID.ToString().Len();
            }*/

            foreach (var Command in Commands)
            {
                // Add it to the list of shit to display.
                NameMap.Add(Command.Id, Command.Description);

                // Update the length if we need to.
                if (NameLength < Command.Id.Length)
                    NameLength = Command.Id.Length;
            }

            // Loop through the names...
            foreach (var Key in NameMap.OrderBy(x=>x.Key))
            {
                Console.Write("{0}:", Key.Key);
                int Spaces = (NameLength - Key.Key.Length) + 1;
                for (int i = 0; i < Spaces; i++)
                    Console.Write(" ");
                Console.WriteLine(Key.Value);
            }
        }
    }
}
