using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace ThePeacenet.Backend.Data
{
    public enum RamAmount
    {
        // 256mb
        Level0,
        // 512mb
        Level1,
        // 1024mb
        Level2,
        // 2048mb
        Level3,
        // 4096mb
        Level4,
        // 8192mb
        Level5,
        // 16384mb
        Level6
    }

    public class Computer
    {
        public int Id { get; set; }
        public List<string> Commands { get; set; } = new List<string>();
        public List<User> Users { get; set; } = new List<User>();
        public RamAmount RamAmount { get; set; } = RamAmount.Level0;
    }
}
