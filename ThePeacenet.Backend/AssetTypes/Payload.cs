using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using ThePeacenet.Backend.OS;

namespace ThePeacenet.Backend.AssetTypes
{
    public abstract class Payload
    {
        protected abstract void OnPayloadDeployed(IConsoleContext Console, UserContext OriginUser, UserContext TargetUser);

        protected void Disconnect()
        {
            Disconnected?.Invoke(this, EventArgs.Empty);
        }

        public event EventHandler Disconnected;

        public void DeployPayload(IConsoleContext OriginConsole, UserContext OriginUser, UserContext TargetUser)
        {
            // Create a networked console context that outputs to the origin but uses the hacked user as a means of
            // gaining a Peacegate context.
            NetworkedConsoleContext HackerContext = new NetworkedConsoleContext(OriginConsole, TargetUser);
            
            // This console is passed to the deriving payload methods - the payload is thus run in the context of the hacked system.
            this.OnPayloadDeployed(HackerContext, OriginUser, TargetUser);

        }
    }
}
