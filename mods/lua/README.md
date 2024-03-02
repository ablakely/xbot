# API

## Lua Commands

### `raw`(`message`: string)

Sends a raw message to the server.

### `privmsg`(`to`: string, `message`: string)

Sends a message to a channel or user.

### `notice`(`to`: string, `message`: string)

Sends a notice to a channel or user.

### `join`(`channel`: string)

Joins a channel.

### `part`(`channel`: string)

Leaves a channel.

### `kick`(`channel`: string, `user`: string, `reason`: string)

Kicks a user from a channel. Reason is optional.

### `mode`(`channel`: string, `mode`: string, `target`: string)

Sets a mode on a channel.

### `ctcp`(`to`: string, `message`: string)

Sends a CTCP message to a channel or user.

