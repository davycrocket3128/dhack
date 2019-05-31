using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using ThePeacenet.Backend.AssetAttributes;
using ThePeacenet.Backend.Data;
using ThePeacenet.Backend.Manual;
using ThePeacenet.Backend.Shell;

namespace ThePeacenet.Backend.AssetTypes
{
    public class CommandAsset : ManualPageAsset
    {
        public List<string> Usages { get; set; } = new List<string>();
        public bool UnlockedByDefault { get; set; } = true;
        public bool ShowInHelp { get; set; } = true;
        public RamUsage RamUsage { get; set; } = RamUsage.Low;
        public Type CommandType { get; set; }

        public CommandAsset(string name) : base(name)
        {
            Name = name;
        }

        protected override void BuildManualPage(ManualPageBuilder builder)
        {
            builder.Name = builder.Id;

            StringBuilder usageBuilder = new StringBuilder();
            
            foreach(var usage in this.Usages)
            {
                usageBuilder.AppendLine(string.Format(" - {0} {1}", builder.Id, usage));
            }

            if(Usages.Count == 0)
            {
                usageBuilder.AppendLine(string.Format(" - {0}", builder.Id));
            }

            builder.SetMetadata("Syntax", usageBuilder.ToString());
        }

        public static CommandAsset FromCommand(Type type)
        {
            if(!typeof(Command).IsAssignableFrom(type))
            {
                throw new InvalidOperationException("Input type must be a type of command.");
            }

            if(type.IsAbstract)
            {
                throw new InvalidOperationException("Type must not be abstract.");
            }

            var asset = new CommandAsset(type.Name.ToLower());

            asset.UnlockedByDefault = type.GetCustomAttributes(false).Any(x => x is UnlockedByDefaultAttribute);
            asset.Description = (type.GetCustomAttributes(false).Any(x => x is DescriptionAttribute) ? (type.GetCustomAttributes(false).First(x => x is DescriptionAttribute) as DescriptionAttribute).Description : "");
            asset.RamUsage = (type.GetCustomAttributes(false).Any(x => x is RamUsageAttribute) ? (type.GetCustomAttributes(false).First(x => x is RamUsageAttribute) as RamUsageAttribute).RamUsage : RamUsage.Low);

            foreach (UsageAttribute usage in type.GetCustomAttributes(false).Where(x=>x is UsageAttribute))
            {
                asset.Usages.Add(usage.Usage);
            }

            asset.CommandType = type;

            return asset;
        }
    }
}
