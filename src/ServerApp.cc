#include "ClientServerAppMsg_m.h"
#include "ServerApp.h"
#include "IPvXAddressResolver.h"
#include <sstream>

Define_Module(ServerApp)

cEnvir& ServerApp::log() const {
    return EV<< "[ServerApp{" << localAddress_ << "}] ";
}

// the initialize method
void ServerApp::initialize(int stage) {
    log() << "initialize(" << stage << ")\n";

    cSimpleModule::initialize(stage);
    if (stage != 3)
        return;

    // obtain the values of parameters
    string localAddress = this->par("localAddress").stringValue();
    int localPort = this->par("localPort");

    // create a new socket for the listening role
    this->socket_ = new TCPSocket();
    this->socket_->setDataTransferMode(TCP_TRANSFER_OBJECT);

    // In the server role, we bind ourselves to the well-defined port and IP address on which
    // we listen for incoming connections
    this->socket_->bind(
            localAddress.length() ?
                    IPvXAddressResolver().resolve(localAddress.c_str()) :
                    IPvXAddress(), localPort);

    // register ourselves as the callback object
    this->socket_->setCallbackObject(this, NULL);  // send the flag

    // do not forget to set the outgoing gate
    this->socket_->setOutputGate(gate("tcpOut"));

    // now listen for incoming connections.  This version is the forking version where
    // upon every new incoming connection, a new socket is created.
    this->socket_->listen();

    // debugging
    log() << localAddress << " created a listening socket with "
            << "connectionId=" << this->socket_->getConnectionId() << endl;

    // now save this socket in our map
    this->socketMap_.addSocket(this->socket_);
    setStatusString("waiting");
}

/** the all serving handle message method */
void ServerApp::handleMessage(cMessage *msg) {
    log() << " handleMessage(cMessage*)\n";

    if (msg->isSelfMessage()) {
        throw cRuntimeError("we shouldn't see a self message");
    }

    // let the socket class process the message and make a call back on the
    // appropriate method. But note that we need to determine which socket must
    // handle this message: it could be connection establishment in our passive role
    // or it could be ack to our active conn establishment or it could be a data packet
    TCPSocket *socket = this->socketMap_.findSocketFor(msg);
    if (!socket) {
        // we are going to be here if we are a passive listener of incoming connection
        // at which point a connection will be established. But there will not be a
        // socket created yet for the data transfer. We create such a socket.

        // make sure first that we are dealing with a TCP command
        TCPCommand *cmd = dynamic_cast<TCPCommand *>(msg->getControlInfo());
        if (!cmd) {
            throw cRuntimeError(
                    "ServerApp::handleMessage: no TCPCommand control info in message (not from TCP?)");
        }

        int connId = cmd->getConnId();
        log() << " creating a new socket with connectionId=" << connId << endl;

        // notice that we must use the other constructor of TCPSocket so that it
        // will use the underlying connID that was created after an incoming
        // connection establishment message
        TCPSocket *new_socket = new TCPSocket(msg);

        // register ourselves as the callback object
        new_socket->setCallbackObject(this, NULL);

        // do not forget to set the outgoing gate
        new_socket->setOutputGate(gate("tcpOut"));

        // another thing I learned the hard way is that we must set up the data trasnfer
        // mode for this new socket
        new_socket->setDataTransferMode(this->socket_->getDataTransferMode());

        // now save this socket in our map
        this->socketMap_.addSocket(new_socket);

        this->peerList.insert(new_socket->getRemoteAddress().str());

        // process the message
        new_socket->processMessage(msg);
    } else {
        // let that socket process the message
        socket->processMessage(msg);
    }
}

/** this method is provided to clean things up when the simulation stops */
void ServerApp::finish() {
    log() << "finish()\n";
    // cleanup all the sockets
    this->socketMap_.deleteSockets();
}

void ServerApp::socketEstablished(int connID, void* role) {
    if (role != NULL)
        throw cRuntimeError("We should always be passive");

    log() << "received socketEstablished message on connID " << connID << " +++"
            << endl;
    setStatusString("ConnectionEstablished");
}

/** handle incoming data. Could be a request or response */
void ServerApp::socketDataArrived(int connID, void *, cPacket *msg, bool) {
    log() << "socketDataArrived(...)\n";

    // incoming request may be of different types
    CS_Packet *packet = dynamic_cast<CS_Packet *>(msg);
    if (!packet) {
        log() << "DYNAMIC CAST TO CS_Packet FAILED\n";
        return;
    }

    switch ((CS_MSG_TYPE) packet->getType()) {
    case CS_REQUEST: {
        CS_Req *req = dynamic_cast<CS_Req *>(msg);
        if (!req) {
            log() << "Arriving packet is not of type CS_Req\n";
        } else {
            setStatusString("Request");
            log() << "Arriving packet: Requestor ID = " << req->getId()
                    << ", Requested file size = " << req->getFilesize() << endl;

            // now send a response
            this->sendResponse(connID, this->localAddress_.c_str(), req->getFilesize());
        }
    }
        break;
    default:
        log() << "unknown incoming request type " << packet->getType() << endl;
        break;
    }
    // cleanup
    delete msg;
}

void ServerApp::socketPeerClosed(int connId, void *) {
    log() << "socketPeerClosed(" << connId << ")\n";
}

void ServerApp::socketClosed(int connId, void *) {
    log() << "socketClosed(" << connId << ")\n";
    setStatusString("closed");
}

void ServerApp::socketFailure(int connId, void *, int code) {
    log() << "socketFailure(" << connId << "," << code << ")\n";
    setStatusString("broken");
}

// close the peer side
void ServerApp::close() {
    log() << "close()\n";

    setStatusString("closing");
    this->socket_->close();
}

// send a response
void ServerApp::sendResponse(int connId, const char *id, unsigned long size) {
    log() << "sendResponse(" << connId << ", size=" << size << ")\n";

    // this is a hack because the TCPSocketMap does not allow us to search based on
    // connection ID. So we have to take a circuitous route to get to the socket
    cMessage *temp_msg = new cMessage("temp");
    TCPCommand *temp_cmd = new TCPCommand();
    temp_cmd->setConnId(connId);
    temp_msg->setControlInfo(temp_cmd);

    TCPSocket *socket = this->socketMap_.findSocketFor(temp_msg);
    if (!socket) {
        log() << "Cannot find socket to send request\n";
    } else {
        CS_Resp *resp = new CS_Resp();
        resp->setType((int) CS_RESPONSE);
        resp->setId(id);
        std::string result = this->getPeersAsString();
        resp->setDataArraySize(result.length());
        for (int i=0; i < result.length(); ++i)
            resp->setData(i, result[i]);
        // need to set the byte length else nothing gets sent as I found the hard way

        // TODO What should this size be?
        resp->setByteLength(sizeof(CS_Resp) + result.length());
        resp->setUniqueId(0);
        socket->send(resp);
    }

    // cleanup
    delete temp_msg;
}

void ServerApp::setStatusString(const char *s) {
    if (ev.isGUI ()) {
        getDisplayString ().setTagArg ("t", 0, s);
        bubble (s);
    }
}

std::string ServerApp::getPeersAsString(){
    std::stringstream ss;
    std::ostream_iterator<std::string> output(ss, " ");
    std::copy(peerList.begin(), peerList.end(), output);
    std::string result = ss.str();
    result.pop_back();
    return result;

}

