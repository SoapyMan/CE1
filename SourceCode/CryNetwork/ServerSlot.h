//////////////////////////////////////////////////////////////////////
//
//	Crytek Network source code
//	
//	File: ServerSlot.h
//  Description: 
//
//	History:
//	-July 25,2001:Created by Alberto Demichelis
//
//////////////////////////////////////////////////////////////////////


#ifndef _SERVER_SLOT_H_
#define _SERVER_SLOT_H_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Interfaces.h"
#include "ServerStateMachine.h"
#ifdef NO_GNB
#include "CTPEndpoint.h"
#else
#include "CTPEndpointGNB.h"
#endif
#include "CCPEndpoint.h"
#include "Network.h"

class ICrySizer;
//
// CServerSlot is released by the CXServerSlot
//

class CServerSlot :public IServerSlot, public _IServerSlotServices
{
public:
	//! constructor
	CServerSlot(CNetwork* pNetwork) :m_pbAuthorizationID(nullptr), m_pSink(0), m_pNetwork(pNetwork)
	{
		m_BandwidthStats.Reset();
		m_pbGlobalID = 0;
	}

	//! destructor
	virtual ~CServerSlot()
	{
		if (m_pbAuthorizationID)
			delete[] m_pbAuthorizationID;
		if (m_pbGlobalID)
			delete[] m_pbGlobalID;
	};

	virtual bool IsActive() = 0;
	virtual void Unlink() = 0;
	virtual void Update(uint nTime, CNP* pCNP, CStream* pStream) = 0;
	virtual CIPAddress& GetIP() { return m_ipAddress; }
	virtual void GetMemoryStatistics(ICrySizer* pSizer) = 0;
	virtual bool SendSecurityQuery(CStream& stm) { return true; };
	virtual bool SendPunkBusterMsg(CStream& stm) { return true; };

	// interface IServerSlot -------------------------------------------------------

	virtual void Advise(IServerSlotSink* pSink) { m_pSink = pSink; }
	virtual void ResetBandwidthStats();
	virtual void GetBandwidthStats(SServerSlotBandwidthStats& out) const;
	virtual uint GetClientIP() const { return m_ipAddress.GetAsUINT(); };
	virtual void OnPlayerAuthorization(bool bAllow, const char* szError, const BYTE* pGlobalID,
		uint uiGlobalIDSize);

	//////////////////////////////////////////////////////////////////////////
	// Returns client specific flags.
	virtual uint GetClientFlags() { return 0; };

	virtual bool IsLocalSlot() const { return false; };

public: // -----------------------------------------------------------------------

	BYTE* m_pbAuthorizationID;				//!< CD Key AuthorizationID (use new and delete)
	uint								m_uiAuthorizationSize;			//!< Size of AuthorizationID in bytes

	BYTE* m_pbGlobalID;								//!< CD Key GlobalID (use new and delete)
	uint								m_uiGlobalIDSize;						//!< Size of GlobalID in bytes

protected: // --------------------------------------------------------------------

	CNetwork* m_pNetwork;									//!<
	CIPAddress									m_ipAddress;								//!<
	uchar								m_cClientID;								//!<	//<<FIXME>> this can be removed in future

	SServerSlotBandwidthStats		m_BandwidthStats;						//!< used for bandwidth calculations (to adjust the bandwidth)
	IServerSlotSink* m_pSink;										//!< connected CXServerSlot (game specific part)
};

/////////////////////////////////////////////////////////////////////////////////////////
//CServerSlot implementation
/////////////////////////////////////////////////////////////////////////////////////////
class CServerSlotImpl :public CServerSlot
{
public:
	//! constructor
	CServerSlotImpl(CNetwork* pNetwork, _IServerServices* pParent);
	//! destructor
	virtual ~CServerSlotImpl();
	//!
	void GetMemoryStatistics(ICrySizer* pSizer);

public: // --------------------------------------------------------------

	//!
	bool IsActive();
	//!
	void Update(uint nTime, CNP* pCNP, CStream* pStream);
	//! unlink from the local server
	void Unlink() { m_pParent = nullptr; }

	// interface IServerSlot -------------------------------------------------------

	virtual void Disconnect(const char* szCause);
	virtual bool ContextSetup(CStream& stm);
	virtual void SendReliable(CStream& stm, bool bSecondaryChannel);
	virtual void SendUnreliable(CStream& stm);
	virtual bool IsReady();
	virtual void Release();
	virtual uint GetPacketsLostCount() { return m_ctpEndpoint.GetLostPackets(); }
	virtual uint GetUnreliablePacketsLostCount();
	virtual uint GetPing();
	virtual uchar GetID() { return m_cClientID; }

	// interface _IServerSlotServices ----------------------------------------------

	virtual void OnConnect();
	virtual void OnContextReady();
	virtual void OnDisconnect(const char* szCause);
	virtual bool SendConnect();
	virtual bool SendContextSetup();
	virtual bool SendDisconnect(const char* szCause);
	virtual bool SendServerReady();
	virtual bool SendSecurityQuery(CStream& stm);
	virtual bool SendPunkBusterMsg(CStream& stm);
	virtual void Start(uchar cClientID, CIPAddress& ip);

	// interface _IEndpointUser ----------------------------------------------------

	virtual bool Send(CStream& stm);
	virtual void OnData(CStream& stm);

	// interface _ICCPUser ---------------------------------------------------------

