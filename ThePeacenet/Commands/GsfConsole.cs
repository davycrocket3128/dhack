using Microsoft.Xna.Framework;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using ThePeacenet.Backend.AssetAttributes;
using ThePeacenet.Backend.AssetTypes;
using ThePeacenet.Backend.OS;
using ThePeacenet.Backend.Shell;

namespace ThePeacenet.Commands
{
    [Description("The Gigasploit Framework Console.")]
    [Usage("<host>")]
    [UnlockedByDefault]
    public class GsfConsole : Shell
    {
        private float _lastStealthiness = 1;
        private bool _isPayloadActive = false;
        private string _enteredHostname = string.Empty;
        private SystemContext _remoteSysstem = null;
        private Exploit _selectedExploit = null;
        private PayloadAsset _selectedPayload = null;

        protected bool ShouldCrashService
        {
            get
            {
                float volatility = (float)this._selectedExploit.Volatility / 5.0f;
                float crashiness = volatility * 0.5f;
                float chance = (float)_rng.NextDouble();
                return (chance < crashiness);
            }
        }

        private Random _rng = new Random();

        protected override bool AllowPipes => false;
        protected override bool AllowRedirection => false;

        protected override string ShellPrompt
        {
            get
            {
                // The format string that is used to create the prompt text.
                string PromptFormat = "{0} ({1}/{2})> ";

                // The text that will be substituted into the above format string.
                string Host = _enteredHostname;
                string Exploit = "none";
                string Payload = "none";

                // Get the actual exploit name.
                if (_selectedExploit != null)
                {
                    Exploit = _selectedExploit.Name;
                }

                if (_selectedPayload != null)
                {
                    Payload = _selectedPayload.Name;
                }

                return string.Format(PromptFormat, Host, Exploit, Payload);
            }
        }

        public override void Update(float deltaSeconds)
        {
            /*
            if (!this->IsTutorialActive())
            {
                if (this->IsSet("gigasploit.firstScan"))
                {
                    this->ShowTutorialIfNotSet("tuts.gigasploit.analyze",
                        "Analyzing a port",
                        "While <ui>scanning</> a system will tell you the active <ui>ports</> of the system, if you'd like to see more details about a <ui>specific port</> you need to <ui>analyze</> it.\r\n\r\nThis can be done by running the <cmd>analyze <port></> command.\r\n\r\nThis will tell you more information about the port, such as what server software is running it and what <ui>exploits</> you can use.\r\n\r\n<bad>WARNING</>: Analyzing a port may result in your IP address being logged, thus decreasing your <ui>cover</>."
                    );
                    this->ShowTutorialIfNotSet("tuts.gigasploit.exploits",
                        "Exploits",
                        "An <ui>exploit</> is used to tell <ui>Gigasploit</> how to attack a specific <ui>vulnerability</> in a service to allow it to <bad>deploy a payload</>.\r\n\r\nRun the <cmd>exploits</> command to see all of the exploits you currently have available to you.  You will find more exploits as you hack more systems."
                    );
                    this->ShowTutorialIfNotSet("tuts.gigasploit.exploits.more",
                        "Exploits",
                        "To see more information about a particular <ui>exploit</>, you can use the <cmd>man <exploit></> command in another Terminal.\r\n\r\nSome exploits are more stable than others, and the more stable an exploit is, the less chance there is of it crashing the service it attacks.\r\n\r\nTo <ui>use</> an exploit, run <cmd>use exploit <exploit></>."
                    );
                    this->ShowTutorialIfNotSet("tuts.gigasploit.payloads",
                        "Payloads",
                        "<ui>Payloads</> are tiny programs that get deployed to the remote computer using <ui>exploits</>.  You can use any <ui>payload</> you'd like, but there are certain payloads that are better for different jobs.\r\n\r\nThe <ui>most common</> payload you will use is the <ui>reverse shell</> - which will connect to your computer and allow you to take control of the remote computer.\r\n\r\nOther <ui>payloads</> can be used to open <ui>additional services</>, bypass <bad>firewalls</>, or even <bad>crash</> a service or the whole system.\r\n\r\nTo see all of your available payloads, run the <cmd>payloads</> command.  You will find more payloads as you hack more systems.\r\n\r\nTo <ui>use</> a payload, run the <cmd>use payload <payload></> command."
                    );
                    this->ShowTutorialIfNotSet("tuts.gigasploit.hud",
                        "Gigasploit HUD",
                        "The <ui>remote system</>'s IP address is displayed in <ui>Gigasploit</>'s prompt.  Next to the IP address is the currently-selected <ui>exploit</> in yellow and <ui>payload</> in red.\r\n\r\nBoth an <ui>exploit</> and a <ui>payload</> need to be selected before you can <ui>attack</>."
                    );
                    this->ShowTutorialIfNotSet("tuts.gigasploit.attack",
                        "Launching the attack",
                        "Once you are ready to <bad>launch the attack</>, run the <cmd>attack <port></> command.  This will attempt to attack the specified port with the selected exploit and deploy the payload.\r\n\r\nIf the specified port is <bad>blocked</> by a <bad>firewall</>, or the service <bad>crashes</> because of the exploit, the attack will <bad>fail</> and you'll need to try a different attack.\r\n\r\nIf the attack <ui>succeeds</>, Gigasploit will exit and, if necessary, let the <ui>payload</> have access to your <ui>Terminal</>."
                    );
                }
            }*/

            // Assess the player's stealthiness.  If it's different than
            // before then we need to set the player's stealthiness value
            // in the world state if the new value is below the current.
            float stealthiness = this.AssessStealthiness();

            if (_lastStealthiness != stealthiness)
            {
                _lastStealthiness = stealthiness;

                /*
                if (stealthiness < this->GetUserContext()->GetStealthiness())
                {
                    this->GetUserContext()->SetStealthiness(stealthiness);
                }*/
            }

            // If the payload is NOT active then we'll call up to our base Tick method to let the
            // shell system take over.
            if (!_isPayloadActive)
            {
                base.Update(deltaSeconds);
            }
        }

