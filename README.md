![xbot](https://raw.githubusercontent.com/ablakely/xbot/master/resources/xbot.png)

# xbot

xbot is a modular IRC bot in C for Linux and Windows


## Building

To build xbot use the following commands

	make
    make mods

Afterwords, just edit xbot.cfg and execute the xbot binary.

## Built-in Admin Commands

These are commands which allow the bot's admin to control it once it's connected to the IRC server.

* LOADMOD <module>
* UNLOADMOD <module>
* MODLIST
* JOIN <channel>

