using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using ThePeacenet.Gui;
using ThePeacenet.Gui.Controls;
using Microsoft.Xna.Framework.Content;
using Microsoft.Xna.Framework;
using Microsoft.Xna.Framework.Graphics;
using ThePeacenet.Backend;
using ThePeacenet.Console;
using ThePeacenet.Backend.OS;
using MonoGame.Extended;

namespace ThePeacenet
{
    public class BootScreen : Screen
    {
        private ContentManager _content = null;
        private AudioManager _audio = null;
        private WorldState _world = null;
        private Queue<WriteInstruction> _writes = new Queue<WriteInstruction>();
        private float _tilNextWrite = 0;
        private string _text = "";
        private float _charWait = 0;
        private float _tilNextChar = 0;

        private bool _prebootFinished = false;

        private UserContext _playerUser = null;

        public Switcher Root => Content as Switcher;

        private ConsoleControl Console => FindControl<ConsoleControl>("Console");

        public BootScreen()
        {
            _content = GameInstance.Get().Content;
            _audio = GameInstance.Get().Audio;
            _world = GameInstance.Get().World;

            Skin = _content.Load<GuiSkin>("Skins/Serenity");

            Content = new Switcher();
            Root.Items.Add(new ConsoleControl { Name = "Console" });

            var loader = new StackPanel
            {
                Spacing = 5,
                VerticalAlignment = VerticalAlignment.Centre,
                HorizontalAlignment = HorizontalAlignment.Centre
            };

            loader.Items.Add(new Image
            {
                BackgroundBrush = new Brush(_content.Load<Texture2D>("Gui/Textures/Peacenet"))
            });
            loader.Items.Add(new ProgressBar
            {
                Marquee = true,
                HorizontalAlignment = HorizontalAlignment.Centre,
                Width = 200
            });
            Root.Items.Add(loader);

            _world.PlayerSystemReady += _world_PlayerSystemReady;

            if (_world.IsNewGame)
            {
                _writes.Enqueue(new WriteInstruction("Bit Phoenix Software presents...", 1, 0.005f));
                _writes.Enqueue(new WriteInstruction("A game by Michael VanOverbeek...", 1, 0.005f));
                _writes.Enqueue(new WriteInstruction(""));
                
            }
        }

        private void _world_PlayerSystemReady(UserContext obj)
        {
            _playerUser = obj;
        }

        public override void Update(GameTime gameTime)
        {
            base.Update(gameTime);

            if (!string.IsNullOrEmpty(_text))
            {
                if(_charWait > 0)
                {
                    _charWait -= gameTime.GetElapsedSeconds();
                }
                else
                {
                    Console.Write(_text[0].ToString());
                    _text = _text.Remove(0, 1);
                    _charWait = _tilNextChar;
                }
                return;
            }

            if(_writes.Count > 0)
            {
                if(_tilNextWrite > 0)
                {
                    _tilNextWrite -= gameTime.GetElapsedSeconds();
                }
                else
                {
                    var i = _writes.Dequeue();
                    _tilNextWrite = i.Wait;
                    _charWait = 0;
                    _tilNextChar = i.TypeDelay;
                    _text = i.Text + Environment.NewLine;
                }
            }
            else
            {
                _prebootFinished = true;
                Root.ActiveIndex++;
            }

            if(_prebootFinished && _playerUser != null)
            {
                GoTo(new Desktop.DesktopScreen(_content, _playerUser));
            }
        }

        private struct WriteInstruction
        {
            public string Text;
            public float Wait;
            public float TypeDelay;

            public WriteInstruction(string text) : this(text, 0)
            {

            }

            public WriteInstruction(string text, float wait) : this(text, wait, 0)
            {
            }

            public WriteInstruction(string text, float wait, float typeDelay)
            {
                Wait = wait;
                Text = text;
                TypeDelay = typeDelay;
            }
        }
    }
}