        protected override bool RunSpecialCommand(string name, string[] args)
        {
            if (name == "exploits")
            {
                Console.WriteLine("Available exploits");
                Console.WriteLine("--------------------\n");

                string searchQuery = string.Join(" ", args).Trim();
                if (User.Exploits.Any(x => x.Id.Contains(searchQuery)))
                {
                    foreach (var exploit in User.Exploits.Where(x => x.Id.Contains(searchQuery)))
                    {
                        Console.WriteLine(" - {0}", exploit.Id);
                    }
                }
                else
                {
                    Console.WriteLine("No exploits were found that match that search query.");
                }

                return true;
            }

            if (name == "payloads")
            {
                Console.WriteLine("Available payloads");
                Console.WriteLine("--------------------\n");

                string searchQuery = string.Join(" ", args).Trim();
                if (User.Payloads.Any(x => x.Id.Contains(searchQuery)))
                {
                    foreach (var payload in User.Payloads.Where(x => x.Id.Contains(searchQuery)))
                    {
                        Console.WriteLine(" - {0}", payload.Id);
                    }
                }
                else
                {
                    Console.WriteLine("No payloads were found that match that search query.");
                }

                return true;
            }

            if (name == "use")
            {
                if (args.Length < 1)
                {
                    Console.WriteLine("error: too few arguments given. Do you want to use an exploit or payload?");
                    return true;
                }

                string ItemType = args[0].ToLower();
                args = args.Skip(1).ToArray();

                string ExploitName = string.Join(" ", args).Trim();

                if (ItemType == "exploit")
                {
                    var exp = User.Exploits.FirstOrDefault(x => x.Id == ExploitName);
                    if (exp == null)
                    {
                        Console.WriteLine("No exploit {0} was found.", ExploitName);
                    }
                    else
                    {
                        _selectedExploit = exp;
                        Console.WriteLine("Now using the \"{0}\" exploit.", exp.Name);
                    }
                    return true;
                }
                else if (ItemType == "payload")
                {
                    var pld = User.Payloads.FirstOrDefault(x => x.Id == ExploitName);
                    if (pld == null)
                    {
                        Console.WriteLine("No payload {0} was found.", ExploitName);
                    }
                    else
                    {
                        _selectedPayload = pld;
                        Console.WriteLine("Now using the \"{0}\" payload.", pld.Name);
                    }

                    return true;
                }
                else
                {
                    Console.WriteLine("error: Invalid item type. Must use 'exploit' or 'payload'.");
                    return true;
                }
            }

            if (name == "scan")
            {
                Console.WriteLine("Performing Nmap scan on remote system...");
                Console.WriteLine(string.Empty);
                Console.WriteLine("PORT\t\tSTATUS\tSERVICE");
                Console.WriteLine("-----\t\t-------\t--------");
                Console.WriteLine(string.Empty);
                foreach (var Service in _remoteSysstem.Services)
                {
                    Console.Write(Service.Port.ToString());
                    Console.Write("\t\t");

                    if (Service.IsFiltered)
                    {
                        Console.Write("filtered");
                    }
                    else
                    {
                        Console.Write("open");
                    }
                    Console.Write("\t");
                    Console.WriteLine(Service.ProtocolName);
                }
                return true;
            }

            if (name == "analyze")
            {
                if (args.Length < 1)
                {
                    Console.WriteLine("error: too few arguments. You must specify a port to analyze.");
                    return true;
                }

                string EnteredPort = args[0];
                int Result = -1;
                if (!int.TryParse(EnteredPort, out Result))
                {
                    Console.WriteLine("error: Port is not a number.");
                    return true;
                }

                this.ShowCoverTutorial();

                Console.WriteLine("GIGASPLOIT PORT ANALYSIS:");

                Console.WriteLine("    {0}:{1}\n", _remoteSysstem.IPAddress, EnteredPort);

                if (_remoteSysstem.Services.Any(x => x.Port == Result))
                {
                    var Service = _remoteSysstem.Services.First(x => x.Port == Result);
                    bool FoundVulns = false;

                    Console.WriteLine("Protocol: {0}", Service.ProtocolName);
                    Console.WriteLine("Detected implementation: {0}\n", Service.Implementation.Name);

                    if (Service.IsFiltered)
                    {
                        _remoteSysstem.AppendLog(User.IPAddress + ": connection blocked by firewall on port " + Service.Port);
                        Console.WriteLine("Service is filtered. Firewall detected.");
                    }
                    else
                    {
                        _remoteSysstem.AppendLog(User.IPAddress + ": connected to port " + Service.Port);
                        Console.WriteLine("Known vulnerabilities:\n");

                        foreach (var Exp in User.Exploits)
                        {
                            if (Exp.Attacks.Contains(Service.Implementation))
                            {
                                FoundVulns = true;
                                Console.WriteLine(" - {0}", Exp.Name);
                            }
                        }

                        if (!FoundVulns)
                        {
                            Console.WriteLine(" - Error: Gigasploit doesn't know any vulnerabilities in this service's implementation.");
                        }

                        _remoteSysstem.AppendLog(User.IPAddress + ": disconnected from port " + Service.Port);
                    }

                    /*
                    Console.GetUserContext()->GetPeacenet()->SendGameEvent("HackAnalyze", {
                        { "Computer", FString::FromInt(this->RemoteSystem->GetComputer().ID) },
                { "Identity", FString::FromInt(this->RemoteSystem->GetCharacter().ID) },
                { "Port", EnteredPort },
                { "Protocol", Service.Service->Protocol->Name.ToString() },
                { "Implementation", Service.Service->Name.ToString() },
                { "Filtered", (Service.IsFiltered) ? "True" : "False" },
                { "Vulnerable", (FoundVulns) ? "True" : "False" }
                    });*/
                    return true;
                }
                Console.WriteLine("Analysis failed:&r&7 Remote system not listening on this port.");
                return true;
            }

            if (name == "attack")
            {
                if (args.Length < 1)
                {
                    Console.WriteLine("error: too few arguments. You must specify a port to attack.");
                    return true;
                }

                if (_selectedExploit == null)
                {
                    Console.WriteLine("error: You must select an exploit to use first.");
                    return true;
                }

                if (_selectedPayload == null)
                {
                    Console.WriteLine("error: You must select a payload to use first.");
                    return true;
                }

                string EnteredPort = args[0];
                int Result = -1;
                if (!int.TryParse(EnteredPort, out Result))
                {
                    Console.WriteLine("&4&*error:&r Port is not a number.&7");
                    return true;
                }

                Console.WriteLine("Finding service on {0}:{1}...", _enteredHostname, EnteredPort);

                this.RunSpecialCommand("scan", args);

                var Service = _remoteSysstem.Services.FirstOrDefault(x => x.Port == Result);

                if (Service != null)
                {
                    /*
                    this->SendGameEvent("HackAttempt", {
                        { "Identity", FString::FromInt(this->RemoteSystem->GetCharacter().ID)},
                    { "Computer", FString::FromInt(this->RemoteSystem->GetComputer().ID)},
                    { "Exploit", this->CurrentExploit->ID.ToString()},
                    { "Payload", this->CurrentPayload->Name.ToString()},
                    { "Port", FString::FromInt(RealPort)},
                    { "Protocol", Service.Service->Protocol->Name.ToString()},
                    { "ServerSoftware", Service.Service->Name.ToString()}
                    });*/

                    Console.WriteLine("Service is {0}.", Service.Implementation.Name);

                    if (Service.IsFiltered)
                    {
                        _remoteSysstem.AppendLog(User.IPAddress + ": connection blocked by firewall on port " + Service.Port);

                        Console.WriteLine("Service is FILTERED! Can't continue with exploit.");

                        /*
                this->SendGameEvent("HackFail", {
                    { "Identity", FString::FromInt(this->RemoteSystem->GetCharacter().ID)},
                            { "Computer", FString::FromInt(this->RemoteSystem->GetComputer().ID)},
                            { "Exploit", this->CurrentExploit->ID.ToString()},
                            { "Payload", this->CurrentPayload->Name.ToString()},
                            { "Port", FString::FromInt(RealPort)},
                            { "Protocol", Service.Service->Protocol->Name.ToString()},
                            { "ServerSoftware", Service.Service->Name.ToString()},
                            { "Reason", "Firewall"}
                });*/

                        return true;
                    }

                    Console.WriteLine("Service is OPEN.");

                    _remoteSysstem.AppendLog(User.IPAddress + ": connected to port " + Service.Port);

                    if (_selectedExploit.Attacks.Contains(Service.Implementation))
                    {
                        Console.WriteLine("Service is vulnerable to the {0} exploit.", _selectedExploit.Name);

                        Console.WriteLine("Deploying {0}...", _selectedPayload.Name);

                        if (ShouldCrashService)
                        {
                            _remoteSysstem.AppendLog(Service.Implementation.Name + ": service stopped unexpectedly.");
                            _remoteSysstem.AppendLog(User.IPAddress + ": disconnected from port " + Service.Port);
                            Service.Crash();
                            Console.WriteLine("Connection refused.");
                            return true;
                        }

                        UserContext PayloadUser = _remoteSysstem.GetHackerContext(0, User);

                        _selectedPayload.Payload.Disconnected += this.OnDisconnect;

                        _isPayloadActive = true;

                        _selectedPayload.Payload.DeployPayload(Console, User, PayloadUser);

                        /*
                this->SendGameEvent("PayloadDeploy", {
                    { "Identity", FString::FromInt(this->RemoteSystem->GetCharacter().ID)},
                            { "Computer", FString::FromInt(this->RemoteSystem->GetComputer().ID)},
                            { "Exploit", this->CurrentExploit->ID.ToString()},
                            { "Payload", this->CurrentPayload->Name.ToString()},
                            { "Port", FString::FromInt(RealPort)},
                            { "Protocol", Service.Service->Protocol->Name.ToString()},
                            { "ServerSoftware", Service.Service->Name.ToString()}
                });*/

                        this.ShowPayloadTutorial();

                        _remoteSysstem.AppendLog(User.IPAddress + ": disconnected from port " + Service.Port);

                        return true;
                    }
                    else
                    {
                        _remoteSysstem.AppendLog(User.IPAddress + ": disconnected from port " + Service.Port);
                        Console.WriteLine("Service is not vulnerable to this exploit. Cannot drop payload.");
                        return true;
                    }
                }
                else
                {
                    Console.WriteLine("Connection refused.");
                    /*this->SendGameEvent("HackFail", {
                        { "Identity", FString::FromInt(this->RemoteSystem->GetCharacter().ID)},
                        { "Computer", FString::FromInt(this->RemoteSystem->GetComputer().ID)},
                        { "Exploit", this->CurrentExploit->ID.ToString()},
                        { "Payload", this->CurrentPayload->Name.ToString()},
                        { "Port", FString::FromInt(RealPort)},
                        { "Protocol", Service.Service->Protocol->Name.ToString()},
                        { "ServerSoftware", Service.Service->Name.ToString()},
                        { "Reason", "Crash"}
                    });*/
                    return true;
                }
            }
            return base.RunSpecialCommand(name, args);
        }

