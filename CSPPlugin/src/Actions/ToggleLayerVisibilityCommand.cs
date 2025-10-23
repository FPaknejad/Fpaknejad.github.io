namespace Loupedeck.CSPPlugin
{
    using System;

    public class ToggleLayerVisibilityCommand : PluginDynamicCommand
    {
        private bool _isVisible = true;

        public ToggleLayerVisibilityCommand()
            : base(displayName: "Toggle Layer Visibility", description: "Toggles visibility of selected layer", groupName: "Layers")
        {
        }

        protected override void RunCommand(String actionParameter)
        {
            try
            {
                // Send Cmd + H (Left Windows for Cmd on macOS, H for hide/show)
                this.Plugin.ClientApplication.SendKeyboardShortcut(VirtualKeyCode.KeyH, ModifierKey.Control | ModifierKey.Win);
                this._isVisible = !this._isVisible;
                this.ActionImageChanged();
                PluginLog.Info($"Toggled layer visibility to {(this._isVisible ? "Visible" : "Hidden")} with Cmd + ^ + H");
            }
            catch (Exception ex)
            {
                PluginLog.Error(ex, $"Failed to toggle layer visibility to {(this._isVisible ? "Visible" : "Hidden")}");
            }
        }

        protected override String GetCommandDisplayName(String actionParameter, PluginImageSize imageSize) =>
            $"Toggle Layer{Environment.NewLine}{(this._isVisible ? "Visible" : "Hidden")}";
    }
}