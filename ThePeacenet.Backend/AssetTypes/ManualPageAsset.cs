using Microsoft.Xna.Framework.Content;
using Microsoft.Xna.Framework.Content.Pipeline;
using Microsoft.Xna.Framework.Content.Pipeline.Serialization.Compiler;
using Newtonsoft.Json;
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using ThePeacenet.Backend.Manual;

namespace ThePeacenet.Backend.AssetTypes
{
    public class ManualPageAsset : Asset
    {
        public string Name { get; set; }
        public string Description { get; set; }

        public ManualPageAsset(string id, ContentManager content) : base(id, content)
        {

        }

        public ManualPageAsset(string id, string name, string description, ContentManager content) : this(id, content)
        {
            this.Name = name;
            this.Description = description;
        }

        protected virtual void BuildManualPage(ManualPageBuilder builder)
        {

        }

        public ManualPage GenerateManualPage()
        {
            var builder = new ManualPageBuilder();
            builder.Id = Id;
            builder.Name = Name;
            builder.Description = Description;
            builder.ItemType = this.GetType().Name;

            this.BuildManualPage(builder);

            return builder.Page;
        }
    }
}
