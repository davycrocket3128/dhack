using System;

namespace ThePeacenet.Backend
{
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
}
