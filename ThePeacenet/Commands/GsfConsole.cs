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
                        NSLOCTEXT("Tutorials", "GigasploitAnalyzeTitle", "Analyzing a port"),
                        NSLOCTEXT("Tutorials", "GigasploitAnalyze", "While <ui>scanning</> a system will tell you the active <ui>ports</> of the system, if you'd like to see more details about a <ui>specific port</> you need to <ui>analyze</> it.\r\n\r\nThis can be done by running the <cmd>analyze <port></> command.\r\n\r\nThis will tell you more information about the port, such as what server software is running it and what <ui>exploits</> you can use.\r\n\r\n<bad>WARNING</>: Analyzing a port may result in your IP address being logged, thus decreasing your <ui>cover</>.")
                    );
                    this->ShowTutorialIfNotSet("tuts.gigasploit.exploits",
                        NSLOCTEXT("Tutorials", "GigasploitExploitsTitle", "Exploits"),
                        NSLOCTEXT("Tutorials", "GigasploitExploits", "An <ui>exploit</> is used to tell <ui>Gigasploit</> how to attack a specific <ui>vulnerability</> in a service to allow it to <bad>deploy a payload</>.\r\n\r\nRun the <cmd>exploits</> command to see all of the exploits you currently have available to you.  You will find more exploits as you hack more systems.")
                    );
                    this->ShowTutorialIfNotSet("tuts.gigasploit.exploits.more",
                        NSLOCTEXT("Tutorials", "GigasploitExploitsTitle", "Exploits"),
                        NSLOCTEXT("Tutorials", "GigasploitExploits", "To see more information about a particular <ui>exploit</>, you can use the <cmd>man <exploit></> command in another Terminal.\r\n\r\nSome exploits are more stable than others, and the more stable an exploit is, the less chance there is of it crashing the service it attacks.\r\n\r\nTo <ui>use</> an exploit, run <cmd>use exploit <exploit></>.")
                    );
                    this->ShowTutorialIfNotSet("tuts.gigasploit.payloads",
                        NSLOCTEXT("Tutorials", "GigasploitPayloadsTitle", "Payloads"),
                        NSLOCTEXT("Tutorials", "GigasploitPayloads", "<ui>Payloads</> are tiny programs that get deployed to the remote computer using <ui>exploits</>.  You can use any <ui>payload</> you'd like, but there are certain payloads that are better for different jobs.\r\n\r\nThe <ui>most common</> payload you will use is the <ui>reverse shell</> - which will connect to your computer and allow you to take control of the remote computer.\r\n\r\nOther <ui>payloads</> can be used to open <ui>additional services</>, bypass <bad>firewalls</>, or even <bad>crash</> a service or the whole system.\r\n\r\nTo see all of your available payloads, run the <cmd>payloads</> command.  You will find more payloads as you hack more systems.\r\n\r\nTo <ui>use</> a payload, run the <cmd>use payload <payload></> command.")
                    );
                    this->ShowTutorialIfNotSet("tuts.gigasploit.hud",
                        NSLOCTEXT("Tutorials", "GigasploitHudTitle", "Gigasploit HUD"),
                        NSLOCTEXT("Tutorials", "GigasploitHud", "The <ui>remote system</>'s IP address is displayed in <ui>Gigasploit</>'s prompt.  Next to the IP address is the currently-selected <ui>exploit</> in yellow and <ui>payload</> in red.\r\n\r\nBoth an <ui>exploit</> and a <ui>payload</> need to be selected before you can <ui>attack</>.")
                    );
                    this->ShowTutorialIfNotSet("tuts.gigasploit.attack",
                        NSLOCTEXT("Tutorials", "GigasploitAttackTitle", "Launching the attack"),
                        NSLOCTEXT("Tutorials", "GigasploitAttack", "Once you are ready to <bad>launch the attack</>, run the <cmd>attack <port></> command.  This will attempt to attack the specified port with the selected exploit and deploy the payload.\r\n\r\nIf the specified port is <bad>blocked</> by a <bad>firewall</>, or the service <bad>crashes</> because of the exploit, the attack will <bad>fail</> and you'll need to try a different attack.\r\n\r\nIf the attack <ui>succeeds</>, Gigasploit will exit and, if necessary, let the <ui>payload</> have access to your <ui>Terminal</>.")
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
            /*
            EConnectionError ConnectionError = EConnectionError::None;
            if (!InConsole->GetUserContext()->GetPeacenet()->ResolveSystemContext(TargetIP, this->RemoteSystem, ConnectionError))
            {
                InConsole->WriteLine(FText::Format(NSLOCTEXT("Gigasploit", "ConnectionError", "{0}: could not connect to remote host."), FText::FromString(InArguments[0])));
                this->Complete();
                return;
            }*/

            Console.WriteLine("Type help for a list of commands.");
            Console.WriteLine("Type exploits for a list of your known exploits.");

            // Broadcast a mission event that a hack has started.
            /*InConsole->GetUserContext()->GetPeacenet()->SendGameEvent("HackStart", {
                { "Computer", FString::FromInt(this->RemoteSystem->GetComputer().ID) },
        { "Identity", FString::FromInt(this->RemoteSystem->GetCharacter().ID) },
        { "Host", TargetIP },        
    });

            this->ShowTutorialIfNotSet("tuts.gigasploit.welcome",
                    NSLOCTEXT("CommandNames", "Gigasploit", "Gigasploit Framework Console"),
                    NSLOCTEXT("Tutorials", "GigasploitWelcome", "This is the <ui>Gigasploit Framework Console</>.\r\n\r\nMost <bad>hacking</> operations will be performed here.\r\n\r\nTo <bad>hack</> a system, you must select an <ui>exploit</>, a <ui>payload</>, and attack a <ui>service</>.\r\n\r\nTo <ui>scan</> the system for hack-able <ui>services</>, run the <cmd>scan</> command.")
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
