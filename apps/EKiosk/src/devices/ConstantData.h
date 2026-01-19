#pragma once

namespace CommandResult {
    enum Result {
        Port = 0x00,
        NoAnswer = 0x01,
        Answer = 0x02,
        Transport = 0x03,
        Protocol = 0x04,
        OK = 0x05,
        Driver = 0x06,
        Id = 0x07,
        CRC = 0x08,
    };
}

typedef CommandResult::Result TResult;
