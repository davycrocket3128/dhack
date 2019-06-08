using Microsoft.Xna.Framework.Content;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using ThePeacenet.Backend;
using ThePeacenet.Gui;
using ThePeacenet.Gui.Controls;

namespace ThePeacenet
{
    public class MainMenu : Screen
    {
        private readonly ContentManager _content = null;
        private readonly WorldState _world = null;

        public Canvas RootCanvas => FindControl<Canvas>("RootCanvas");

        public MainMenu(ContentManager content, WorldState world)
        {
            Skin = content.Load<GuiSkin>("Skins/Menu");
            _content = content;
            _world = world;

            Content = new Border
            {
                StyleClass = "Wallpaper",
                Padding = new Thickness(25),
                Content = new Canvas
                {
                    Name = "RootCanvas"
                }
            };

            RootCanvas.Items.Add(new TextBox
            {
                Text = "Kaylin is cute <3"
            });
        }

        public override void Draw(IGuiContext context, IGuiRenderer renderer, float deltaSeconds)
        {
        }
    }
}
