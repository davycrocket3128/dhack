﻿using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using ThePeacenet.Backend.FileSystem;

namespace ThePeacenet.Backend.Data
{
    public enum ComputerType
    {
        Personal,
        Hub,
        EmailServer,
        PeacenetSite
    }

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

    [Serializable]
    public class Computer
    {
        public int Id { get; set; }
        public List<string> Commands { get; set; } = new List<string>();
        public List<User> Users { get; set; } = new List<User>();
        public List<Folder> Folders { get; set; } = new List<Folder>();
        public List<FileRecord> Files { get; set; } = new List<FileRecord>();
        public List<TextFile> TextFiles { get; set; } = new List<TextFile>();
        public RamAmount RamAmount { get; set; } = RamAmount.Level0;
        public IdentityType OwnerType { get; set; } = IdentityType.None;
        public List<FirewallRule> Services { get; set; } = new List<FirewallRule>();
        public ComputerType ComputerType { get; set; }
    }

    [Serializable]
    public class FirewallRule
    {
        public int Port { get; set; }
        public bool IsFiltered { get; set; } = false;
        public string Service { get; set; }
        public bool IsCrashed { get; set; } = false;
    }
}
