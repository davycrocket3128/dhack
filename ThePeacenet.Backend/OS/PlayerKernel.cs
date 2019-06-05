using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using ThePeacenet.Backend.AssetTypes;
using ThePeacenet.Backend.Data;
using ThePeacenet.Backend.FileSystem;
using ThePeacenet.Backend.Shell;

namespace ThePeacenet.Backend.OS
{
    public class PlayerKernel : IKernel, IKernelFileSystem
    {
        private WorldState _worldState = null;
        private readonly int _characterId = 0;
        private readonly int _computerId = 0;
        private List<UserLand> _users = new List<UserLand>();
        public WorldState WorldState => _worldState;

        public Computer Computer => WorldState.GetComputer(_computerId);
        public Identity Identity => WorldState.GetIdentity(_characterId);

        public IEnumerable<FileRecord> FileRecords => Computer.Files;

        public IEnumerable<CommandAsset> Commands => WorldState.GetAvailableCommands(Computer);

        public List<FileRecord> Records => Computer.Files;
        public IEnumerable<Program> Programs => WorldState.Items.GetAll<Program>();

        public int NextRecordId => (Records.Count > 0) ? Records.Max(x => x.Id) + 1 : 0;

        public int NextTextFileId => (TextFiles.Count > 0) ? TextFiles.Max(x => x.Id) + 1 : 0;

        public List<Folder> Folders => Computer.Folders;

        public List<TextFile> TextFiles => Computer.TextFiles;

        public int NextFolderId => (Folders.Count > 0) ? Folders.Max(x=>x.Id) + 1 : 0;

        public PlayerKernel(WorldState world, int identity, int computer)
        {
            _worldState = world;
            _characterId = identity;
            _computerId = computer;
        }

        public IUserLand GetUserLand(int userId)
        {
            if (_users.Any(x => x.UserID == userId))
                return _users.First(x => x.UserID == userId);

            var userLand = new UserLand(this, userId);

            _users.Add(userLand);

            return userLand;
        }

        public User GetUser(int uid)
        {
            if (uid < 0 || uid >= Computer.Users.Count) throw new ArgumentOutOfRangeException(nameof(uid));

            return Computer.Users[uid];
        }

        public bool Execute(IUserLand user, string program, out IProcess process)
        {
            Program programAsset = _worldState.Items.GetItem<Program>(program);
            if(programAsset != null)
            {
                process = _worldState.GuiBuilder.BuildProgram(programAsset);
                return true;
            }

            var commands = Commands;

            if(commands.Any(x=>x.Id == program))
            {
                process = (IProcess)Activator.CreateInstance(commands.First(x => x.Id == program).CommandType, null);

                (process as Command).RamUsage = commands.First(x => x.Id == program).RamUsage;

                return true;
            }

            process = null;
            return false;
        }

        public Folder GetFolder(string path)
        {
            // Is there a root folder?
            if(!Folders.Any(x=>x.Parent == -1))
            {
                // Create it.
                Folders.Add(new Folder
                {
                    Id = NextFolderId,
                    Name = "<root>",
                    Parent = -1,
                    IsReadOnly = false
                });
            }

            // Get the root directory.
            Folder Root = Folders.First(x => x.Parent == -1);

            // Get the absolute path parts.
            string[] parts = GetPathParts(path);

            // If the part list is empty return the root directory.
            if (parts.Length == 0) return Root;

            // Go through each part and try to find a subdirectory with the matching name.
            foreach(var part in parts)
            {
                // Are there any subfolders in the current folder where the name matches?
                if(Folders.Where(x=>Root.SubFolders.Contains(x.Id)).Any(x=>x.Name == part))
                {
                    // Traverse to that root.
                    Root = Folders.Where(x => Root.SubFolders.Contains(x.Id)).First(x => x.Name == part);
                }
                else
                {
                    // Directory doesn't exist.
                    return null;
                }
            }

            // Return where we ended up.
            return Root;
        }

        public FileRecord GetFile(string path)
        {
            // Get the absolute path.
            string abs = ResolveToAbsolute(path);

            // If we end up at a directory then we're going to return null.
            if (GetFolder(abs) != null) return null;

            // Get the folder of its parent - if that's null then the directory wasn't found so there's no file.
            Folder parentFolder = GetFolder(abs + "/..");

            if (parentFolder == null) return null;

            // Get the filename from the absolute path.
            string filename = GetPathParts(abs).Last();

            // Return a file with the matching name.
            return Records.Where(x => parentFolder.Files.Contains(x.Id)).FirstOrDefault(x => x.Name == filename);
        }

