using Microsoft.Xna.Framework.Graphics;
using MonoGame.Extended.ViewportAdapters;
using Newtonsoft.Json;
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace ThePeacenet
{
    public class GameConfig
    {
        public bool StartFullscreen { get; set; } = true;
        public string Resolution { get; set; } = "System";
        
        public bool ParseResolution(out int width, out int height)
        {
            width = 0;
            height = 0;

            if (string.IsNullOrWhiteSpace(Resolution))
                return false;

            if (!Resolution.ToLower().Contains("x"))
                return false;

            int xIndex = Resolution.ToLower().IndexOf('x');

            if (!int.TryParse(Resolution.Substring(0, xIndex), out width))
                return false;

            if (!int.TryParse(Resolution.Substring(xIndex + 1), out height))
                return false;

            return true;
        }

        public void ApplySettings(GameInstance game)
        {
            // Should we use the system's current resolution?
            if (string.Equals(Resolution, "System", StringComparison.OrdinalIgnoreCase) || !ParseResolution(out int width, out int height))
            {
                // Apply the current resolution of the graphics adapter.
                game.GraphicsManager.PreferredBackBufferWidth = GraphicsAdapter.DefaultAdapter.CurrentDisplayMode.Width;
                game.GraphicsManager.PreferredBackBufferHeight = GraphicsAdapter.DefaultAdapter.CurrentDisplayMode.Height;
            }
            else
            {
                // Apply the parsed resolution.
                game.GraphicsManager.PreferredBackBufferWidth = width;
                game.GraphicsManager.PreferredBackBufferHeight = height;
            }

            System.Console.WriteLine("Applying graphics settings...");

            game.GraphicsManager.IsFullScreen = StartFullscreen;

            // Apply all graphics settings.
            game.GraphicsManager.ApplyChanges();
        }

        public static void Apply(GameInstance game)
        {
            string LocalUserDataPath = Environment.GetFolderPath(Environment.SpecialFolder.LocalApplicationData);
            string PeacenetPath = Path.Combine(LocalUserDataPath, "Bit Phoenix Software", "The Peacenet");

            if (!Directory.Exists(PeacenetPath))
                Directory.CreateDirectory(PeacenetPath);

            string ConfigPath = Path.Combine(PeacenetPath, "HypervisorConfig.json");

            if(!File.Exists(ConfigPath))
            {
                System.Console.WriteLine("Creating new config file in {0}", ConfigPath);
                var config = new GameConfig();
                config.ApplySettings(game);
                File.WriteAllText(ConfigPath, JsonConvert.SerializeObject(config, Formatting.Indented));
                return;
            }

            var existingConfig = JsonConvert.DeserializeObject<GameConfig>(File.ReadAllText(ConfigPath));
            existingConfig.ApplySettings(game);
        }
    }
}
