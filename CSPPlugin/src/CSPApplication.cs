namespace Loupedeck.CSPPlugin
{
    using System;
    using System.Diagnostics;

    public class CSPApplication : ClientApplication
    {
        public CSPApplication()
        {
        }

        protected override String GetProcessName() => "";

        protected override String GetBundleName() => "jp.co.celsys.CLIPSTUDIO"; // Verified bundle ID

        public override ClientApplicationStatus GetApplicationStatus()
        {
            try
            {
                var processes = Process.GetProcesses();
                foreach (var process in processes)
                {
                    PluginLog.Info($"Checking process: {process.ProcessName}");
                    if (process.ProcessName.Contains("CLIP STUDIO PAINT", StringComparison.OrdinalIgnoreCase))
                    {
                        return ClientApplicationStatus.Installed;
                    }
                }
                return ClientApplicationStatus.NotInstalled;
            }
            catch (Exception ex)
            {
                PluginLog.Error(ex, "Failed to check application status");
                return ClientApplicationStatus.Unknown;
            }
        }
    }
}