using Microsoft.Xna.Framework;
using Microsoft.Xna.Framework.Audio;
using Microsoft.Xna.Framework.Content;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace ThePeacenet
{
    public class AudioManager
    {
        private ContentManager _content = null;
        private SoundEffectInstance _currentSong = null;
        private float _masterVolume = 1.0f;
        private SoundEffectInstance _nextSong = null;
        private bool _fading = false;
        private float _fadeProgress = 0;
        private float _fadeLength = 0;
        private Queue<AudioFadeRequest> _nextFades = new Queue<AudioFadeRequest>();

        public float MasterVolume
        {
            get => _masterVolume;
            set => _masterVolume = value;
        }

        public AudioManager(ContentManager content)
        {
            _content = content;
        }

        public void FadeToSong(string song, float fadeDuration = 2, bool loop = true)
        {
            FadeToSong(_content.Load<SoundEffect>(song), fadeDuration, loop);
        }

        public void FadeToSong(SoundEffect song, float fadeDuration = 2, bool loop = true)
        {
            // If we're not currently playing a song then we can simply skip the fade effect.
            if(_currentSong == null || _currentSong.IsDisposed)
            {
                PlaySong(song, loop);
                return;
            }

            // Queue the track to be faded in by the game when the time is right.
            _nextFades.Enqueue(new AudioFadeRequest
            {
                Song = song,
                FadeDuration = fadeDuration,
                Loop = loop
            });
        }

        public void PlaySong(SoundEffect song, bool loop = true)
        {
            if(_currentSong != null && !_currentSong.IsDisposed)
            {
                _currentSong.Stop();
                _currentSong.Dispose();
            }
            _currentSong = song.CreateInstance();
            _currentSong.IsLooped = loop;
            _currentSong.Play();
        }

        public void PlaySong(string path, bool loop = true)
        {
            PlaySong(_content.Load<SoundEffect>(path), loop);
        }

        public void Update(float deltaSeconds)
        {
            if (_fading)
            {
                if(_currentSong != null)
                {
                    _currentSong.Volume = _masterVolume * (1 - _fadeProgress);
                }
                if(_nextSong != null)
                {
                    _nextSong.Volume = _masterVolume * _fadeProgress;
                }
                _fadeProgress = MathHelper.Clamp(_fadeProgress + (deltaSeconds / _fadeLength), 0, 1);
                if(_fadeProgress >= 1)
                {
                    if(_currentSong != null)
                    {
                        _currentSong.Stop();
                        _currentSong.Dispose();
                        _currentSong = _nextSong;
                        _nextSong = null;
                        _fading = false;
                    }
                }
            }
            else
            {
                if (_nextFades.Count > 0)
                {
                    var request = _nextFades.Dequeue();
                    _fading = true;
                    _fadeProgress = 0;
                    _fadeLength = request.FadeDuration;
                    _nextSong = request.Song.CreateInstance();
                    _nextSong.Volume = 0;
                    _nextSong.IsLooped = request.Loop;
                    _nextSong.Play();
                }
                else
                {
                    if(_currentSong != null)
                    {
                        _currentSong.Volume = _masterVolume;
                    }
                }
            }
        }

        private class AudioFadeRequest
        {
            public SoundEffect Song;
            public float FadeDuration;
            public bool Loop;
        }
    }
}
