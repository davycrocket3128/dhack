using ThePeacenet.Backend.AssetTypes;
using ThePeacenet.Backend.Data;

namespace ThePeacenet.Backend.OS
{
    public class ServiceInfo
    {
        private FirewallRule _fw = null;
        private SystemContext _system = null;

        public ServiceInfo(SystemContext system, FirewallRule fwRule)
        {
            _fw = fwRule;
            _system = system;
        }

        public int Port => _fw.Port;
        public bool IsFiltered => _fw.IsFiltered;

        public ProtocolImplementation Implementation => _system.Peacenet.Items.GetItem<ProtocolImplementation>(_fw.Service);
        public Protocol Protocol => Implementation.Protocol;

        public string ProtocolName => Protocol.Name;

        public void Crash()
        {
            _fw.IsCrashed = true;
        }
    }
}
