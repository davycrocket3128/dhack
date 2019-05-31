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
        internal Protocol(string id, string name, string summary, string description, int port, bool isDefault, ComputerType targetComputerType) : base(id)
        {
            Name = name;
            Summary = summary;
            Description = description;
            Port = port;
            IsDefault = isDefault;
            TargetComputerType = targetComputerType;
        }

        public string Name { get; }
        public string Summary { get; }
        public string Description { get; }
        public int Port { get; }
        public bool IsDefault { get; }
        public ComputerType TargetComputerType { get; }
    }

    public class ProtocolData : AssetBuilder<Protocol>
    {
        public string Id { get; set; }
        public string Name { get; set; }
        public string Summary { get; set; }
        public string Description { get; set; }
        public int Port { get; set; }
        public bool IsDefault { get; set; }
        public ComputerType TargetComputerType { get; set; }

        public override Protocol Build(ItemContainer items)
        {
            return new Protocol(
                    Id,
                    Name,
                    Summary,
                    Description,
                    Port,
                    IsDefault,
                    TargetComputerType
                );
        }
    }
}
