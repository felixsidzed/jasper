/*
 *  IXSocketFactory.cpp
 *  Author: Benjamin Sergeant
 *  Copyright (c) 2019 Machine Zone, Inc. All rights reserved.
 */

#include "IXSocketFactory.h"

#include "IXUniquePtr.h"
#include "IXsocket.h"
#include "IXSocketOpenSSL.h"

namespace ix
{
    std::unique_ptr<Socket> createSocket(bool tls,
                                         int fd,
                                         std::string& errorMsg,
                                         const SocketTLSOptions& tlsOptions)
    {
        (void) tlsOptions;
        errorMsg.clear();
        std::unique_ptr<Socket> socket;

        if (!tls)
        {
            socket = ix::make_unique<Socket>(fd);
        }
        else
        {
            socket = ix::make_unique<SocketOpenSSL>(tlsOptions, fd);
        }

        if (!socket->init(errorMsg))
        {
            socket.reset();
        }

        return socket;
    }
} // namespace ix