        public string[] GetPathParts(string path)
        {
            return ResolveToAbsolute(path).Split(new[] { '/' }, StringSplitOptions.RemoveEmptyEntries);
        }

        public bool CreateFile(string path, FileRecordType type, int id)
        {
            if (GetFile(path) != null) return false;

            if (GetFolder(path) != null) return false;

            string abs = ResolveToAbsolute(path);

            Folder parent = GetFolder(abs + "/..");

            if (parent == null) return false;

            int fileId = NextRecordId;

            parent.Files.Add(fileId);

            Records.Add(new FileRecord
            {
                Id = fileId,
                Name = GetPathParts(abs).Last(),
                ContentId = id,
                RecordType = type
            });

            return true;
        }

        public string ResolveToAbsolute(string path)
        {
            Stack<string> stack = new Stack<string>();

            string[] split = path.Split('/');

            foreach(var part in split)
            {
                if (string.IsNullOrWhiteSpace(part)) continue;

                if (part == ".") continue;

                if(part == ".." && stack.Count > 0)
                {
                    stack.Pop();
                    continue;
                }

                stack.Push(part);
            }

            if (stack.Count == 0) return "/";

            string p = "";
            
            while(stack.Count > 0)
            {
                p = "/" + stack.Pop() + p;
            }

            return p;
        }

        public bool CreateDirectory(string path)
        {
            if (GetFolder(path) != null) return false;
            if (GetFile(path) != null) return false;

            string abs = ResolveToAbsolute(path);

            var parent = GetFolder(abs + "/..");
            if (parent == null) return false;

            string name = GetPathParts(abs).Last();

            int folderId = NextFolderId;

            Folders.Add(new Folder
            {
                Id = folderId,
                Name = name,
                IsReadOnly = false,
                Parent = parent.Id
            });

            parent.SubFolders.Add(folderId);
            return true;
        }
    }

    public class UserLand : IUserLand, IFileSystem
    {
        private PlayerKernel _kernel = null;
        private readonly int _uid = 0;

        public IEnumerable<Program> Programs => _kernel.Programs;

        internal int UserID => _uid;

        public UserLand(PlayerKernel kernel, int uid)
        {
            _kernel = kernel;
            _uid = uid;

            CreateDirectory("/home");
            CreateDirectory(HomeFolder);
            CreateDirectory(HomeFolder + "/Desktop");
            CreateDirectory(HomeFolder + "/Documents");
            CreateDirectory(HomeFolder + "/Music");
            CreateDirectory(HomeFolder + "/Pictures");
            CreateDirectory(HomeFolder + "/Videos");
            CreateDirectory(HomeFolder + "/Downloads");
        }

        public User User => _kernel.GetUser(_uid);

        public string Username => User.Username;

        public string Hostname => "localhost";

        public string HomeFolder => (IsAdmin) ? "/root" : "/home/" + Username;

        public string IdentityName => _kernel.Identity.Name;

        public IFileSystem FileSystem => this;

        public string EmailAddress => _kernel.Identity.Email;

        public bool IsAdmin => User.UserType == UserType.Admin;

        public ConsoleColor UserColor => User.UserColor;

        public IEnumerable<CommandAsset> Commands => _kernel.Commands;

        public bool Execute(string program, out IProcess process)
        {
            return _kernel.Execute(this, program, out process);
        }

        public bool CreateDirectory(string path)
        {
            return _kernel.CreateDirectory(path);
        }

        public bool DirectoryExists(string path)
        {
            return _kernel.GetFolder(path) != null;
        }

        public bool FileExists(string path)
        {
            return _kernel.GetFile(path) != null;
        }

