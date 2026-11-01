Pocket One — macOS universal (Apple Silicon and Intel)

The .pkg installs the standalone application in /Applications and the VST3 in
/Library/Audio/Plug-Ins/VST3. Installing it requires administrator permission.

For the portable ZIP, copy Pocket One.app to Applications. Copy Pocket One.vst3
to ~/Library/Audio/Plug-Ins/VST3 (create this folder if needed). Restart Ableton
Live and enable VST3 plug-in system folders in its plug-in preferences.

These builds have local ad-hoc signatures and are not notarized by Apple.
macOS may require allowing this downloaded app/installer in Privacy & Security.

To remove the package, delete /Applications/Pocket One.app and
/Library/Audio/Plug-Ins/VST3/Pocket One.vst3. For a user-only portable install,
remove the copy in ~/Library/Audio/Plug-Ins/VST3 instead.

Native compilation, unit tests and package inspection do not certify playback
in Ableton Live. The hybrid engines contain verified DSP cores and documented
adapters; complete hardware sound matching remains pending.
