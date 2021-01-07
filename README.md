# MPQDraft
MPQDraft allows in-memory patching of game data files, added features through plugins, and creation of self-executing mods.
This fork aims to update MPQDraft up to Win10 APIs and streamline the development process when used in combination with GPTP plugin development.

## Command Line Usage
```
Usage: MPQDraft.exe -launch <scExePath> <mpqFiles> <qdpFiles>

Example:
MPQDraft.exe -launch "C:\Starcraft\StarCraft.exe" "C:\Mod\my_mod_1.mpq,C:\Mod\my_mod_2.mpq" "C:Mod\my_plugin_1.qdp,C:\Mod\my_plugin_2.qdp"
```

## Development
Required Visual Studio components:

* C++ MFC for v142 build tools
* C++ ATL for v141 build tools
* C++ MFC for v141 build tools

### Building
- Open MPQDraft\MPQDraft.sln in Visual Studio 2019
- Build->Build Solution
- Open Debug\MPQDraftD.exe

## Credits
[Quantam, creator of MPQDraft](http://qstuff.blogspot.com/2010/01/bibliography-programming.html) Licensed under CDDL 