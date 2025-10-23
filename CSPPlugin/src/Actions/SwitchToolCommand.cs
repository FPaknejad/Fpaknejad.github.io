namespace Loupedeck.CSPPlugin
{
    using System;

    public class SwitchToolCommand : PluginDynamicCommand
    {
        private Boolean _isBrush = true;

        public SwitchToolCommand()
            : base(displayName: "Switch Tool", description: "Switches between Brush and Eraser", groupName: "Commands")
        {
        }

        protected override void RunCommand(String actionParameter)
        {
            var keyCode = this._isBrush ? VirtualKeyCode.KeyB : VirtualKeyCode.KeyE; // B for brush, E for eraser
            try
            {
                this.Plugin.ClientApplication.SendKeyboardShortcut(keyCode);
                this._isBrush = !this._isBrush;
                this.ActionImageChanged();
                PluginLog.Info($"Switched to tool: {(this._isBrush ? "Brush" : "Eraser")} with key {keyCode} ({(Int32)keyCode})");
            }
            catch (Exception ex)
            {
                PluginLog.Error(ex, $"Failed to switch tool to {(this._isBrush ? "Brush" : "Eraser")}");
            }
        }

        protected override String GetCommandDisplayName(String actionParameter, PluginImageSize imageSize) =>
            $"Switch Tool{Environment.NewLine}{(this._isBrush ? "Brush" : "Eraser")}";
    }
}