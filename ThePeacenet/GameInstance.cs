using Microsoft.Xna.Framework;
using Microsoft.Xna.Framework.Graphics;
using Microsoft.Xna.Framework.Input;
using MonoGame.Extended;
using MonoGame.Extended.ViewportAdapters;
using ThePeacenet.Backend;
using ThePeacenet.Backend.OS;
using ThePeacenet.Backend.Shell;
using ThePeacenet.Console;
using ThePeacenet.Gui;

namespace ThePeacenet
{
    /// <summary>
    /// This is the main type for your game.
    /// </summary>
    public class GameInstance : Game
    {
        private readonly GraphicsDeviceManager _graphics;
        private Shell _commandShell = null;
        private ViewportConsole _console = null;
        private GuiSystem _guiSystem = null;
        private WorldState _worldState = null;
        private IUserLand _playerUserLand = null;

        public GameInstance()
        {
            _graphics = new GraphicsDeviceManager(this);
            Content.RootDirectory = "Content";
            IsMouseVisible = true;
        }

        protected override void Initialize()
        {
            base.Initialize();
        }

        protected override void LoadContent()
        {
            var viewportAdapter = new DefaultViewportAdapter(this.GraphicsDevice);

            _worldState = new WorldState();
            _worldState.Initialize();

            _playerUserLand = _worldState.GetPlayerUser();

            _guiSystem = new GuiSystem(viewportAdapter);
            _console = new ViewportConsole(viewportAdapter, Content, _playerUserLand);
            _commandShell = new Sh();
            _commandShell.Run(_console, new[] { "" });
        }

        protected override void UnloadContent()
        {
        }

        protected override void Update(GameTime gameTime)
        {
            _worldState.Update(gameTime.GetElapsedSeconds());

            _guiSystem.Update(gameTime);
            _console.Update(gameTime);
            _commandShell.Update(gameTime.GetElapsedSeconds());

            if (_commandShell.Completed) this.Exit();

            base.Update(gameTime);
        }

        protected override void Draw(GameTime gameTime)
        {
            _guiSystem.Draw(gameTime);
            _console.Draw(gameTime);

            base.Draw(gameTime);
        }
    }
}
