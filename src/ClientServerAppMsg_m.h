//
// Generated file, do not edit! Created by opp_msgc 4.4 from ClientServerAppMsg.msg.
//

#ifndef _CLIENTSERVERAPPMSG_M_H_
#define _CLIENTSERVERAPPMSG_M_H_

#include <omnetpp.h>

// opp_msgc version check
#define MSGC_VERSION 0x0404
#if (MSGC_VERSION!=OMNETPP_VERSION)
#    error Version mismatch! Probably this file was generated by an earlier version of opp_msgc: 'make clean' should help.
#endif



/**
 * Enum generated from <tt>ClientServerAppMsg.msg</tt> by opp_msgc.
 * <pre>
 * enum CS_MSG_TYPE {
 *     CS_REQUEST = 0;
 *     CS_RESPONSE = 1;
 *     PEER_REQUEST = 2; 
 *     PEER_RESPONSE = 3;
 * };
 * </pre>
 */
enum CS_MSG_TYPE {
    CS_REQUEST = 0,
    CS_RESPONSE = 1,
    PEER_REQUEST = 2,
    PEER_RESPONSE = 3
};

/**
 * Class generated from <tt>ClientServerAppMsg.msg</tt> by opp_msgc.
 * <pre>
 * packet CS_Packet
 * {
 *    int     type @enum(CS_MSG_TYPE);  
 * };
 * </pre>
 */
class CS_Packet : public ::cPacket
{
  protected:
    int type_var;

  private:
    void copy(const CS_Packet& other);

  protected:
    // protected and unimplemented operator==(), to prevent accidental usage
    bool operator==(const CS_Packet&);

  public:
    CS_Packet(const char *name=NULL, int kind=0);
    CS_Packet(const CS_Packet& other);
    virtual ~CS_Packet();
    CS_Packet& operator=(const CS_Packet& other);
    virtual CS_Packet *dup() const {return new CS_Packet(*this);}
    virtual void parsimPack(cCommBuffer *b);
    virtual void parsimUnpack(cCommBuffer *b);

    // field getter/setter methods
    virtual int getType() const;
    virtual void setType(int type);
};

inline void doPacking(cCommBuffer *b, CS_Packet& obj) {obj.parsimPack(b);}
inline void doUnpacking(cCommBuffer *b, CS_Packet& obj) {obj.parsimUnpack(b);}

/**
 * Class generated from <tt>ClientServerAppMsg.msg</tt> by opp_msgc.
 * <pre>
 * packet CS_Req extends CS_Packet
 * {
 *     string	id;		        
 *     int filesize;	        
 *     int uniqueId;
 * };
 * </pre>
 */
class CS_Req : public ::CS_Packet
{
  protected:
    opp_string id_var;
    int filesize_var;
    int uniqueId_var;

  private:
    void copy(const CS_Req& other);

  protected:
    // protected and unimplemented operator==(), to prevent accidental usage
    bool operator==(const CS_Req&);

  public:
    CS_Req(const char *name=NULL, int kind=0);
    CS_Req(const CS_Req& other);
    virtual ~CS_Req();
    CS_Req& operator=(const CS_Req& other);
    virtual CS_Req *dup() const {return new CS_Req(*this);}
    virtual void parsimPack(cCommBuffer *b);
    virtual void parsimUnpack(cCommBuffer *b);

    // field getter/setter methods
    virtual const char * getId() const;
    virtual void setId(const char * id);
    virtual int getFilesize() const;
    virtual void setFilesize(int filesize);
    virtual int getUniqueId() const;
    virtual void setUniqueId(int uniqueId);
};

inline void doPacking(cCommBuffer *b, CS_Req& obj) {obj.parsimPack(b);}
inline void doUnpacking(cCommBuffer *b, CS_Req& obj) {obj.parsimUnpack(b);}

/**
 * Class generated from <tt>ClientServerAppMsg.msg</tt> by opp_msgc.
 * <pre>
 * packet CS_Resp extends CS_Packet
 * {
 *     string  id;			
 * 
 *     char data [];	    
 *     int uniqueId;
 * };
 * </pre>
 */
class CS_Resp : public ::CS_Packet
{
  protected:
    opp_string id_var;
    char *data_var; // array ptr
    unsigned int data_arraysize;
    int uniqueId_var;

  private:
    void copy(const CS_Resp& other);

  protected:
    // protected and unimplemented operator==(), to prevent accidental usage
    bool operator==(const CS_Resp&);

  public:
    CS_Resp(const char *name=NULL, int kind=0);
    CS_Resp(const CS_Resp& other);
    virtual ~CS_Resp();
    CS_Resp& operator=(const CS_Resp& other);
    virtual CS_Resp *dup() const {return new CS_Resp(*this);}
    virtual void parsimPack(cCommBuffer *b);
    virtual void parsimUnpack(cCommBuffer *b);

    // field getter/setter methods
    virtual const char * getId() const;
    virtual void setId(const char * id);
    virtual void setDataArraySize(unsigned int size);
    virtual unsigned int getDataArraySize() const;
    virtual char getData(unsigned int k) const;
    virtual void setData(unsigned int k, char data);
    virtual int getUniqueId() const;
    virtual void setUniqueId(int uniqueId);
};

inline void doPacking(cCommBuffer *b, CS_Resp& obj) {obj.parsimPack(b);}
inline void doUnpacking(cCommBuffer *b, CS_Resp& obj) {obj.parsimUnpack(b);}

/**
 * Class generated from <tt>ClientServerAppMsg.msg</tt> by opp_msgc.
 * <pre>
 * packet Peer_Req extends CS_Req {};
 * </pre>
 */
class Peer_Req : public ::CS_Req
{
  protected:

  private:
    void copy(const Peer_Req& other);

  protected:
    // protected and unimplemented operator==(), to prevent accidental usage
    bool operator==(const Peer_Req&);

  public:
    Peer_Req(const char *name=NULL, int kind=0);
    Peer_Req(const Peer_Req& other);
    virtual ~Peer_Req();
    Peer_Req& operator=(const Peer_Req& other);
    virtual Peer_Req *dup() const {return new Peer_Req(*this);}
    virtual void parsimPack(cCommBuffer *b);
    virtual void parsimUnpack(cCommBuffer *b);

    // field getter/setter methods
};

inline void doPacking(cCommBuffer *b, Peer_Req& obj) {obj.parsimPack(b);}
inline void doUnpacking(cCommBuffer *b, Peer_Req& obj) {obj.parsimUnpack(b);}

/**
 * Class generated from <tt>ClientServerAppMsg.msg</tt> by opp_msgc.
 * <pre>
 * packet Peer_Resp extends CS_Resp {};
 * </pre>
 */
class Peer_Resp : public ::CS_Resp
{
  protected:

  private:
    void copy(const Peer_Resp& other);

  protected:
    // protected and unimplemented operator==(), to prevent accidental usage
    bool operator==(const Peer_Resp&);

  public:
    Peer_Resp(const char *name=NULL, int kind=0);
    Peer_Resp(const Peer_Resp& other);
    virtual ~Peer_Resp();
    Peer_Resp& operator=(const Peer_Resp& other);
    virtual Peer_Resp *dup() const {return new Peer_Resp(*this);}
    virtual void parsimPack(cCommBuffer *b);
    virtual void parsimUnpack(cCommBuffer *b);

    // field getter/setter methods
};

inline void doPacking(cCommBuffer *b, Peer_Resp& obj) {obj.parsimPack(b);}
inline void doUnpacking(cCommBuffer *b, Peer_Resp& obj) {obj.parsimUnpack(b);}


#endif // _CLIENTSERVERAPPMSG_M_H_
