using System;
using System.Collections.Generic;
using System.Linq;

namespace ThePeacenet.Backend
{
    public class MarkovChain
    {
        private readonly Random Random;
        private readonly Markov.MarkovChain<char> Chain = null;

        public MarkovChain(string[] InArray, int N, Random InRng)
        {
            this.Random = InRng;
            Chain = new Markov.MarkovChain<char>(N);

            foreach(var str in InArray)
            {
                Chain.Add(str);
            }
        }

        public string GetMarkovString()
        {
            string Out = "";

            foreach(var ch in Chain.Chain(Random))
            {
                Out += ch;
            }

            return Out;
        }
    }
}
