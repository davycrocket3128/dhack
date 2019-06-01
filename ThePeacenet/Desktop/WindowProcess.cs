using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using ThePeacenet.Backend;
using ThePeacenet.Backend.Data;
using ThePeacenet.Backend.OS;

namespace ThePeacenet.Desktop
{
    public class WindowProcess : IProcess
    {
        public WindowProcess(ThePeacenet.Backend.AssetTypes.Program program, DesktopScreen desktop)
        {
            Program = program;
            Desktop = desktop;
        }

        public string Name => Window.WindowTitle;

        public string Path => Program.Id;

        public RamUsage RamUsage => Program.RamUsage;

        public Window Window { get; private set; }
        public ThePeacenet.Backend.AssetTypes.Program Program { get; }
        public DesktopScreen Desktop { get; }

        public void Kill()
        {
            Window.Close();
        }

        public void Run(IConsoleContext console, string[] arguments)
        {
            Window = new Window(Program, console);
            Desktop.ShowWindow(Window);
        }
    }
}
