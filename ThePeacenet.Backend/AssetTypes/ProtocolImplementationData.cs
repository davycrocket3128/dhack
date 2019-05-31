using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace ThePeacenet.Backend.AssetTypes
{
    public class ProtocolImplementation : Asset
    {
        internal ProtocolImplementation(string id, string name, int minSkillLevel, Protocol protocol) : base(id)
        {
            Name = name;
            MinimumSkillLevel = minSkillLevel;
            Protocol = protocol;
        }

        public string Name { get; private set; }
        public Protocol Protocol { get; private set; }
        public int MinimumSkillLevel { get; private set; }
    }

    public class ProtocolImplementationData : AssetBuilder<ProtocolImplementation>
    {
        public string Id { get; set; }
        public string Name { get; set; }
        public int MinimumSkillLevel { get; set; }
        public string ProtocolId { get; set; }

        public override ProtocolImplementation Build(ItemContainer items)
        {
            return new ProtocolImplementation(Id, Name, MinimumSkillLevel, items.GetItem<Protocol>(ProtocolId));
        }
    }
}
