using Microsoft.Xna.Framework;
using Microsoft.Xna.Framework.Graphics;
using Microsoft.Xna.Framework.Input;
using MonoGame.Extended;
using MonoGame.Extended.Input.InputListeners;
using MonoGame.Extended.ViewportAdapters;
using ThePeacenet.Backend;
using ThePeacenet.Backend.OS;
using ThePeacenet.Backend.Shell;
using ThePeacenet.Console;
using SpriteFontPlus;
using System.IO;
using System;
using ThePeacenet.Gui;
using ThePeacenet.Desktop;
using ThePeacenet.Backend.AssetTypes;

namespace ThePeacenet
{
    /// <summary>
    /// This is the main type for your game.
    /// </summary>
    public class GameInstance : Game, IProgramGuiBuilder
    {
        private readonly GraphicsDeviceManager _graphics;
        private GuiSystem _guiSystem = null;
        private WorldState _worldState = null;
        private IUserLand _playerUserLand = null;
        private IGuiRenderer _renderer = null;
        private Brush _brush;
        private DynamicSpriteFont _defaultFont = null;

        public GameInstance()
        {
            _graphics = new GraphicsDeviceManager(this);
            Content.RootDirectory = "Content";
            IsMouseVisible = true;
            
            
        }

        protected override void Initialize()
        {
            GraphicsDevice.PresentationParameters.BackBufferWidth = 1280;
            GraphicsDevice.PresentationParameters.BackBufferHeight = 720;
            _renderer = new GuiSpriteBatchRenderer(GraphicsDevice);
            base.Initialize();
        }

        protected override void LoadContent()
        {
            var viewport = new DefaultViewportAdapter(GraphicsDevice);
            
            using (var stream = TitleContainer.OpenStream("Content/DefaultFont.ttf"))
            {
                byte[] data = new byte[stream.Length];
                stream.Read(data, 0, data.Length);
                _defaultFont = DynamicSpriteFont.FromTtf(data, 16);
            }

            _guiSystem = new GuiSystem(viewport, _renderer, _defaultFont);
            _guiSystem.ActiveScreen = new LoadingScreen(Content);

            _worldState = new WorldState(this);

            _worldState.PlayerSystemReady += (userland) =>
            {
                _playerUserLand = userland;

                _guiSystem.ActiveScreen = new DesktopScreen(Content, _playerUserLand);

                IProcess prc = null;
                userland.Execute("terminal", out prc);
                prc.Run(new NullConsoleContext(userland), new[] { "" });

            };

            _worldState.Initialize(Content);
        }

        protected override void UnloadContent()
        {
        }

        protected override void Update(GameTime gameTime)
        {
            _worldState.Update(gameTime.GetElapsedSeconds());
            _guiSystem.Update(gameTime);

            base.Update(gameTime);
        }

        protected override void Draw(GameTime gameTime)
        {
            GraphicsDevice.Clear(Color.Black);

            _guiSystem.Draw(gameTime);
            
            base.Draw(gameTime);
        }

        public IProcess BuildProgram(Backend.AssetTypes.Program program)
        {
            return new WindowProcess(program, this._guiSystem.ActiveScreen as DesktopScreen);
        }
    }
}
