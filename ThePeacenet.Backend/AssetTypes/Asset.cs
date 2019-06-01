using Microsoft.Xna.Framework.Content;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace ThePeacenet.Backend.AssetTypes
{
    public class Asset
    {
        internal Asset(string id, ContentManager content)
        {
            Id = id;
            Content = content;
        }

        public string Id { get; }
        public ContentManager Content { get; }
    }
}
