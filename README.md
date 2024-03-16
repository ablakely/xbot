<p align="center">
  <img src="https://raw.githubusercontent.com/ablakely/xbot/master/resources/xbot.png" alt="xbot">
  <br>
  <h2 align="center">xbot</h2>
</p>

xbot is a modular IRC bot in C for Linux and Windows


## Building

### Linux
To build xbot use the following commands

	make
    make mods

### Windows
To build xbot on Windows, you will need to use the Visual Studio solution file which is located in the root directory of the project.
Currently only the debug build target is configured for the Visual Studio solution file. The release build target will be added in the future.

All of the dependencies for xbot are included in the repository, so you should be able to build the project without any additional setup.

Tested with Visual Studio 2010.

#### Modules
As of current modules are built outside of the main build process. To build modules, you will need to either use the provided batch file or build them manually.

To build the modules you need to run ```buildmods.bat``` located in the mods directory.  This can be done from the command line or by double clicking the file.

## Configuration
The bot will read xbot.cfg if none is specified at the command line with ```-c <file>```.

## Built-in Admin Commands

These are commands which allow the bot's admin to control it once it's connected to the IRC server.

* LOADMOD <module>
* UNLOADMOD <module>
* MODLIST
* JOIN <channel>

## Modules

Modules are the main way to extend the functionality of the bot. They are loaded at runtime and can be unloaded and reloaded without restarting the bot.

Currently, the following modules are included:
* lua - Provides Lua scripting support
* openssl - Provides SSL support
* test

## Windows Support Notes
### Windows XP
* OpenSSL doesn't work without some kind of kernel extensions for XP.
* Luarocks doesn't work due to symlinks not being supported.  This will limit what can be done with the Lua scripts.

These issues might be resolved with a project like [onecore](https://github.com/Skulltrail192/One-Core-API-Binaries) but further testing is needed.

## License
xbot is licensed under the MIT License. See the LICENSE file for more information.

---
Written by Aaron Blakely. Copyright 2024.
