using Microsoft.Xna.Framework.Content;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace ThePeacenet.Backend.AssetTypes
{
    public class PayloadAsset : Asset
    {
        public PayloadAsset(PayloadData data, ContentManager content) : base(data.Id, content)
        {
            Name = data.Name;
            Description = data.Description;
            UnlockedByDefault = data.UnlockedByDefault;
            Payload = data.Payload;
        }

        public string Name { get; }
        public string Description { get; }
        public bool UnlockedByDefault { get; }
        public Payload Payload { get; }
    }

    public class PayloadData : AssetBuilder<PayloadAsset>
    {
        public string Id { get; set; }
        public string Name { get; set; }

        [ContentSerializer(Optional = true)]
        public string Description { get; set; }

        [ContentSerializer(Optional = true)]
        public bool UnlockedByDefault { get; set; } = true;

        public Payload Payload { get; set; }

        public override PayloadAsset Build(ItemContainer items)
        {
            return new PayloadAsset(this, items.Content);
        }
    }
}
