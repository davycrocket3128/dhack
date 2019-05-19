using Microsoft.Xna.Framework.Graphics;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace ThePeacenet.Backend.Gui
{
    public interface IWindow
    {
        bool IsMaximized { get; }
        bool IsFocused { get; }

        string WindowTitle { get; set; }
        Texture2D WindowIcon { get; set; }

        bool CanClose { get; set; }
        bool CanMinimize { get; set; }
        bool CanMazimize { get; set; }

        void Close();
        void Minimize();
        void Maximize();
        void Restore();
    }
}
