using System;
using System.Collections.Generic;
using System.Linq;

namespace ThePeacenet.Backend
{
    public class MarkovChain
    {
        private Dictionary<MarkovSource, Dictionary<char, int>> MarkovMap = new Dictionary<MarkovSource, Dictionary<char, int>>();
        private Random Random;
        private readonly int SourceCount = 0;

        char GetNext(MarkovSource InSource)
        {
            if (!MarkovMap.ContainsKey(InSource))
                return '\0';

            var Map = this.MarkovMap[InSource];

            int Total = 0;

            char[] Keys = Map.Keys.ToArray();

            for (int i = 0; i < Keys.Length; i++)
            {
                Total += Map[Keys[i]];
            }

            int Rng = this.Random.Next(0, Total);

            for (int i = 0; i < Keys.Length; i++)
            {
                Rng -= Map[Keys[i]];
                if (Rng < 0)
                {
                    return Keys[i];
                }
            }

            return '\0';
        }

        MarkovSource RandomSource()
        {
            MarkovSource[] Keys = this.MarkovMap.Keys.ToArray();

            return Keys[Random.Next(0, Keys.Length)];
        }

        bool IsDeadEnd(MarkovSource InSource, int Depth)
        {
            if (Depth <= 0)
                return false;

            if (!MarkovMap.ContainsKey(InSource))
                return true;

            var Map = this.MarkovMap[InSource];

            char[] Keys = Map.Keys.ToArray();

            if (Keys.Length == 1)
                if (Keys[0] == '\0')
                    return true;

            MarkovSource TempSource = InSource;

            for (int i = 0; i < Keys.Length; ++i)
            {
                TempSource = InSource;
                TempSource.Rotate(Keys[i]);
                if (!IsDeadEnd(TempSource, Depth - 1)) return false;
            }

            return true;
        }

        char GetNextCharGuarantee(MarkovSource InSource, int InSteps)
        {
#if DEBUG
            if(IsDeadEnd(InSource, InSteps)) throw new InvalidOperationException("Debug assert.");
#endif

            Dictionary<char, int> Temp = new Dictionary<char, int>();
            var Map = MarkovMap[InSource];

            var Keys = Map.Keys.ToArray();

            if (Keys.Length == 1)
                return Keys[0];

#if DEBUG
            if(Keys.Length < 1) throw new InvalidOperationException("Debug assert.");
#endif

            int Total = 0;
            for (int i = 0; i < Keys.Length; ++i)
            {
                MarkovSource TempSource = InSource;
                TempSource.Rotate(Keys[i]);
                if (!IsDeadEnd(TempSource, InSteps))
                {
                    if (Temp.ContainsKey(Keys[i]))
                        Temp[Keys[i]] = Map[Keys[i]];
                    else
                        Temp.Add(Keys[i], Map[Keys[i]]);
                    Total += Map[Keys[i]];
                }
            }

            int Rng = Random.Next(Total);

            char[] TempKeys = Temp.Keys.ToArray();

            for (int i = 0; i < TempKeys.Length; i++)
            {
                Rng -= Temp[TempKeys[i]];
                if (Rng < 0)
                {
                    return TempKeys[i];
                }
            }

#if DEBUG
            if(Rng >= 0) throw new InvalidOperationException("Debug assert.");
#endif

            return '\0';
        }

        public MarkovChain(string[] InArray, int N, Random InRng)
        {
            this.Random = InRng;

            foreach (string ArrayString in InArray)
            {
                MarkovSource Source = new MarkovSource();
                Source.SetCount(N);

                foreach (char Char in ArrayString)
                {
                    if (Char == '\0')
                        break;

                    if (!MarkovMap.ContainsKey(Source))
                        MarkovMap.Add(Source, new Dictionary<char, int>());

                    if (!MarkovMap[Source].ContainsKey(Char))
                        MarkovMap[Source].Add(Char, 0);

                    MarkovMap[Source][Char]++;
                    Source.Rotate(Char);
                }
                if (!MarkovMap.ContainsKey(Source))
                    MarkovMap.Add(Source, new Dictionary<char, int>());

                if (!MarkovMap[Source].ContainsKey('\0'))
                    MarkovMap[Source].Add('\0', 0);

                MarkovMap[Source]['\0']++;
            }

            SourceCount = N;
        }

        public string GetMarkovString(int InLength)
        {
            string Out = "";

            MarkovSource src = new MarkovSource();
            src.SetCount(SourceCount);

            if (InLength < 1)
            {
                char tmp = GetNext(src);
                while (tmp != '\0')
                {
                    Out += tmp;
                    src.Rotate(tmp);
                    tmp = GetNext(src);
                }
                return Out;
            }
            for (int i = 0; i < InLength; ++i)
            {
                int x = (i > InLength - SourceCount ? InLength - i : SourceCount);
                char tmp = GetNextCharGuarantee(src, x);
                Out += tmp;
                src.Rotate(tmp);
            }
            return Out;
        }
    }
}
