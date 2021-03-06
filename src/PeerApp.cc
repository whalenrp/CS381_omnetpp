/*
 * ClientApp.cc
 *
 *  Created on: Feb 1, 2014
 *      Author: Aniruddha Gokhale
 *      Class:  CS381
 *      Institution: Vanderbilt University
 */

// An application logic for Client behavior. This code has been developed by
// modifying the code in TCPBasicClientApp and TCPGenericSrvApp available
// in the INET framework
#include "ClientServerAppMsg_m.h"    // generated header from the message file
#include "PeerApp.h"               // our header
#include "IPvXAddressResolver.h"     // manages both IPv4 and IPV6 addess resolution
// this is needed by OMNeT++ because it registers our C++ class with the underlying
// OMNeT simulator upon which all the callbacks can be made when events occur
Define_Module(PeerApp)
;


int PeerApp::uniqueIdCounter = 0;

// constructor
PeerApp::PeerApp(void) :
        myID_(), server_(), connectPort_(0), fileSize_(1024), // 1 Kilo bytes
        socket_(NULL), peerList(), uniqueId(uniqueIdCounter++), mPacketTimes() {
    // nothing
}

// the initialize method. We initialize the parameter. Connection to the server is done after an event is triggered
void PeerApp::initialize(int stage) {
    EV<< "--- ClientApp::initialize(" << stage << ")\n";

    cSimpleModule::initialize (stage);
    if (stage != 3)
    return;

    // here is a way to get a string value. In this case we get our ID
    this->myID_ = this->par ("myID").stringValue ();

    // here is a way to get a string value. In this case we get the server's address
    this->server_ = this->par ("connectAddress").stringValue ();

    // obtain the value of server port
    this->connectPort_ = this->par ("connectPort");

    // obtain the value of file size requested
    this->fileSize_ = this->par ("fileSize");

    // Debugging purposes
    EV << "Client " << this->myID_ << " is initialized with transfer mode = TCP_TRANSFER_OBJECT for file size of " << this->fileSize_
    << " and server details: name = " << this->server_
    << " and port = " << this->connectPort_ << endl;

    // now we start a timer so that when it kicks in, we make a connection to the server.
    // this event serves as a way to kickstart things.
    cMessage *timer_msg = new cMessage ("timer");
    this->scheduleAt (simTime () + exponential (0.001), timer_msg);

    setStatusString ("waiting");

    packetArrivalSignal = registerSignal("arrival");

    /*****
     * New Stuff
     */
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

    this->socketMap_.addSocket(socket_);
}

    /** This is the all-encompassing event handling function. It is our responsibility to
     *  figure out what to do with every event, which is appln-specific */
void PeerApp::handleMessage(cMessage *msg) {
    EV<< "Client received handleMessage message" << endl;

    // check if this was a self generated message, such as a timeout to wake us up to
    // connect to the server
    if (msg->isSelfMessage ())
    this->handleTimer (msg);
    else {  // not a timer; must be a packet.


        // make sure first that we are dealing with a TCP command (and not something else) because otherwise
        // we do not know how to handle that kind of message
        TCPCommand *cmd = dynamic_cast<TCPCommand *>(msg->getControlInfo());
        if (!cmd) {
            throw cRuntimeError("ClientApp::handleMessage: no TCPCommand control info in message (not from TCP?)");
        }

        // for the sake of correctness, we make sure the socket exists
        if (!this->socket_) {  // no socket. How did we come here?
            throw cRuntimeError("ClientApp::handleMessage: no connection yet to server");
        }

        /*****
         * New Stuff
         */
        //TCPSocket* socket = socketMap_.findSocketFor(msg);
        socket_ = socketMap_.findSocketFor(msg);
        if (!socket_){
            socket_ = new TCPSocket(msg);

            // register ourselves as the callback object
            socket_->setCallbackObject(this, NULL);

            // do not forget to set the outgoing gate
            socket_->setOutputGate(gate("tcpOut"));

            // another thing I learned the hard way is that we must set up the data trasnfer
            // mode for this new socket
            socket_->setDataTransferMode(TCP_TRANSFER_OBJECT);

            // now save this socket in our map
            this->socketMap_.addSocket(socket_);
            this->peerList.insert(socket_->getRemoteAddress().str());
        }
        socket_->processMessage(msg);

        /****
         * End new stuff
         */

        // Everything seems fine. So process the message. Note that the processMessage is a method defined on the
        // TCPSocket class. Internally it will make the appropriate callback on the overridden methods of the
        // TCPSocketCallbackInterface object
//        this->socket_->processMessage (msg);
    }
}

    /** this method is provided to clean things up when the simulation stops and all statistics collection should be finalized here */
