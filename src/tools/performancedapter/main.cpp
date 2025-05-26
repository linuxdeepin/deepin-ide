// SPDX-FileCopyrightText: 2023 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "serverhandler.h"
#include "tools.h"

#include <jsonrpccpp/server/connectors/tcpsocketserver.h>

#include <QApplication>

#include <iostream>

namespace OptionNames
{
const QString port {"port"};
}

namespace DefaultValues
{
const QString port{"3309"};
}

const QList<QCommandLineOption> options
{
    {
        QCommandLineOption {
            OptionNames::port, QString("Server open port, default %0.\n").arg(DefaultValues::port), "number", DefaultValues::port
        }
    },
};

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QCommandLineParser parser;

    parser.addOptions(options);
    parser.addHelpOption();
    parser.process(a);

    auto list = parser.optionNames();

    quint16 port = DefaultValues::port.toUShort();
    if (list.contains(OptionNames::port)) {
        port = parser.value(OptionNames::port).toUShort();
    }

    jsonrpc::TcpSocketServer server("127.0.0.1", port);
    ServerHandler hand(server, new Tools);

    if (hand.StartListening()) {
        std::cout << "Server started successfully name: "
                  << QCoreApplication::applicationName().toStdString()
                  << "port: " << port
                  << std::endl;
    } else {
        std::cout << "Error starting Server name: "
                  << QCoreApplication::applicationName().toStdString()
                  << "port: " << port
                  << std::endl;
    }

    return a.exec();
}
