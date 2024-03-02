# API

## Events

### Event Types

- `PRIVMSG_SELF - user, host, message`
- `PRIVMSG_CHAN - user, host, channel, message`
- `JOIN - user, host, channel`
- `JOIN_MYSELF - channel`
- `PART - user, host, channel, reason`
- `PART_MYSELF - channel, reason`
- `QUIT - user, host, reason`
- `NICK - user, host, newnick`
- `NICK_MYSELF - newnick`
- `NICK_INUSE - newnick`
- `CTCP - user, host, to, message`
- `IRC_CONNECTED`
- `TICK`

### `add_handler(event, handler)`

Adds a handler for an event.

Example:

```lua
function msg(user, host, channel, message)
    print(user .. " said: " .. message)
end

add_handler(PRIVMSG_CHAN, msg)
```

### `del_handler(event, handler)`

Removes a handler for an event.

Example:

```lua
del_handler(PRIVMSG_CHAN, msg)
```

## IRC Commands

### `raw(message)`

Sends a raw message to the server.

### `privmsg(to, message)`

Sends a message to a channel or user.

### `notice(to, message)`

Sends a notice to a channel or user.

### `join(channel)`

Joins a channel.

### `part(channel, reason)`

Leaves a channel.

### `kick(channel, user, reason)`

Kicks a user from a channel. Reason is optional.

### `mode(channel, mode, target)`

Sets a mode on a channel.

### `ctcp(to, message)`

Sends a CTCP message to a channel or user.

