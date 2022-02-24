/* SPDX-FileCopyrightText: 2014-2019 The KPhotoAlbum Development Team

   SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "Server.h"

#include "RemoteCommand.h"

#include <KLocalizedString>
#include <QMessageBox>
#include <QTcpSocket>
#include <QUdpSocket>

using namespace RemoteControl;

Server::Server(QObject *parent)
    : RemoteConnection(parent)
{
}

bool Server::isConnected() const
{
    return m_isConnected;
}

void Server::listen(const QHostAddress &address)
{
    if (!m_socket) {
        m_socket = new QUdpSocket(this);
        bool ok = m_socket->bind(address, UDPPORT);
        if (!ok) {
            QMessageBox::critical(0, i18n("Unable to bind to socket"),
                                  i18n("Unable to listen for remote Android connections. "
                                       "This is likely because you have another KPhotoAlbum application running."));
        }
        connect(m_socket, &QUdpSocket::readyRead, this, &Server::readIncommingUDP);
    }
}

void Server::stopListening()
{
    delete m_socket;
    m_socket = nullptr;
    delete m_tcpSocket;
    m_tcpSocket = nullptr;
    emit stoppedListening();
}

QTcpSocket *Server::socket()
{
    return m_tcpSocket;
}

void Server::readIncommingUDP()
{
    Q_ASSERT(m_socket->hasPendingDatagrams());
    char data[1000];

    qint64 len = m_socket->readDatagram(data, 1000, &m_remoteAddress);
    QString string = QString::fromUtf8(data).left(len);
    QStringList list = string.split(QChar::fromLatin1(' '));
    if (list[0] != QString::fromUtf8("KPhotoAlbum")) {
        return;
    }
    if (list[1] != QString::number(RemoteControl::VERSION)) {
        QMessageBox::critical(0, i18n("Invalid Version"),
                              i18n("Version mismatch between Remote Client and KPhotoAlbum on the desktop.\n"
                                   "Desktop protocol version: %1\n"
                                   "Remote Control protocol version: %2",
                                   RemoteControl::VERSION,
                                   list[1]));
        stopListening();
        return;
    }

    connectToTcpServer(m_remoteAddress);
}

void Server::connectToTcpServer(const QHostAddress &address)
{
    m_remoteAddress = address;

    m_tcpSocket = new QTcpSocket;
    connect(m_tcpSocket, &QTcpSocket::connected, this, &Server::gotConnected);
    connect(m_tcpSocket, &QTcpSocket::readyRead, this, &Server::dataReceived);
    m_tcpSocket->connectToHost(m_remoteAddress, TCPPORT);
    connect(m_tcpSocket, &QTcpSocket::disconnected, this, &Server::lostConnection);
}

QHostAddress Server::remoteAddress() const
{
    return m_remoteAddress;
}

void Server::gotConnected()
{
    m_isConnected = true;
    emit connected();
}

void Server::lostConnection()
{
    m_isConnected = false;
    emit disConnected();
}
