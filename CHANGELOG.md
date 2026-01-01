# Changelog

All notable changes to this project from 2009 and onwards will be documented in this file. Listed are a number of old releases (between 2002-05-25 and 2008-08-21) - but that list is not comprehensive. It is unknown what was changed in those releases.

## 2026-01-01

### Added
- Completely rewritten GUI in Qt. It has a few more features than the old and is more structured.
- More help for the user, with texts that explain what things do.
- Ability to patch games without built-in support.
- CLI that can do all that the GUI can (except configure plugins, which for technical reasons is not feasible from the terminal in a pretty way - if you want to configure a plugin, use the GUI once and then the plugins in the CLI should have the same settings)
- Support for adding custom icons to SEMPQs.
- Cross-compilation is now possible. Using MinGW-w64, one can now compile the code from a Linux machine (Mac too presumably). MPQDraft itself is still very much a Windows 32-bit only program though, but at least you no longer need a Windows machine to build it.

### Removed
- MFC dependency from the 1990s. The GUI is now based on Qt, and the application is now cross-compilable



## 2025-11-18

### Added
- GitHub Actions build pipeline to compile for Windows.
- The CLI will now check that plugins are properly cuonfigured.
- Added `--help` and `--version` arguments to the CLI.
- The pipeline will now build separate executables, one CLI (MFC free) and one GUI (MFC dependent, also including CLI).

### Changed
- Refactored codebase.
- Changed CLI commands to be more Unix-like.
- Made CLI be free from MFC and thus easier to build on modern systems.



## 2021-01-08

### Added
- Added CLI to run MPQDraft from the command line without opening the GUI
- Added debugging output functionality to help troubleshoot issues

### Changed
- Upgraded the project from an older Visual Studio version to Visual Studio 2019
- Updated Windows compatibility to Windows 10
- Fixed build configuration issues so the project compiles correctly
- Organized build output into proper Debug and Release folders
- Made Debug builds create a separate executable (MPQDraftD.exe) so you can tell debug and release versions apart



## 2009-09-13

### Changed
- Fixed issue where SEMPQ wizard couldn't select files that don't exist yet (while still requiring existing files for regular patching).
- Made plugin loading more robust - one broken plugin won't prevent other plugins from loading.
- Fixed bug where plugin modules weren't properly linked to their parent plugins.
- Fixed missing data assignment in plugin list display.
- Improved reliability of detecting whether the correct program is being patched by using a more direct comparison method instead of comparing file paths as text.

### Removed
- Removed unnecessary debug logging.



## 2008-08-21
## 2008-04-06
## 2004-08-24
## 2003-07-30
## 2003-03-22
## 2002-12-17



## 2002-05-25

### Added
- Now works with War3 (at least it doesn't give any error messages :-p).
- Now allows you to run MPQDraft on programs that don't use MPQs (so that you can use plugins).

### Changed
- Fixed "failed to distribute map" bug when playing SC Battle.net games.
- Made the patching error messages more descriptive.



## 2002-01-01

### Added
- Initial release.