void PeerApp::finish() {
    EV<< "=== finish called" << endl;

    std::string modulePath = getFullPath();
    EV << "=== Client: Before cleaning up socket, it remains in "
    << this->socket_->stateName (this->socket_->getState ()) << " state ====" << endl;

    // our socket is closed. Clean it.
    delete this->socket_;
}

    /** handle the timeout method. We are using this as the initial method to kick start the process */
void PeerApp::handleTimer(cMessage *msg) {
    EV<< "=== Client: " << this->myID_ << " received handleTimer message. "
    << "Make a connection to server" << endl;

    // cleanup the incoming message (remember to free up because the param was dynamically allocated; see initialize method)
    delete msg;

    // make connection to our server using our helper method
    this->socket_ = this->connect (this->server_.c_str());
    socketMap_.addSocket(socket_);
}

    /*************************************************/
    /** implement all the callback interface methods */
    /*************************************************/

// when the client actively establishes a connection to the server, and if it is successful,
// this callback will be invoked so that we can do the next set of things. In our case it is as
// simple as telling the server that we are interested in a file of a certain size.
//
// Note that the second parameter is going to be NULL because we did not
// provide any param when registering ourself with the callback.
void PeerApp::socketEstablished(int connID, void *role) {
    EV<< "=== Client: " << this->myID_
    << " received socketEstablished message on connID " << connID << " ===" << endl;

    // just to make sure some random stuff did not show up for our second param
    if (role != NULL) {
        throw cRuntimeError("ClientApp::socketEstablished -- unrecognized param in callback");
    }

    // bubble during animation
    setStatusString("ConnectionEstablished");

    // send requests to the server to whom we just got connected
    this->sendRequest (connID);
}

    /** handle incoming data. Could be a request or response */
void PeerApp::socketDataArrived(int connID, void *, cPacket *msg, bool) {
    EV<< "=== Client: " << this->myID_
    << " received socketDataArrived message. ===" << endl;

    // incoming request may be of different types.
    // incoming request may be of different types
    CS_Packet *packet = dynamic_cast<CS_Packet *>(msg);
    if (!packet) {
        EV << "DYNAMIC CAST TO CS_Packet FAILED\n";
        return;
    }

    switch ((CS_MSG_TYPE) packet->getType()) {
        case CS_REQUEST: // We will never receive a request from the tracker here
        {
            break;
        }
        case CS_RESPONSE: // We will received updates from the tracker here with peer info.
        {
            CS_Resp *response = dynamic_cast<CS_Resp *> (msg);
            if (!response) {
                EV << "Arriving packet is not of type CS_Resp" << endl;
            } else {
                setStatusString ("Response Arrived");

                EV << "Arriving packet: Responder ID = " << response->getId ()
                << ", packet size = " << response->getDataArraySize () << endl;

                this->handleResponse (response);
            }
            break;
        }
        case PEER_REQUEST:
        {
            EV << "******************** Peer Request *******************" << endl;
            Peer_Req *req = dynamic_cast<Peer_Req *>(msg);
            if (!req) {
                EV << "Arriving packet is not of type Peer_Req\n";
            } else {
                setStatusString("Request");
                EV << "****************** Arriving Peer packet: Requestor ID = " << req->getId()
                        << ", Requested file size = " << req->getFilesize() << endl;

                // now send a response
                this->sendResponse(connID, this->localAddress_.c_str(), 1000);
            }
            break;
        }
        case PEER_RESPONSE:
        {
            // now that the response to our file request is received, our job is done
            // and so we close the connection
            this->close ();
            break;
        }

    }
    // cleanup the incoming message after it is handled.
    delete msg;



}

