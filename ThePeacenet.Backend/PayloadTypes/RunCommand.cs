using Microsoft.Xna.Framework.Content;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using ThePeacenet.Backend.AssetTypes;
using ThePeacenet.Backend.OS;
using ThePeacenet.Backend.Shell;

namespace ThePeacenet.Backend.PayloadTypes
{
    public class RunCommand : Payload
    {
        public string Command { get; set; }

        [ContentSerializer(Optional = true, CollectionItemName = "Argument")]
        public List<string> Arguments { get; set; }

        protected override void OnPayloadDeployed(IConsoleContext Console, UserContext OriginUser, UserContext TargetUser)
        {
            if(TargetUser.Execute(Command, out IProcess process))
            {
                if (process is Command command)
                {
                    command.OnComplete += (o, a) =>
                    {
                        this.Disconnect();
                    };

                    var args = new string[Arguments.Count + 1];
                    args[0] = Command;
                    for (int i = 0; i < Arguments.Count; i++)
                        args[i + 1] = Arguments[i];

                    command.Run(Console, args);
                }
                else
                {
                    Console.WriteLine("{0}: command not found", Command);
                    Disconnect();
                }
            }
            else
            {
                Console.WriteLine("{0}: command not found", Command);
                Disconnect();
            }
        }
    }
}
