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

        public MainMenu(ContentManager content, WorldState world)
        {
            _content = content;
            _world = world;

            Content = new Label
            {
                Content = "Some day, this will be a main menu."
            };
        }

        public override void Draw(IGuiContext context, IGuiRenderer renderer, float deltaSeconds)
        {
        }
    }
}
