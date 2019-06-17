using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using ThePeacenet.Backend.AssetAttributes;
using ThePeacenet.Backend.Shell;

namespace ThePeacenet.Commands
{
#if DEBUG
    [Description("Control the game's audio manager.")]
    [UnlockedByDefault]
    [Usage("play <contentPath> [-t <fadeLength>] [-l]")]
    public class Audio : Command
    {
        protected override void OnRun(string[] args)
        {
            string contentPath = GetArgument("<contentPath>").ToString();
            bool loop = GetArgument("-l").IsTrue;
            bool doFade = GetArgument("-t").IsTrue;
            float fadeLength = (doFade) ? ((float)GetArgument("<fadeLength>").AsInt / 1000) : 0;

            Console.WriteLine("Play: {0}", contentPath);
            Console.WriteLine("Loop: {0}", loop);
            Console.WriteLine("Fade: {0} ({1} seconds)", doFade, fadeLength);

            try
            {
                var audio = GameInstance.Get().Audio;
                if(doFade)
                {
                    audio.FadeToSong(contentPath, fadeLength, loop);
                }
                else
                {
                    audio.PlaySong(contentPath, loop);
                }
            }
            catch(Exception ex)
            {
                Console.WriteLine("Hey, I just prevented the game from crashing with a {0}", ex);
            }
        }
    }
#endif
}
