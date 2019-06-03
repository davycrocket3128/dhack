using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using ThePeacenet.Backend.AssetTypes;
using ThePeacenet.Backend.Data;
using ThePeacenet.Backend.OS;

namespace ThePeacenet.Backend
{
    public sealed class ProcgenEngine
    {
        private WorldState _world = null;
        private List<Asset> _peacenetSites = null;
        private Random _rng = null;
        private List<ProtocolImplementation> _protocolVersions = null;
        private List<Asset> _lootableFiles = null;
        private MarkovChain _maleNameGenerator = null;
        private MarkovChain _domainNameGenerator = null;
        private MarkovChain _usernameGenerator = null;
        private MarkovChain _femaleNameGenerator = null;
        private MarkovChain _lastNameGenerator = null;

        public Random RNG => _rng;

        public ProcgenEngine(WorldState world)
        {
            _world = world;
        }

        protected string[] GetMarkovData(MarkovTrainingDataUsage usage)
        {
            var assets = _world.Items.GetAll<MarkovTrainingDataAsset>().Where(x=>x.Usage == usage);

            if (assets.Count() == 0)
                return new string[0];

            List<string> data = new List<string>();

            foreach(MarkovTrainingDataAsset item in assets)
            {
                data.AddRange(item.TrainingData);
            }

            return data.ToArray();
        }

        protected void GenerateEmailServers()
        {
            throw new NotImplementedException();
        }

        protected void SpawnPeacenetSites()
        {
            throw new NotImplementedException();
        }

        protected void GenerateIdentityPosition(ref Identity pivot, ref Identity identity)
        {
            throw new NotImplementedException();
        }

        protected void UpdateStoryCharacter(StoryCharacter character)
        {
            throw new NotImplementedException();
        }

        protected ProtocolImplementation GetProtocolImplementation(Protocol protocol, int skill)
        {
            throw new NotImplementedException();
        }

        protected void GenerateAdjacentNodes(ref Identity identity)
        {
            throw new NotImplementedException();
        }

        protected void GenerateFirewall(ref Computer computer)
        {
            throw new NotImplementedException();
        }

        protected void GenerateNPCs()
        {
            throw new NotImplementedException();
        }

        protected void GenerateCharacterRelationships()
        {
            throw new NotImplementedException();
        }

        protected Identity GenerateNPC()
        {
            throw new NotImplementedException();
        }

        protected string GetPassword(int length)
        {
            throw new NotImplementedException();
        }

        protected void CleanEntities()
        {
            throw new NotImplementedException();
        }

        protected Computer GenerateComputer(string hostname, ComputerType type)
        {
            throw new NotImplementedException();
        }

        protected void UpdateStoryComputer(StoryCharacter character)
        {
            throw new NotImplementedException();
        }

        protected void PlaceLootables(IUserLand userLand)
        {
            throw new NotImplementedException();
        }

        protected string ChooseEmailDomain()
        {
            throw new NotImplementedException();
        }
    }

    public struct MarkovSource
    {
        private string Data;
        private char[] Chars;

        public void SetCount(int InCount)
        {
            Chars = new char[InCount];
        }

        public void Rotate(char nextChar)
        {
#if DEBUG
            if (Chars.Length < 1) throw new InvalidOperationException("debug assert: chars > 0 == false.");
#endif

            for (int i = 0; i < Chars.Length - 1; i++)
            {
                Chars[i] = Chars[i + 1];
            }
            Chars[Chars.Length - 1] = nextChar;
            Data = ToString();
        }

        public bool IsLessThan(MarkovSource OtherSource)
        {
            int i = 0;
            for (i = 0; i < Chars.Length - 1; i++)
            {
                if (Chars[i] != OtherSource.Chars[i]) break;
            }
            return Chars[i] < OtherSource.Chars[i];
        }

        public bool IsStartSource()
        {
            foreach (var Char in Chars)
            {
                if (Char != '\0') return false;
            }
            return true;
        }

        public static bool operator ==(MarkovSource a, MarkovSource b)
        {
            return a.Chars == b.Chars;
        }

        public static bool operator !=(MarkovSource a, MarkovSource b)
        {
            return a.Chars != b.Chars;
        }

        public override string ToString()
        {
            string Ret = "";

            for (int i = 0; i < this.Chars.Length; i++)
            {
                Ret += Chars[i];
            }

            return Ret;
        }

        public char[] GetChars()
        {
            return this.Chars;
        }
    }

    public class MarkovChain
    {
        Dictionary<MarkovSource, Dictionary<char, int>> MarkovMap;
        Random Random;
        int SourceCount = 0;

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
            if(KeyCount < 1) throw new InvalidOperationException("Debug assert.");
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