	virtual void OnCCPSetup(CStream& stm);
	virtual void OnCCPConnect(CStream& stm) {}
	virtual void OnCCPConnectResp(CStream& stm);
	virtual void OnCCPContextSetup(CStream& stm) {}
	virtual void OnCCPContextReady(CStream& stm);
	virtual void OnCCPServerReady() {}
	virtual void OnCCPDisconnect(const char* szCause);
	virtual void OnCCPSecurityQuery(CStream& stm);
	virtual void OnCCPSecurityResp(CStream& stm);
	virtual void OnCCPPunkBusterMsg(CStream& stm);

	virtual uint GetClientFlags() { return m_nClientFlags; };

	virtual bool IsLocalSlot() const { return false; };

private: // --------------------------------------------------------------------------

	void ProcessPacket(uchar cFrameType, bool bSTC, CStream* pStream);

	CServerStateMachine					m_smCCPMachine;					//!<
#ifdef NO_GNB
	CCTPServer									m_ctpEndpoint;					//!<
#else
	CCTPEndpointGNB							m_ctpEndpoint;					//!<
	CCTPEndpointGNB							m_ctpEndpoint2;					//!<
#endif
	CCCPEndpoint								m_ccpEndpoint;					//!<

	DWORD												m_dwKeepAliveTimer;			//!< after 3 second without any incoming packets the connection will be considered lost

	bool												m_bActive;							//!<
	bool												m_bClientAdded;

	CStream											m_stmContextReady;			//!<

	_IServerServices* m_pParent;							//!<
	CStream											m_stmContext;						//!<

	struct CNPServerVariables		m_ServerVariables;			//!<

	uint								m_nCurrentTime;					//!<
	uint								m_nPublicKey;						//!<
	//////////////////////////////////////////////////////////////////////////
	// Client info.
	//////////////////////////////////////////////////////////////////////////
	// Client OS.
	unsigned										m_nClientOS_Minor;			//!<
	unsigned										m_nClientOS_Major;			//!<
	// Client flags.
	uint								m_nClientFlags;					//!<
};

/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
class CClientLocal;
class CServer;

//! fake CServerSlot implementation for local clients, doesn't use the network!*/
class CServerSlotLocal : public CServerSlot
{
public:
	//! constructor
	CServerSlotLocal(CServer* pServer, CClientLocal* pClient, CIPAddress& ip, CNetwork* pNetwork);
	//! destructor
	virtual ~CServerSlotLocal();

	void GetMemoryStatistics(ICrySizer* pSizer);

	bool IsActive() { return true; }
	void Update(uint nTime, CNP* pCNP, CStream* pStream) {};
	void PushData(CStream& stm);
	void UpdateSlot();
	void Unlink() { m_pServer = nullptr; }
	void ResetSink();

	// interface IServerSlot ---------------------------------------------------

	virtual void Disconnect(const char* szCause);
	virtual bool ContextSetup(CStream& stm);
	virtual void SendReliable(CStream& stm, bool bSecondaryChannel);
	virtual void SendUnreliable(CStream& stm);
	virtual bool IsReady() { return ((m_pClient && m_pSink) ? true : false); }
	virtual void Release();
	virtual uint GetPacketsLostCount() { return 0; }
	virtual uint GetUnreliablePacketsLostCount() { return 0; }
	virtual uint GetPing() { return 0; }
	virtual uchar GetID() { return m_cClientID; }

	// interface _IServerSlotServices ------------------------------------------

	virtual void OnConnect() {}
	virtual void OnContextReady();
	virtual void OnDisconnect(const char* szCause);
	virtual bool SendConnect() { return true; }
	virtual bool SendContextSetup() { return true; }
	virtual bool SendDisconnect(const char* szCause) { return true; }
	virtual bool SendServerReady() { return true; }
	virtual void Start(uchar cClientID, CIPAddress& ip) {}

	// interface _IEndpointUser ----------------------------------------------------

	virtual bool Send(CStream& stm) { return true; }
	virtual void OnData(CStream& stm) {}

	// interface _ICCPUser ----------------------------------------------------

	virtual void OnCCPSetup(CStream& stm) {};
	virtual void OnCCPConnect(CStream& stm) {};
	virtual void OnCCPConnectResp(CStream& stm);
	virtual void OnCCPContextSetup(CStream& stm) {};
	virtual void OnCCPContextReady(CStream& stm) { if (m_pSink)m_pSink->OnContextReady(stm); };
	virtual void OnCCPServerReady() {};
	virtual void OnCCPDisconnect(const char* szCause) { if (m_pSink)m_pSink->OnXServerSlotDisconnect(szCause); };
	virtual void OnCCPSecurityQuery(CStream& stm) {};
	virtual void OnCCPSecurityResp(CStream& stm) {};
	virtual void OnCCPPunkBusterMsg(CStream& stm) {};

	virtual bool IsLocalSlot() const { return true; };

private: // -----------------------------------------------------------------

	CClientLocal* m_pClient;						//!<
	CServer* m_pServer;						//!<
	bool								m_bReady;							//!<
	STREAM_QUEUE				m_qData;							//!<
	bool								m_bContextSetup;			//!<
	CStream							m_stmContextSetup;		//!<
	int									m_nUpdateCounter;			//!<
	bool								m_bClientAdded;				//!<
};
#endif // _SERVER_SLOT_H_
