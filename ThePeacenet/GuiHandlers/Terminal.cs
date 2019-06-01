using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using ThePeacenet.Backend.OS;
using ThePeacenet.Backend.Shell;
using ThePeacenet.Console;

namespace ThePeacenet.GuiHandlers
{
    public class Terminal : GuiHandler
    {
        private IProcess _shellProcess = null;

        public void Terminal_Load(object sender, EventArgs e)
        {
            var terminal = FindControl<ConsoleControl>("Console");

            User.Execute("sh", out _shellProcess);
            _shellProcess.Run(terminal, new string[] { "sh" });
        }

        public override void Update(float deltaSeconds)
        {
            if(_shellProcess is Command command)
            {
                command.Update(deltaSeconds);
                if(command.Completed)
                {
                    Window.Close();
                    _shellProcess = null;
                }
            }

            base.Update(deltaSeconds);
        }
    }
}
