#include "server.h"

Server::Server(QObject *parent)
{

}

void Server::startServer()
{
    allClients = new QVector<QTcpSocket*>;
    chatServer = new QTcpServer();
    //limits the maximum pending connections to 10 clients
    chatServer->setMaxPendingConnections(10);
    //handles new connections by calling newClientConnection
    connect(chatServer, SIGNAL(newConnection()),this,SLOT(newClientConnection()));

    //port ranges from 1024 to 49151
    //Tells the server to listen for incoming connections on address address
    //and port port.
    if(chatServer->listen(QHostAddress::Any, 8001))
    {
        qDebug() << "Server has started. Listening to port 8001";
    }
    else
    {
        qDebug() << "Server failed to start. Error: " + chatServer->errorString();
    }
}

void Server::sendMessageToClients(QString message)
{
    //checks there are clients connected
    if(allClients->size() > 0)
    {
        //iterates over the clients
        for (int var = 0; var < allClients->size(); ++var)
        {
            //if the client is open and is writable
            //isOpen:  A device is open if it can be read from and/or written to.
            //isWritable: This is a convenience function which checks if the OpenMode of the device contains the WriteOnly flag.
            if(allClients->at(var)->isOpen() && allClients->at(var)->isWritable())
            {
                //write the message
                //write:Writes at most maxSize bytes of data from data to the device. Returns the number of bytes that were actually written,
                //or -1 if an error occurred
                allClients->at(var)->write(message.toUtf8());
            }
        }
    }
}

void Server::newClientConnection()
{
    //Returns the next pending connection as a connected QTcpSocket object.
    QTcpSocket* client = chatServer->nextPendingConnection();
    //Returns the address of the connected peer if the socket is in ConnectedState
    QString ipAddress = client->peerAddress().toString();
    //Returns the port of the connected peer if the socket is in ConnectedState
    int port = client->peerPort();

    //connects incoming client to slots for
    //disconnected, read to read and state changed events
    connect(client, &QTcpSocket::disconnected, this, &Server::socketDisconnected);
    connect(client, &QTcpSocket::readyRead, this, &Server::socketReadyRead);
    connect(client, &QTcpSocket::stateChanged, this, &Server::socketStateChanged);

    //adds the client to the client vector
    allClients->push_back(client);
    qDebug() << "Socket connected from " + ipAddress + ":" + QString::number(port);
}

void Server::socketDisconnected()
{
    QTcpSocket* client = qobject_cast<QTcpSocket*>(QObject::sender());
    QString socketIpAddress = client->peerAddress().toString();
    int port = client->peerPort();
    qDebug() << "Socket disconnected from " + socketIpAddress + ":" + QString::number(port);
}

void Server::socketReadyRead()
{
    QTcpSocket* client = qobject_cast<QTcpSocket*>(QObject::sender());
    QString socketIpAddress = client->peerAddress().toString();
    int port = client->peerPort();

    //readAll(): Reads all remaining data from the device, and returns it as a byte array
    QString data = QString(client->readAll());
    qDebug() << "Message: " + data + " (" + socketIpAddress + ":" + QString::number(port) + ")";
    sendMessageToClients(data);
}

void Server::socketStateChanged(QAbstractSocket::SocketState state)
{
    QTcpSocket* client = qobject_cast<QTcpSocket*>(QObject::sender());
    QString socketIpAddress = client->peerAddress().toString();
    int port = client->peerPort();
    QString description;

    if(state == QAbstractSocket::UnconnectedState)
    {
        description = "The socket is not connected.";
    }
    else if(state == QAbstractSocket::HostLookupState)
    {
        description = "The socket is performing a host name lookup.";
    }
    else if(state == QAbstractSocket::ConnectingState)
    {
        description = "The socket has started establishing a connection.";
    }
    else if(state == QAbstractSocket::ConnectedState)
    {
        description = "A connection is estabilished.";
    }
    else if(state == QAbstractSocket::BoundState)
    {
        description = "The socket is bound to an address and port.";
    }
    else if(state == QAbstractSocket::ClosingState)
    {
        description = "The socket is about to close (data may still be waiting to be written).";
    }
    else if(state == QAbstractSocket::ListeningState)
    {
        description = "For internal use only.";
    }

    qDebug() << "Socket state changed (" + socketIpAddress + ":" + QString::number(port) + "):" + description;
}
