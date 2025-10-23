namespace Loupedeck.CSPPlugin
{
    using System;

    public class ZoomAdjustment : PluginDynamicAdjustment
    {
        private int _zoomLevel = 100; // Starting at 100% zoom, adjustable range TBD

        public ZoomAdjustment()
            : base(displayName: "Zoom Level", description: "Adjusts zoom level in CSP", groupName: "Zoom", hasReset: true)
        {
        }

        protected override void ApplyAdjustment(String actionParameter, Int32 diff)
        {
            try
            {
                _zoomLevel += diff * 10; // Adjust zoom by 10% per tick (customize as needed)
                _zoomLevel = Math.Max(10, Math.Min(1000, _zoomLevel)); // Limit range 10% to 1000%

                if (diff > 0) // Zoom in
                {
                    this.Plugin.ClientApplication.SendKeyboardShortcut(VirtualKeyCode.Equals, ModifierKey.Win);
                    PluginLog.Info($"Zoomed in to {_zoomLevel}% with Cmd + =");
                }
                else if (diff < 0) // Zoom out
                {
                    this.Plugin.ClientApplication.SendKeyboardShortcut(VirtualKeyCode.Minus, ModifierKey.Win);
                    PluginLog.Info($"Zoomed out to {_zoomLevel}% with Cmd + -");
                }
                this.AdjustmentValueChanged();
            }
            catch (Exception ex)
            {
                PluginLog.Error(ex, $"Failed to adjust zoom to {_zoomLevel}%");
            }
        }

        protected override void RunCommand(String actionParameter)
        {
            _zoomLevel = 100; // Reset to 100% zoom
            this.AdjustmentValueChanged();
            PluginLog.Info("Zoom level reset to 100%");
        }

        protected override String GetAdjustmentValue(String actionParameter) => $"{_zoomLevel}%";
    }
}