void PeerApp::socketPeerClosed(int connID, void *) {
    EV<< "=== Client: " << this->myID_ << " received socketPeerClosed message" << endl;
    EV << "server closed for connID = " << connID << endl;

    // in our case, we are not asking our server to initiate closing the socket. Instead the client
    // will do so once it gets the response.
}

void PeerApp::socketClosed(int, void *) {
    // for some odd reason, we are never getting invoked here. My understanding was that
    // after we explicitly call close, we should be getting here.

    EV<< "=== Client: " << this->myID_ << " received socketClosed message" << endl;
    setStatusString("connection closed");

    EV << "=== Client: Before cleaning up socket, it remains in "
    << this->socket_->stateName (this->socket_->getState ()) << " state ====" << endl;

    // our socket is closed. Clean it.
    delete this->socket_;
}

void PeerApp::socketFailure(int, void *, int code) {
    EV<< "=== Client: " << this->myID_ << " received socketFailure message" << endl;
    // subclasses may override this function, and add code try to reconnect after a delay.
    EV << "connection broken\n";
    setStatusString("broken");

}

    /**********************************************************************/
    /**           helper methods                                          */
    /**********************************************************************/

// connect to server in the active role
TCPSocket* PeerApp::connect(const char* serverAddr) {
    EV<< "=== Client: " << this->myID_ << " issuing a connect to server" << endl;
    setStatusString ("connecting"); // this is for generating the bubble in the animation

    // we allocate a socket to be used for actively connecting to the server and
    // transferring data over it. Note, we are a client.
    TCPSocket* tempSocket = new TCPSocket ();
    if (!tempSocket) {
        throw cRuntimeError("ClientApp::initialize: failed to create connecting socket");
    }

    // don't forget to set the output gate for this socket. I learned it the
    // hard way :-(
    tempSocket->setOutputGate (gate ("tcpOut"));

    // another thing I learned the hard way is that we must set up the data transfer
    // mode for this new socket
    tempSocket->setDataTransferMode (TCP_TRANSFER_OBJECT);

    // issue a connect request. Note that as an active entity, we must connect to the server's address
    // and its port
    tempSocket->connect (IPvXAddressResolver().resolve (serverAddr),
            this->connectPort_);

    // do not forget to set ourselves as the callback on this new socket.
    tempSocket->setCallbackObject (this, NULL);// we don't send any metadata as second param

    // debugging
    EV << "+++ Client: " << this->myID_ << " created a new socket with "
    << "connection ID = " << tempSocket->getConnectionId () << " ===" << endl;

    return tempSocket;
}

// close the client
void PeerApp::close() {
    EV<< "=== Client: " << this->myID_ << " received close () message" << endl;
    EV << "issuing CLOSE command\n";

    setStatusString("closing");

    this->socket_->close ();
}

// send a request to the other side
void PeerApp::sendRequest(int connId) {
    EV<< "=== Client: " << this->myID_ << " sendRequest. === " << endl;

    // for correctness, check for validity of the socket. But this should never happen because the only way
    // we could have arrived in this method is from the socketConnectionEstablished method where
    // we have already established the connection. But we shall remain ultra cautious :-)
    if (!this->socket_) {
        throw cRuntimeError("ClientApp::sendRequest -- no valid socket");
    }

    // create a Request message
    CS_Req *req = new CS_Req ();
    req->setType ((int)CS_REQUEST);
    req->setId (this->myID_.c_str ());
    req->setFilesize (this->fileSize_);

    // need to set the byte length else nothing gets sent as I found the hard way. I am assuming
    // this is the size of the application packet
    req->setByteLength (sizeof (CS_MSG_TYPE)
            + this->myID_.length()
            + sizeof (this->fileSize_));

    this->socket_->send (req);

    // Push on the current time to our queue
    mPacketTimes.push(std::time(0));

}

