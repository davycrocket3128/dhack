using Microsoft.Xna.Framework.Content;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using ThePeacenet.Gui;
using ThePeacenet.Gui.Controls;

namespace ThePeacenet
{
    public class LoadingScreen : Screen
    {
        public StackPanel Root => Content as StackPanel;

        public LoadingScreen(ContentManager content)
        {
            this.Content = new StackPanel
            {
                HorizontalAlignment = HorizontalAlignment.Centre,
                VerticalAlignment = VerticalAlignment.Centre,
                Spacing = 5
            };

            Root.Items.Add(new Label
            {
                Content = "The Peacenet - Loading..."
            });

            Root.Items.Add(new ProgressBar
            {
                Marquee = true
            });
        }
    }
}
