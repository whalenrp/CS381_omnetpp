#ifndef _CS381_SERVER_APP_H_
#define _CS381_SERVER_APP_H_

#include <string>
using namespace std;

#include "INETDefs.h"
#include "TCPSocket.h"
#include "TCPSocketMap.h"

class ServerApp: public cSimpleModule, public TCPSocket::CallbackInterface {

private:

    cEnvir& log() const;

    string localAddress_;
    TCPSocket *socket_;   // our main listening socket
    TCPSocketMap socketMap_; // maps of sockets we maintain

protected:

    virtual void initialize(int stage);

    /**
     * define how many initialization stages are we going to need.
     */
    virtual int numInitStages(void) const {
        return 4;
    }

    virtual void handleMessage(cMessage *msg);

    /**
     * Records basic statistics: numSessions, packetsSent, packetsRcvd,
     * bytesSent, bytesRcvd. Redefine to record different or more statistics
     * at the end of the simulation.
     */
    virtual void finish();

    /** @name Utility functions */

    /** Issues CLOSE command */
    virtual void close(void);

    /** Sends a request and response */
    virtual void sendResponse(int connId, const char *id, unsigned long size);

    /** When running under GUI, it displays the given string next to the icon */
    virtual void setStatusString(const char *s);
    //@}

    /** @name TCPSocket::CallbackInterface callback methods */

    //@{
    /** Does nothing but update statistics/status. Redefine to perform or schedule first sending. */
    virtual void socketEstablished(int connId, void *yourPtr);

    /**
     * Does nothing but update statistics/status. Redefine to perform or schedule next sending.
     * Beware: this funcion deletes the incoming message, which might not be what you want.
     */
    virtual void socketDataArrived(int connId, void *yourPtr, cPacket *msg,
            bool urgent);

    /** Since remote TCP closed, invokes close(). Redefine if you want to do something else. */
    virtual void socketPeerClosed(int connId, void *yourPtr);

    /** Does nothing but update statistics/status. Redefine if you want to do something else, such as opening a new connection. */
    virtual void socketClosed(int connId, void *yourPtr);

    /** Does nothing but update statistics/status. Redefine if you want to try reconnecting after a delay. */
    virtual void socketFailure(int connId, void *yourPtr, int code);

    /** Redefine to handle incoming TCPStatusInfo. */
    virtual void socketStatusArrived(int connId, void *yourPtr,
            TCPStatusInfo *status) {
        delete status;
    }
    //@}

};

#endif