        protected override void OnRun(string[] args)
        {
            // Get the target IP address from docopt.
            string TargetIP = this.GetArgument("<host>").ToString();

            _enteredHostname = TargetIP;

            // Prompt that we're about to connect to it.
            Console.WriteLine("Connecting to {0}...", TargetIP);

            // Try to get a system context for theremote host.
            try
            {
                _remoteSysstem = User.ConnectTo(_enteredHostname);
            }
            catch(Exception ex)
            {
                Console.WriteLine("Error: {0}", ex.Message);
                Complete();
                return;
            }

            Console.WriteLine("Type help for a list of commands.");
            Console.WriteLine("Type exploits for a list of your known exploits.");

            // Broadcast a mission event that a hack has started.
            /*Console.GetUserContext()->GetPeacenet()->SendGameEvent("HackStart", {
                { "Computer", FString::FromInt(this->RemoteSystem->GetComputer().ID) },
        { "Identity", FString::FromInt(this->RemoteSystem->GetCharacter().ID) },
        { "Host", TargetIP },        
    });

            this->ShowTutorialIfNotSet("tuts.gigasploit.welcome",
                    "Gigasploit Framework Console",
                    "This is the <ui>Gigasploit Framework Console</>.\r\n\r\nMost <bad>hacking</> operations will be performed here.\r\n\r\nTo <bad>hack</> a system, you must select an <ui>exploit</>, a <ui>payload</>, and attack a <ui>service</>.\r\n\r\nTo <ui>scan</> the system for hack-able <ui>services</>, run the <cmd>scan</> command."
                );*/
        }

