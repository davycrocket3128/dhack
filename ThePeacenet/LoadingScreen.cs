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
        public LoadingScreen(ContentManager content)
        {
            this.Content = new Border
            {
                Content = new StackPanel
                {
                    Orientation = Orientation.Vertical,
                    HorizontalAlignment = HorizontalAlignment.Centre,
                    VerticalAlignment = VerticalAlignment.Centre,
                    Name = "LoadingStacker"
                }
            };

            this.FindControl<StackPanel>("LoadingStacker").Items.Add(new Label
            {
                Content = "The Peacenet - Project: Greenlight - Loading content..."
            });
        }
    }
}
