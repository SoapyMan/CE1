#ifndef _SERVERSNOOPER_H_
#define _SERVERSNOOPER_H_

#include <INetwork.h>
class CServerSnooper :
	public IServerSnooper
{
public:
	CServerSnooper(void);
	virtual ~CServerSnooper(void);
	bool Init(IServerSnooperSink* pSink);
public:
	//IServerSnooper
	void SearchForLANServers(uint nTime);
	void Update(uint nTime);
	void Release() { delete this; }
protected:
	void ProcessPacket(CStream& stmPacket, CIPAddress& ip);
private:
	CDatagramSocket m_socket;
	uint m_nStartTime;
	uint m_nCurrentTime;
	IServerSnooperSink* m_pSink;
};

#endif //_SERVERSNOOPER_H_