using System;
using System.Collections.Generic;
using System.IO;
using System.IO.Compression;
using System.Linq;
using System.Runtime.Serialization.Formatters.Binary;
using System.Text;
using System.Threading.Tasks;
using ThePeacenet.Backend.AssetTypes;

namespace ThePeacenet.Backend.Data
{
    [Serializable]
    public class SaveGame
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

        private static readonly byte[] magic = Encoding.UTF8.GetBytes("kY1n"); // Fuck off.  I wrote this.  I can use whatever magic number I want. Even if it references something only related to me and not at all to the data format. - Michael

        public static SaveGame FromStream(Stream stream)
        {
            using (var gzip = new GZipStream(stream, CompressionMode.Decompress))
            {
                return ReadObject<SaveGame>(gzip);
            }
        }

        public void SaveToStream(Stream stream)
        {
            using (var gzip = new GZipStream(stream, CompressionMode.Compress))
            {
                WriteObject(gzip, this);
            }
        }

        public static void WriteObject(Stream writer, object obj)
        {
            var bf = new BinaryFormatter();
            bf.Serialize(writer, obj);
        }

        public static T ReadObject<T>(Stream stream)
        {
            var bf = new BinaryFormatter();
            return (T)bf.Deserialize(stream);
        }
    }
}
