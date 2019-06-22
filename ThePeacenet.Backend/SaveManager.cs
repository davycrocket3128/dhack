using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using LiteDB;
using ThePeacenet.Backend.AssetTypes;
using ThePeacenet.Backend.Data;

namespace ThePeacenet.Backend
{
    public class SaveManager
    {
        private LiteDatabase _profileDatabase = null;
        private LiteCollection<SaveInfo> _saves = null;
        private WorldState _world = null;
        private SaveInfo _currentInfo = null;
        private SaveGame _currentSave = null;

        public IEnumerable<SaveInfo> AvailableAgents => _saves.FindAll();
        public string SavesDirectory => Path.Combine(_world.GameDataPath, "saves");
        internal SaveGame CurrentSave => _currentSave;

        public SaveManager(WorldState world)
        {
            _world = world;
            Console.WriteLine("Opening game profile...");
            _profileDatabase = new LiteDatabase(Path.Combine(_world.GameDataPath, "profile.ldb"));
            _saves = _profileDatabase.GetCollection<SaveInfo>("saves");

            CleanDatabase();
        }

        public void NewGame(string playerName)
        {
            if (_currentInfo != null || _currentSave != null)
                throw new InvalidOperationException("A game is already in progress");

            _saves.Insert(_currentInfo = new SaveInfo
            {
                LastPlayed = DateTime.Now,
                Name = playerName,
                Path = playerName.ToIdentifier() + "_" + _saves.Count() + ".bps"
            });

            _currentSave = new SaveGame();

            var playerPC = new Computer
            {
                Id = 0,
                Users = new List<User>
                {
                    new User {
                         Username = "root",
                         Password = "",
                         UserType = UserType.Admin
                    },
                    new User
                    {
                        Username = playerName.ToIdentifier(),
                        Password = "",
                        UserType = UserType.Sudoer
                    }
                }
            };

            CurrentSave.Computers.Add(playerPC);

            var playerIdentity = new Identity
            {
                Id = 0,
                Computers = new List<int>
                 {
                     playerPC.Id
                 },
                Alias = "",
                IdentityType = IdentityType.Player,
                IsMissionImportant = false,
                Name = playerName,
                Reputation = 0,
                Skill = 0
            };

            CurrentSave.Characters.Add(playerIdentity);

            CurrentSave.PlayerCharacterID = 0;
            CurrentSave.PlayerUserID = 1;


            Save();
        }

        public void UnloadGame(bool save = true)
        {
            if(_currentInfo != null && _currentSave != null)
            {
                if(save)
                {
                    Save();
                }

                _currentSave = null;
                _currentInfo = null;
            }
        }

        public void Save()
        {
            if (_currentInfo == null || _currentSave == null)
                throw new InvalidOperationException("A game is not currently loaded.");

            Console.WriteLine("Saving the game to disk...");

            using (var stream = File.Open(GetTruePath(_currentInfo.Path), System.IO.FileMode.OpenOrCreate))
            {
                _currentSave.SaveToStream(stream);
            }

            Console.WriteLine("Done.");
        }

        private SaveGame LoadFile(SaveInfo info)
        {
            if (info == null) throw new ArgumentNullException(nameof(info));

            if (!SaveExists(info.Path)) throw new InvalidOperationException("The save file referenced by the specified save entry doesn't exist.");

            string path = GetTruePath(info.Path);

            using (var stream = File.OpenRead(path))
            {
                return SaveGame.FromStream(stream);
            }
        }

        public void Load(SaveInfo info)
        {
            if (_currentInfo != null || _currentSave != null)
                throw new InvalidOperationException("A game is already in-progress, you can't load another one.");

            _currentInfo = info ?? throw new ArgumentNullException(nameof(info));

            if(SaveExists(_currentInfo.Path))
            {
                try
                {
                    _currentSave = LoadFile(_currentInfo) ?? throw new InvalidOperationException("An unknown error occurred loading the save file.");
                }
                catch(Exception ex)
                {
                    _currentInfo = null;
                    _currentSave = null;
                    throw ex;
                }
            }
            else
            {
                _currentInfo = null;
                throw new InvalidOperationException("The save file referenced by this save entry doesn't exist.");
            }

            _currentInfo.LastPlayed = DateTime.Now;
            _saves.Update(_currentInfo);
        }

        private void CleanDatabase()
        {
            if (!Directory.Exists(SavesDirectory))
                Directory.CreateDirectory(SavesDirectory);

            Console.WriteLine("Cleaning profile...");

            Console.WriteLine("Deleted {0} save records with non-existent files.", _saves.Delete(x => !SaveExists(x.Path)));
        }

        private string GetTruePath(string path)
        {
            return Path.Combine(SavesDirectory, path);
        }

        public bool SaveExists(string path)
        {
            return File.Exists(GetTruePath(path));
        }
    }

    public class SaveInfo
    {
        public Guid Id { get; set; } = Guid.NewGuid();
        public string Name { get; set; }
        public DateTime LastPlayed { get; set; }
        public string Path { get; set; }
    }
}
