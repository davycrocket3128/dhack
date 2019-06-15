using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using ThePeacenet.Backend.OS;
using ThePeacenet.Gui;
using ThePeacenet.Gui.Controls;
using ThePeacenet.Gui.Windowing;

namespace ThePeacenet.Backend
{
    public abstract class GuiHandler : ITickable
    {
        private Window _window = null;

        public Window Window => _window;
        public UserContext User { get; internal set; }

        protected T FindControl<T>(string name) where T : Control
        {
            return Screen.FindControl<T>(_window, name);
        }

        public void Initialize(Window win)
        {
            _window = win;
        }

        public virtual void Update(float deltaSeconds)
        {

        }
    }
}
