using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace ThePeacenet.Backend.Data
{
    class SaveGame
    {
        public List<string> CompletedMissions { get; set; } = new List<string>();
        public Dictionary<string, int> GameStats { get; set; } = new Dictionary<string, int>();
        public bool IsNewGame { get; set; } = true;
        public int PlayerCharacterID { get; set; } = 0;
        public Dictionary<string, int> StoryCharacterIDs { get; set; } = new Dictionary<string, int>();
        public int PlayerUserID { get; set; } = 0;
        public string GameTypeName { get; set; }
        public List<CharacterRelationship> CharacterRelationships { get; set; } = new List<CharacterRelationship>();
        public List<Computer> Computers { get; set; } = new List<Computer>();
        public List<EntityPosition> EntityPositions { get; set; } = new List<EntityPosition>();
        public List<Identity> Characters { get; set; } = new List<Identity>();
        public float EpochTime { get; set; } = 43200;
        public Dictionary<string, bool> Booleans { get; set; } = new Dictionary<string, bool>();
        public Dictionary<string, string> DomainNameMap { get; set; } = new Dictionary<string, string>();
        public Dictionary<string, int> ComputerIPMap { get; set; } = new Dictionary<string, int>();
        public List<Email> EmailMessages { get; set; } = new List<Email>();
        public List<AdjacentNode> AdjacentNodes { get; set; } = new List<AdjacentNode>();
        public int WorldSeed { get; set; } = -1;
        public List<int> PlayerDiscoveredNodes { get; set; } = new List<int>();
    }
}
