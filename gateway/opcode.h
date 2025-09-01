#pragma once

namespace jasper {
    enum EventOpCode : int {
        OP_DISPATCH,
        OP_HEARTBEAT,
        OP_IDENTIFY,
        OP_PRESENCE_UPDATE,
        OP_VOICE_STATE_UPDATE,
        OP_RESUME = 6,
        OP_RECONNECT,
        OP_REQUEST_GUILD_MEMBERS,
        OP_INVALID_SESSION,
        OP_HELLO,
        OP_HEARTBEAT_ACK,
    };
}
