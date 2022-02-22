/* SPDX-FileCopyrightText: 2014 Jesper K. Pedersen <blackie@kde.org>

   SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef REMOTECONNECTION_H
#define REMOTECONNECTION_H

#include <QHostAddress>
#include <QObject>

class QUdpSocket;
class QTcpSocket;

namespace RemoteControl
{
class RemoteCommand;

class RemoteConnection : public QObject
{
    Q_OBJECT
public:
    static constexpr int UDPPORT = 23455;
    static constexpr int TCPPORT = 23456;
    explicit RemoteConnection(QObject *parent = 0);
    virtual bool isConnected() const = 0;
    void sendCommand(const RemoteCommand &);

signals:
    void gotCommand(const RemoteCommand &);

protected slots:
    void dataReceived();

protected:
    virtual QTcpSocket *socket() = 0;

private:
    enum ReadingState { WaitingForLength,
                        WaitingForData };
    ReadingState m_state = WaitingForLength;
    qint32 m_length;
};

}

#endif // REMOTECONNECTION_H
