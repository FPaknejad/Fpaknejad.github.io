namespace Loupedeck.CSPPlugin
{
    using System;

    public class RotateAdjustment : PluginDynamicAdjustment
    {
        private int _rotateLevel = 0; // Starting at 100% zoom, adjustable range TBD

        public RotateAdjustment()
            : base(displayName: "Rotate", description: "Rotate page in CSP", groupName: "View", hasReset: true)
        {
        }

        protected override void ApplyAdjustment(String actionParameter, Int32 diff)
        {
            try
            {
                _rotateLevel += diff * 10; // Adjust zoom by 10% per tick (customize as needed)
                _rotateLevel = Math.Max(10, Math.Min(1000, _rotateLevel)); // Limit range 10% to 1000%

                if (diff > 0) // Zoom in
                {
                    this.Plugin.ClientApplication.SendKeyboardShortcut(66);
                    PluginLog.Info($"Rotate clockwise {_rotateLevel}% with Cmd + R");
                }
                else if (diff < 0) // Zoom out
                {
                    this.Plugin.ClientApplication.SendKeyboardShortcut(44);
                    PluginLog.Info($"Rotate counterclockwise {_rotateLevel}% with Cmd + L");
                }
                this.AdjustmentValueChanged();
            }
            catch (Exception ex)
            {
                PluginLog.Error(ex, $"Failed to rotate {_rotateLevel}%");
            }
        }

        protected override void RunCommand(String actionParameter)
        {
            _rotateLevel = 0; // Reset to 0°
            this.AdjustmentValueChanged();
            PluginLog.Info("Rotate level reset to 0°");
            
        }

        protected override String GetAdjustmentValue(String actionParameter) => $"{_rotateLevel}%";
    }
}