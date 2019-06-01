using Microsoft.Xna.Framework.Content;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using ThePeacenet.Backend.Data;

namespace ThePeacenet.Backend.AssetTypes
{
    public class Program : Asset
    {
        public Program(ProgramData data, ContentManager content) : base(data.Name.ToLower().Replace(" ", "_"), content)
        {
            this.Name = data.Name;
            this.Description = data.Description;
            this.LauncherCategory = data.LauncherCategory;
            this.LauncherIcon = data.LauncherIcon;
            this.UnlockedByDefault = data.UnlockedByDefault;
            this.SingleInstance = data.IsSingleInstance;
            this.EnableMinimizeMaximize = data.WindowEnableMinMax;
            this.RamUsage = data.RamUsage;
            this.Gui = data.Gui;
        }

        public string Name { get; }
        public string Description { get; }
        public string LauncherCategory { get; }
        public string LauncherIcon { get; }
        public ProgramGui Gui { get; }
        public bool UnlockedByDefault { get; }
        public bool EnableMinimizeMaximize { get; }
        public bool SingleInstance { get; }
        public RamUsage RamUsage { get; }
    }

    public class ProgramData : AssetBuilder<Program>
    {
        public string Name { get; set; }
        public string Description { get; set; }
        public string LauncherCategory { get; set; }
        public string LauncherIcon { get; set; }
        
        [ContentSerializer(Optional = true)]
        public bool UnlockedByDefault { get; set; } = true;

        [ContentSerializer(Optional = true)]
        public bool WindowEnableMinMax { get; set; } = true;
        
        [ContentSerializer(Optional = true)]
        public bool IsSingleInstance { get; set; } = true;

        [ContentSerializer(CollectionItemName = "Extension", Optional = true)]
        public List<string> SupportedFileExtensions { get; set; } = new List<string>();

        [ContentSerializer(Optional = true)]
        public RamUsage RamUsage { get; set; } = RamUsage.Low;

        public ProgramGui Gui { get; set; }

        public override Program Build(ItemContainer items)
        {
            return new Program(this, items.Content);
        }
    }
}