        public void DeleteFolderIdRecursive(Folder folder)
        {
            while(folder.SubFolders.Count > 0)
            {
                DeleteFolderIdRecursive(_kernel.Folders.First(x => x.Id == folder.SubFolders[0]));
                folder.SubFolders.RemoveAt(0);
            }

            _kernel.Folders.RemoveAll(x => x.Id == folder.Id);

            _kernel.Folders.ForEach(x =>
            {
                x.SubFolders.RemoveAll(y => y == folder.Id);
            });

            while(folder.Files.Count > 0)
            {
                var file = _kernel.Records.First(x => x.Id == folder.Files[0]);
                folder.Files.RemoveAt(0);
                
                if(!_kernel.Folders.Any(x=>x.Files.Contains(file.Id)))
                {
                    _kernel.Records.RemoveAll(x => x.Id == file.Id);

                    if(file.RecordType == FileRecordType.Text)
                    {
                        if(!_kernel.Records.Any(x=>x.RecordType == FileRecordType.Text && x.ContentId == file.ContentId))
                        {
                            _kernel.TextFiles.RemoveAll(x => x.Id == file.ContentId);
                        }
                    }
                }
            }
        }

        public bool Delete(string path, bool recursive = false)
        {
            if(DirectoryExists(path))
            {
                var folder = _kernel.GetFolder(path);

                if (folder.Parent == -1) return false;

                if(folder.SubFolders.Count > 0 || folder.Files.Count > 0)
                {
                    if(recursive)
                    {
                        DeleteFolderIdRecursive(folder);
                        return true;
                    }
                    return false;
                }

                _kernel.Folders.ForEach(x =>
                {
                    x.SubFolders.RemoveAll(y => y == x.Id);
                });

                _kernel.Folders.RemoveAll(x => x.Id == folder.Id);
                return true;
            }
            else if(FileExists(path))
            {
                var file = GetFileRecord(path);

                _kernel.Records.RemoveAll(x => x.Id == file.Id);

                if(file.RecordType == FileRecordType.Text)
                {
                    if(!_kernel.Records.Any(x=>x.ContentId == file.ContentId))
                    {
                        _kernel.TextFiles.RemoveAll(x => x.Id == file.ContentId);
                    }
                }

                _kernel.Folders.ForEach(x =>
                {
                    x.Files.RemoveAll(y => y == file.Id);
                });

                return true;
            }

            return false;
        }

        public string[] GetDirectories(string path)
        {
            if(DirectoryExists(path))
            {
                var folder = _kernel.GetFolder(path);

                return _kernel.Folders.Where(x => folder.SubFolders.Contains(x.Id)).Select(x => x.Name).ToArray();
            }
            return new string[0];
        }

        public string[] GetFiles(string path)
        {
            if (DirectoryExists(path))
            {
                var folder = _kernel.GetFolder(path);

                return _kernel.Records.Where(x => folder.Files.Contains(x.Id)).Select(x => x.Name).ToArray();
            }
            return new string[0];
        }

        public void WriteText(string path, string text)
        {
            var record = GetFileRecord(path);
            if (record != null && record.RecordType == FileRecordType.Text)
            {
                _kernel.TextFiles.First(x => x.Id == record.ContentId).Content = text;
                return;
            }
            else if (record != null) return;

            if (DirectoryExists(path)) return;

            int textId = _kernel.NextTextFileId;

            _kernel.TextFiles.Add(new TextFile
            {
                Id = textId,
                Content = text
            });

            SetFileRecord(path, FileRecordType.Text, textId);
        }

        public string ReadText(string path)
        {
            FileRecord record = GetFileRecord(path);
            if(record != null && record.RecordType == FileRecordType.Text)
            {
                return _kernel.TextFiles.First(x => x.Id == record.ContentId).Content;
            }
            return "";
        }

        public bool MoveFile(string sourcePath, string destinationPath, bool overwrite)
        {
            throw new NotImplementedException();
        }

        public bool MoveFolder(string sourcePath, string destinationPath, bool overwrite)
        {
            throw new NotImplementedException();
        }

        public bool CopyFile(string sourcePath, string destinationPath, bool overwrite)
        {
            throw new NotImplementedException();
        }

        public void SetFileRecord(string path, FileRecordType type, int id)
        {
            FileRecord record = GetFileRecord(path);
            if(record != null)
            {
                record.RecordType = type;
                record.ContentId = id;
            }
            else
            {
                _kernel.CreateFile(path, type, id);
            }
        }

        public FileRecord GetFileRecord(string path)
        {
            return _kernel.GetFile(path);
        }

        public string GetAbsolutePath(string path)
        {
            return _kernel.ResolveToAbsolute(path);
        }
    }
}
