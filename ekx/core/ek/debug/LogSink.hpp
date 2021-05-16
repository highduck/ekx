#pragma once

#include "LogMessage.hpp"

namespace ek {

class LogSink {
public:
    virtual ~LogSink() = default;

    virtual void onMessageWrite(const LogMessage& message) = 0;
};

}