using Microsoft.Xna.Framework.Graphics;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using ThePeacenet.Backend;
using ThePeacenet.Gui.Controls;

namespace ThePeacenet.GuiHandlers
{
    public class Icedog : GuiHandler
    {
        public Image Envelope => FindControl<Image>("Envelope");
        public Label Username => FindControl<Label>("IcedogUsername");
        public Label Email => FindControl<Label>("IcedogEmail");


        public void Icedog_Load(object sender, EventArgs e)
        {
            Envelope.BackgroundBrush = new Gui.Brush(Content.Load<Texture2D>("Gui/Icons/envelope"), 48);
            Envelope.MinWidth = Envelope.MinHeight = 48;

            Username.Content = User.Username;
            Email.Content = User.EmailAddress;
        }
    }
}
