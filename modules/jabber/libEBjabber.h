#ifndef __LIB_JABBER__
#define __LIB_JABBER__

#include <jabber/jabber.h>
#include <glib.h>

#define DEFAULT_HOST "jabber.com"
#define DEFAULT_PORT 5222

#define LINE_LENGTH 513

#define DBG_JBR do_jabber_debug
extern int do_jabber_debug;

extern GList *agent_list;

enum
{
	JABBER_ONLINE,
	JABBER_AWAY,
	JABBER_DND,
	JABBER_XA,
	JABBER_CHAT,
	JABBER_OFFLINE,
};

typedef struct {
	char host[256];
	char name[256];
	char alias[256];
	char desc[256];
	char service[256];
	char type[256];
} Agent;


typedef struct INSTANT_MESSAGE {
    int year;                    /* six fields used for time stamping */
    int month;                   
    int day;                 
    int hour;
    int minute;
    int sec;            
    char *msg;                   /* message itself             */
    char *sender;                /* sender of the message      */
} JABBER_InstantMessage, *JABBER_InstantMessage_PTR;

/* 
** The following structure holds the connection information.
** I included the cookie and the common name, because both those things 
** come from the server.  Common name might be moved out later.
**                           - shane
*/

typedef struct JABBERCONN {
    char   passwd[LINE_LENGTH+1];
    char   jid[LINE_LENGTH+1];	/* The jabber id, of the form user@host/resource */
    int	   listenerID;
    jconn  conn;		/* The actualy jabber connection struct */
    int    id;			/* FIXME: User better. A key to connect sent requests and returned packets */
    int    reg_flag;		/* Indicates whether we have registered successfully with the jabber server */
    struct JABBERCONN *next;
} JABBER_Conn;

struct jabber_buddy
{
	char *name;				/* Users name */
	char *jid;				/* the buddy's id */
	char *sub;				/* Subscriptions state */
	int  status;				/* Their current status */
	JABBER_Conn *JConn;
};


/*
** Dialog message
*/

typedef struct DIALOG_MSG {
    char     *handle;
    char     *requestor;
    char     *message;
    char     *heading;
    char     *response;
    JABBER_Conn *JConn;
    void     (*callback)(void *data);
} JABBER_Dialog, *JABBER_Dialog_PTR;

/*
** Authorization message
*/

typedef struct AUTH_MSG {
    char     *handle;
    char     *requestor;
    JABBER_Conn *conn;
} JABBER_AuthMessage, *JABBER_AuthMessage_PTR;
 
/*
** Name:    JABBER_AuthorizeContact
** Purpose: This function sends an authorize message to the server
** Input:   conn   - JABBER connection structure
**          handle - handle to authorize
** Output:  0 on success, -1 on failure
*/

int JABBER_AuthorizeContact(JABBER_Conn *conn, char *handle);

/*
** Name:    JABBER_Login
** Purpose: This function encapsulates the login process to JABBER
** Input:   handle   - user's handle
**          passwd   - user's password
**          host     - notification server to use
**          port     - port number of notifcation server
** Output:  0 on success, -1 on failure
*/

JABBER_Conn *JABBER_Login(char *handle, char *passwd, char *host, int port);

/*
** Name:    JABBER_SendMessage
** Purpose: This function encapuslates the sending of an instant message
** Input:   handle - user's handle who is receiving the message
**          message - the actual message to send
** Output:  0 on success, -1 on failure
*/

int JABBER_SendMessage(JABBER_Conn *JConn, char *handle, char *message);

/*
** Name:    JABBER_AddContact
** Purpose: This function sends a add to forward contact list message to the
**          server
** Input:   handle  - handle to add to the list
** Output:  0 on success, -1 on failure
*/

int JABBER_AddContact(JABBER_Conn *JConn, char *handle);

/*
** Name:    JABBER_RemoveContact
** Purpose: This function sends a remove to forward contact list message to
**          the server
** Input:   handle - handle to remove from the list
** Output:  0 on success, -1 on failure
*/

int JABBER_RemoveContact(JABBER_Conn *JConn, char *handle);

/*
** Name:    JABBER_EndChat
** Purpose: This function sends an OUT mesage to the server to end a
**          chat with a user
** Input:   handle - handle to end chat with
** Output:  0 on success, -1 on failure
*/

int JABBER_EndChat(JABBER_Conn *JConn, char *handle);

/*
** Name:    JABBER_Logout
** Purpose: This function properly logs out of the JABBER service
** Input:   none
** Output:  0 on success, -1 on failure
*/

int JABBER_Logout(JABBER_Conn *JConn);

/*
** Name:    JABBER_ChangeState
** Purpose: This function changes the current state of the user
** Input:   state - integer representation of the state
** Output:  0 on success, -1 on failure
*/

int JABBER_ChangeState(JABBER_Conn *JConn, int state);

/*
** Name:    JABBER_LeaveChatRoom
** Purpose: This function leaves a jabber chat room
** Input:   room_name - name of the chat room
** Input:   nick - name to use in the chat room
** Output:  0 on success, -1 on failure
*/

int JABBER_LeaveChatRoom(JABBER_Conn *JConn, char *room_name, char *nick);

/*
** Name:    JABBER_JoinChatRoom
** Purpose: This function joins a jabber chat room
** Input:   room_name - name of the chat room
** Input:   nick - name to use in the chat room
** Output:  0 on success, -1 on failure
*/

int JABBER_JoinChatRoom(JABBER_Conn *JConn, char *room_name, char *nick);

/*
** Name:    JABBER_SendChatRoomMessage
** Purpose: This function sends messages to a  chatroom
** Input:   room_name - Name of the chat room
** Input:   message - The message to send
** Input:   nick - The name that is sending the message
** Output:  0 on success, -1 on failure
*/

int JABBER_SendChatRoomMessage(JABBER_Conn *JConn, char *room_name, char *message, char *nick);

/* Debugging routines */
void j_list_agents();

#endif /* __LIB_JABBER__ */
