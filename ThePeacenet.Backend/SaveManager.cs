using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using LiteDB;
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
        
        public string SavesDirectory => Path.Combine(_world.GameDataPath, "saves");

        public SaveManager(WorldState world)
        {
            _world = world;
            Console.WriteLine("Opening game profile...");
            _profileDatabase = new LiteDatabase(Path.Combine(_world.GameDataPath, "profile.ldb"));
            _saves = _profileDatabase.GetCollection<SaveInfo>("saves");

            CleanDatabase();
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
        }

        private SaveGame LoadFile(SaveInfo info)
        {
            if (info == null) throw new ArgumentNullException(nameof(info));

            if (!SaveExists(info.Path)) throw new InvalidOperationException("The save file referenced by the specified save entry doesn't exist.");

            string path = GetTruePath(info.Path);

            return null;
        }

        public void Load(SaveInfo info)
        {
            if (_currentInfo != null || _currentSave != null)
                throw new InvalidOperationException("A game is already in-progress, you can't load another one.");

            _currentInfo = info ?? throw new ArgumentNullException(nameof(info));

            if(SaveExists(_currentInfo.Path))
            {
                _currentSave = LoadFile(_currentInfo) ?? throw new InvalidOperationException("An unknown error occurred loading the save file.");
            }
            else
            {
                _currentInfo = null;
                throw new InvalidOperationException("The save file referenced by this save entry doesn't exist.");
            }
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
        public string Name { get; set; }
        public DateTime LastPlayed { get; set; }
        public string Path { get; set; }
    }
}
