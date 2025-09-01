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

### `get_user_host(user)`

Returns the host of the user.

### `get_user_user(user)`

Returns the user of the user

### `get_hostmask(user)`

Returns the hostmask of the user

### `channel_exists(chan)`

Returns a boolean value if chan is a channel the bot is in.

### `user_exists(user)`

Returns a boolean value if user exists to the bot.

### `is_op(chan, user)`

Returns a boolean value of the user's OP status in chan.

### `is_halfop(chan, user)`

Returns a boolean value of the user's Half Op status in chan.

### `is_voice(chan, user)`

Returns a boolean value of the user's voice status in chan.

### `is_on_channel(chan, user)`

Returns a boolean value of the user status in chan

### `is_botadmin(user)`

Returns a boolean value if the user is a bot admin

## Timers

### `add_timer(timeout, repeat, callback, data)`

Creates a timer that will call the `callback` function in `timeout` seconds, repeated `repeat` times.

`data` is optional and will be passed as the first argument of the callback function

Returns the timer_id number

### `set_timer_name(timer_id, name)`

Sets the name of the timer_id timer

### `get_timer_repeat(timer_id)`

Returns the number of times the timer has rang

### `del_timer(timer_id)`

Removes a timer