        protected float AssessStealthiness()
        {
            // If the origin system and remote system are the same, never report anything
            // but pure stealth.  Otherwise the game goes a little apeshit.
            if (_remoteSysstem == null || this.User.Computer == _remoteSysstem.Computer)
                return 1;

            // Stealthiness is a percentage value.
            float stealthiness = 1;

            // So basically what we're going to do is...
            //
            // 1. Check how many text files in the remote system
            // contain the local system's IP address compared to how many text files
            // are actually on the system - and calculate a percentage.  This percentage counts for
            // 50% of the player's stealthiness value.
            //
            // 2. 20% of the stealthiness comes from how many services have crashed compared to the total number of
            // services on the computer in the first place.
            //

            // Start by calculating how many tracks the player left behind.
            float filesCount = (float)_remoteSysstem.Computer.TextFiles.Count;
            float filesContainingIPAddress = 0;

            if (filesCount > 0)
            {
                foreach (var file in _remoteSysstem.Computer.TextFiles)
                {
                    if (file.Content.Contains(User.IPAddress))
                    {
                        filesContainingIPAddress += 1;
                    }
                }

                float tracksLeftPercentage = (filesContainingIPAddress / filesCount) * 0.5f;

                // Decrease stealthiness by that amount.
                stealthiness -= tracksLeftPercentage;
            }

            // Check crashiness percentage.
            float serviceCount = _remoteSysstem.Computer.Services.Count;
            float crashes = 0;

            if (serviceCount > 0)
            {
                foreach (var rule in _remoteSysstem.Computer.Services)
                {
                    if (rule.IsCrashed) crashes += 1;
                }

                stealthiness -= (crashes / serviceCount) * 0.20f;
            }

            // Step 3 would be take off another 20% if the system's owner is currently using the system
            // but this isn't implemented.

            // Return the stealthiness value as a definite percentage.
            return MathHelper.Clamp(stealthiness, 0, 1);

        }

        protected void OnDisconnect(object sender, EventArgs e)
        {
            _isPayloadActive = false;
            _selectedPayload.Payload.Disconnected -= OnDisconnect;

            // this->SendGameEvent("HackSuccess", {
            //    { "Identity", FString::FromInt(this->RemoteSystem->GetCharacter().ID)},
            //        { "Computer", FString::FromInt(this->RemoteSystem->GetComputer().ID)}
            // });

            // Finish up.
            Complete();
        }

        protected void ShowCoverTutorial()
        {

        }

        protected void ShowPayloadTutorial()
        {

        }
    }
}
