using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace ThePeacenet.Backend.Data
{
    public enum IdentityType
    {
        None,
        Player,
        NPC,
        Story
    }

    [Serializable]
    public class Identity
    {
        public int Id { get; set; }
        public string Name { get; set; }
        public string Email { get; set; }
        public string Alias { get; set; }
        public int Skill { get; set; }
        public float Reputation { get; set; } = 0;
        public IdentityType IdentityType { get; set; }
        public bool IsMissionImportant { get; set; }
        public List<int> Computers { get; set; } = new List<int>();
    }
}
