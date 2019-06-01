using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using ThePeacenet.Backend.OS;
using ThePeacenet.Desktop;
using ThePeacenet.Gui;
using ThePeacenet.Gui.Controls;

namespace ThePeacenet.GuiHandlers
{
    public abstract class GuiHandler
    {
        private Window _window = null;

        public Window Window => _window;
        public IUserLand User => Window.User;

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