// handle incoming response and collect statistics
void PeerApp::handleResponse(CS_Resp *response) {
    EV<< "=== Client: " << this->uniqueId << " handleResponse. === " << endl;

    // Pop the earliest time from our queue and emit it as our Round Trip Time
    double rtt = difftime(std::time(0), mPacketTimes.front());
    emit(packetArrivalSignal, rtt);
    mPacketTimes.pop();


    if (response->getUniqueId() == 0){ // Received a response from the tracker
        // let us print what we got
        std::stringstream ss;
        EV << "Client received file from server: " << response->getId ()
        << " and a file of size " << response->getDataArraySize() << endl;
        for (unsigned int i = 0; i < response->getDataArraySize (); ++i) {
            EV << response->getData (i);
            ss << response->getData(i);
        }
        while (!ss.eof()){
            std::string peer;
            ss >> peer;

            TCPSocket* socket = this->connect (peer.c_str());
            if (socket->getLocalAddress().str() != socket->getRemoteAddress().str()){
                peerList.insert(peer);

                EV << endl;
                EV << "-------------CONNECTING TO PEER-------------"<< endl;
                EV << "-- local: " << socket->getLocalAddress().str() << endl;
                EV << "-- remote: " << socket->getRemoteAddress().str() << endl;

                this->sendPacketToPeer(socket);
                socketMap_.addSocket(socket);
            }
        }
    }else{ // Received a response from a client
        //sendPacketToPeers();

    }
    EV << endl;
}

// this method is used to flash messages during animation where we can see bubbles on the screen.
void PeerApp::setStatusString(const char *s) {
    if (ev.isGUI ()) {
        getDisplayString ().setTagArg ("t", 0, s);
        bubble (s);
    }
}

void PeerApp::sendPacketToPeer(TCPSocket* socket){
    Peer_Req * req = new Peer_Req();
    req->setType((int)PEER_REQUEST);
    req->setUniqueId(this->uniqueId);
    req->setId (this->myID_.c_str ());
    req->setUniqueId(uniqueId);
    req->setFilesize(8);

    // need to set the byte length else nothing gets sent as I found the hard way. I am assuming
    // this is the size of the application packet
    req->setByteLength (sizeof (CS_MSG_TYPE)
            + this->myID_.length()
            + sizeof (this->fileSize_));

    socket->send (req);
}

// send a response
void PeerApp::sendResponse(int connId, const char *id, unsigned long size) {
    EV << "sendResponse(" << connId << ", size=" << size << ")\n";

    // this is a hack because the TCPSocketMap does not allow us to search based on
    // connection ID. So we have to take a circuitous route to get to the socket

    if (!socket_) {
        EV << "Cannot find socket to send request\n";
    } else {
        Peer_Resp *resp = new Peer_Resp();
        resp->setType((int) PEER_RESPONSE);
        resp->setId(id);
        resp->setUniqueId(uniqueId);
        resp->setDataArraySize(size);
        // need to set the byte length else nothing gets sent as I found the hard way

        // TODO What should this size be?
        resp->setByteLength(sizeof(CS_Resp) + size);
        resp->setUniqueId(uniqueId);
        socket_->send(resp);
        EV << "************** Peer sending response *****************"<< endl;
        EV << "localAddress: " << socket_->getLocalAddress() << " remoteAddress: " << socket_->getRemoteAddress() << endl;
    }
}

