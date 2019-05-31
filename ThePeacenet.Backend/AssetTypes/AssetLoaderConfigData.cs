using Microsoft.Xna.Framework.Content;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace ThePeacenet.Backend.AssetTypes
{
    public class AssetLoaderConfigData
    {
        [ContentSerializer(CollectionItemName = "Directory")]
        public List<string> DirectoryBlacklist { get; set; } = new List<string>();
    }
}
