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

        public void Clean()
        {
            Console.WriteLine("Cleaning save file: Removing NPCs");
            while (Characters.Any(x => x.IdentityType != IdentityType.Player))
            {
                var character = Characters.First(x => x.IdentityType != IdentityType.Player);
                Characters.Remove(character);
            }

            Console.WriteLine("Cleaning save file: Removing orphaned computers...");
            while(Computers.Any(x=>x.OwnerType != IdentityType.None && !Characters.Any(y=>y.Computers.Contains(x.Id))))
            {
                var computer = Computers.First(x => x.OwnerType != IdentityType.None && !Characters.Any(y => y.Computers.Contains(x.Id)));
                Computers.Remove(computer);
            }

            Console.WriteLine("Cleaning save file: Removing orphaned entity map positions...");
            while (EntityPositions.Any(x => !Characters.Any(y => y.Id == x.Id)))
            {
                EntityPositions.Remove(EntityPositions.First(x => !Characters.Any(y => y.Id == x.Id)));
            }

            Console.WriteLine("Cleaning save file: Removing unlinked entity relationships...");
            while(CharacterRelationships.Any(x=>!(Characters.Any(y=>y.Id == x.FirstId) && Characters.Any(y=>y.Id == x.SecondId))))
            {
                CharacterRelationships.Remove(CharacterRelationships.First(x => !(Characters.Any(y => y.Id == x.FirstId) && Characters.Any(y => y.Id == x.SecondId))));
            }

            while (AdjacentNodes.Any(x => !(Characters.Any(y => y.Id == x.NodeA) && Characters.Any(y => y.Id == x.NodeB))))
            {
                AdjacentNodes.Remove(AdjacentNodes.First(x => !(Characters.Any(y => y.Id == x.NodeA) && Characters.Any(y => y.Id == x.NodeB))));
            }

            while(PlayerDiscoveredNodes.Any(x=>!Characters.Any(y=>y.Id == x)))
            {
                PlayerDiscoveredNodes.Remove(PlayerDiscoveredNodes.First(x => !Characters.Any(y => y.Id == x)));
            }

            Console.WriteLine("Cleaning save file: Removing invalid IP addresses...");
            while (ComputerIPMap.Any(x => !Computers.Any(y => y.Id == x.Value)))
            {
                ComputerIPMap.Remove(ComputerIPMap.First(x => !Computers.Any(y => y.Id == x.Value)).Key);
            }

            Console.WriteLine("Cleaning save file: Removing invalid domain names...");
            while (DomainNameMap.Any(x => !ComputerIPMap.ContainsKey(x.Value)))
            {
                ComputerIPMap.Remove(DomainNameMap.First(x => !ComputerIPMap.ContainsKey(x.Value)).Key);
            }

            Console.WriteLine("Cleaning save file: Done.");
        }

        public static SaveGame FromStream(Stream stream)
        {
            using (var gzip = new GZipStream(stream, CompressionMode.Decompress))
            {
                try
                {
                    var save = ReadObject<SaveGame>(gzip);
                    if (save.IsNewGame)
                        save.Clean();
                    return save;
                }
                catch (Exception ex)
                {
                    throw new CorruptedSaveGameException(ex);
                }
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

    public class CorruptedSaveGameException : Exception
    {
        public CorruptedSaveGameException(Exception innerException) : base("The save file is corrupted and therefore cannot be loaded.", innerException)
        {

        }
    }
}
