using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace ThePeacenet.Backend.Data
{
    public enum RelationshipType
    {
        Friend,
        Enemy
    }

    public class CharacterRelationship
    {
        public int FirstId { get; set; }
        public int SecondId { get; set; }
        public RelationshipType RelationshipType { get; set; }
    }
}
