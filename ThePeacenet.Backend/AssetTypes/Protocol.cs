using Microsoft.Xna.Framework.Content;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using ThePeacenet.Backend.Data;

namespace ThePeacenet.Backend.AssetTypes
{
    public class Protocol : Asset
    {
        internal Protocol(string id, string name, string summary, int port, bool isDefault, ComputerType targetComputerType, ContentManager content) : base(id, content)
        {
            Name = name;
            Summary = summary;
            Port = port;
            IsDefault = isDefault;
            TargetComputerType = targetComputerType;
        }

        public string Name { get; }
        public string Summary { get; }
        public int Port { get; }
        public bool IsDefault { get; }
        public ComputerType TargetComputerType { get; }
    }

    public class ProtocolData : AssetBuilder<Protocol>
    {
        public string Id { get; set; }
        public string Name { get; set; }
        public string Summary { get; set; }
        public int Port { get; set; }

        [ContentSerializer(Optional = true)]
        public bool IsDefault { get; set; } = false;

        [ContentSerializer(Optional = true)]
        public ComputerType TargetComputerType { get; set; } = ComputerType.Any;

        public override Protocol Build(ItemContainer items)
        {
            return new Protocol(
                    Id,
                    Name,
                    Summary,
                    Port,
                    IsDefault,
                    TargetComputerType,
                    items.Content
                );
        }
    }
}
