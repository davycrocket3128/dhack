using Microsoft.Xna.Framework.Content;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace ThePeacenet.Backend.AssetTypes
{
    public enum MarkovTrainingDataUsage
    {
        MaleFirstNames,
        FemaleFirstNames,
        LastNames,
        Hostnames,
        FileNames,
        Usernames
    }

    public class MarkovTrainingDataAsset : Asset
    {
        public MarkovTrainingDataAsset(string id, ContentManager content, MarkovTrainingDataUsage usage, string[] data) : base(id, content)
        {
            Usage = usage;
            TrainingData = data;
        }

        public MarkovTrainingDataUsage Usage { get; }
        public string[] TrainingData { get; }
    }

    public class MarkovTrainingData : AssetBuilder<MarkovTrainingDataAsset>
    {
        public MarkovTrainingDataUsage Usage { get; set; } = MarkovTrainingDataUsage.MaleFirstNames;

        [ContentSerializer(CollectionItemName = "Data")]
        public List<string> Data { get; set; } = new List<string>();

        public override MarkovTrainingDataAsset Build(ItemContainer items)
        {
            return new MarkovTrainingDataAsset(Guid.NewGuid().ToString(), items.Content, Usage, Data.ToArray());
        }
    }
}
