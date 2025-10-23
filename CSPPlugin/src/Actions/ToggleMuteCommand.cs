namespace Loupedeck.CSPPlugin
{
    using System;

    public class ToggleMuteCommand : PluginDynamicCommand
    {
        private Boolean _isMuted = false;

        public ToggleMuteCommand()
            : base(displayName: "Toggle Mute", description: "Toggles audio mute state", groupName: "Audio")
        {
        }

        protected override void RunCommand(String actionParameter)
        {
            try
            {
                // Attempt to use VolumeMute (173) for system mute
                this.Plugin.ClientApplication.SendKeyboardShortcut(VirtualKeyCode.VolumeMute);
                PluginLog.Info("Attempted to toggle mute with VolumeMute (173)");
                
                // Fallback to F10 (121) - note: Fn modifier not handled
                if (!this._isMuted) // Only try if not already muted
                {
                    this.Plugin.ClientApplication.SendKeyboardShortcut(VirtualKeyCode.F10);
                    PluginLog.Info("Attempted to toggle mute with F10 (121) - Fn required manually");
                }
                this._isMuted = !this._isMuted;
                this.ActionImageChanged();
            }
            catch (Exception ex)
            {
                PluginLog.Error(ex, "Failed to toggle mute");
            }
        }

        protected override String GetCommandDisplayName(String actionParameter, PluginImageSize imageSize) =>
            $"Toggle Mute{Environment.NewLine}{(this._isMuted ? "Muted" : "Unmuted")}";
    }
}