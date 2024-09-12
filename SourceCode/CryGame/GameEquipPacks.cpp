
//////////////////////////////////////////////////////////////////////
//
//	Game source code (c) Crytek 2001-2003
//	
//	File: GameEquicpPacks.cpp
//  
//	History:
//	-October	31,2003: created
//	
//////////////////////////////////////////////////////////////////////

#include "StdAfx.h"

#include "Game.h"
#include "XNetwork.h"
#include "XServer.h"
#include "XClient.h"
#include "UIHud.h"
#include "XPlayer.h"
#include "PlayerSystem.h"
#include "XServer.h"
#include "WeaponSystemEx.h"
#include "ScriptObjectGame.h"
#include "ScriptObjectInput.h"
#include <IEntitySystem.h>

#include "UISystem.h"
#include "ScriptObjectUI.h"

#include "WeaponSystemEx.h"
#include "WeaponClass.h"

//////////////////////////////////////////////////////////////////////////
INameIterator * CXGame::GetAvailableWeaponNames()
{ 
	return m_pWeaponSystemEx->GetAvailableWeapons(); 
}

//////////////////////////////////////////////////////////////////////////
INameIterator * CXGame::GetAvailableProjectileNames()
{ 
	return m_pWeaponSystemEx->GetAvailableProjectiles(); 
}

//////////////////////////////////////////////////////////////////////////
bool CXGame::AddWeapon(const char *pszName)
{
	return m_pWeaponSystemEx->AddWeapon(pszName);
}

//////////////////////////////////////////////////////////////////////////
bool CXGame::RemoveWeapon(const char *pszName)
{
///		return m_pWeaponSystem->RemoveWeapon(pszName, this, m_pServer->m_pISystem);
	return true;
}

//////////////////////////////////////////////////////////////////////////
void CXGame::RemoveAllWeapons()
{
	//m_pWeaponSystem->RemoveAllWeapons(this, m_pServer->m_pISystem);
}

//////////////////////////////////////////////////////////////////////////
bool CXGame::AddEquipPack(const char *pszXML)
{
	XDOM::IXMLDOMDocumentPtr pDoc = m_pSystem->CreateXMLDocument();
	if (!pDoc->loadXML(pszXML))
		return false;
	XDOM::IXMLDOMNodeListPtr pNodes = pDoc->getChildNodes();
	if (pNodes)
	{
		XDOM::IXMLDOMNodePtr pEquipNode;
		pNodes->reset();
		while (pEquipNode = pNodes->nextNode())
			AddEquipPack(pEquipNode);
	}
	else
		return false;
	return true;
}

//////////////////////////////////////////////////////////////////////////
bool CXGame::AddEquipPack(XDOM::IXMLDOMNode *pPack)
{
	_SmartScriptObject cEquipPacks(GetScriptSystem(), true);
	XDOM::IXMLDOMNodePtr pPackName, pPrimaryWeapon;
	GetScriptSystem()->GetGlobalValue("EquipPacks", cEquipPacks);
	pPackName = pPack->getAttribute("name");
	pPrimaryWeapon = pPack->getAttribute("primary");
	if (pPackName)
	{
//		TRACE("Equipment Pack Available: %s", pPackName->getText());
		_SmartScriptObject cWeaponList(GetScriptSystem(), false);
		XDOM::IXMLDOMNodeListPtr pItemList = pPack->getElementsByTagName("Items");
		if (pItemList)
		{
			XDOM::IXMLDOMNodePtr pCurItemList;
			pItemList->reset();
			while (pCurItemList = pItemList->nextNode())
			{
				XDOM::IXMLDOMNodeListPtr pItems = pCurItemList->getChildNodes();
				XDOM::IXMLDOMNodePtr pCurItem;
				pItems->reset();
				UINT iCurItem = 1;

				while (pCurItem = pItems->nextNode())
				{
					XDOM::IXMLDOMNodePtr pIType = pCurItem->getAttribute("type");
					// if (strcmp(pIType->getText(), "Weapon") == 0)
					{
						_SmartScriptObject cEntry(GetScriptSystem(), false);
						cEntry->SetValue("Type", pIType->getText());
						cEntry->SetValue("Name", pCurItem->getName());
						if(pPrimaryWeapon)
						{
							if(strcmp(pPrimaryWeapon->getText(),pCurItem->getName())==0)
							{
								cEntry->SetValue("Primary", 1);
							}
						}
						cWeaponList->SetAt(iCurItem, *cEntry);
						iCurItem++;
						cEquipPacks->SetValue(pPackName->getText(), *cWeaponList);
					}
				}
			}
		}
		_SmartScriptObject cAmmoAvailTable(GetScriptSystem(), false);
	 	XDOM::IXMLDOMNodeListPtr pAmmoTag = pPack->getElementsByTagName("Ammo");
		if (pAmmoTag)
		{
			pAmmoTag->reset();
			XDOM::IXMLDOMNodePtr pAmmoTagFirst = pAmmoTag->nextNode();
			if (pAmmoTagFirst)
			{
				XDOM::IXMLDOMNodeListPtr pChildNodes = pAmmoTagFirst->getChildNodes();
				XDOM::IXMLDOMNodePtr pAmmo;
				pChildNodes->reset();	
				while (pAmmo = pChildNodes->nextNode())
				{
					if (pAmmo->getNodeType() == XDOM::NODE_ATTRIBUTE)
					{
						//TRACE("Ammo: %s = %s", pAmmo->getName(), pAmmo->getText());
						cAmmoAvailTable->SetValue(pAmmo->getName(), atoi(pAmmo->getText()));
					}
				}
			}
		}
		cWeaponList->SetValue("Ammo", *cAmmoAvailTable);
	}
	RestoreWeaponPacks();
	return true;
}

//////////////////////////////////////////////////////////////////////////
void CXGame::SetPlayerEquipPackName(const char *pszPackName)
{ 
	// Pass NULL or "" to remove all equipment from the player
	IScriptSystem *pIScriptSystem = GetScriptSystem();
	IEntity *pIMyPlayer = (m_pServer != nullptr) ? m_pServer->m_pISystem->GetLocalPlayer() : 
		m_pClient->m_pISystem->GetLocalPlayer();
	void *pContainer = nullptr;
	IEntityContainer *pIContainer = nullptr;
	CPlayer *pPlayer = nullptr;

	// Store the name of the equipment pack
	IScriptObject *pGlobals=pIScriptSystem->GetGlobalObject();
	
	pGlobals->SetValue("MainPlayerEquipPack", (pszPackName != nullptr) ? pszPackName : "");

	pGlobals->Release();

	if (pIMyPlayer == nullptr)
		return;

	if (pszPackName == nullptr || strlen(pszPackName) == 0)
	{
		// No equipment pack specified, remove all weapons
		pIContainer = pIMyPlayer->GetContainer();
		if (pIContainer == nullptr)
			return;
		if (pIContainer->QueryContainerInterface(CIT_IPLAYER, &pContainer))
		{
			pPlayer = (CPlayer *) pContainer;
			pPlayer->RemoveAllWeapons();
		}
		return;
	}

	// Let the player reload his weapons
	pIScriptSystem->BeginCall("BasicPlayer", "InitAllWeapons");
	pIScriptSystem->PushFuncParam(pIMyPlayer->GetScriptObject());
	pIScriptSystem->PushFuncParam((int)1);
	pIScriptSystem->EndCall();
}

//////////////////////////////////////////////////////////////////////////
void CXGame::RestoreWeaponPacks()
{
	IEntityIt *pEtyIt = nullptr;
	IEntity *pICurEty = nullptr;
	void *pContainer = nullptr;
	IEntityContainer *pIContainer = nullptr;

	// start .. temporary checks to find a bug 
	if(m_pServer)
	{
		if(!m_pServer->m_pISystem)
			{ m_pLog->LogError("CXGame::RestoreWeaponPacks Error1");return; }
	}
	else 
	{
		if(!m_pClient)
			{ m_pLog->LogError("CXGame::RestoreWeaponPacks Error2");return; }

		if(!m_pClient->m_pISystem)
			{ m_pLog->LogError("CXGame::RestoreWeaponPacks Error3");return; }
	}
	// end .. temporary checks to find a bug 


	pEtyIt = (m_pServer ? m_pServer->m_pISystem : m_pClient->m_pISystem)->GetEntities();	
	while (pICurEty = pEtyIt->Next())
	{
		pIContainer = pICurEty->GetContainer();
		if (pIContainer == nullptr)
			continue;
		if (pIContainer->QueryContainerInterface(CIT_IPLAYER, &pContainer))
		{
			IScriptSystem *pIScriptSystem = GetScriptSystem();
			pIScriptSystem->BeginCall("BasicPlayer", "InitAllWeapons");
			pIScriptSystem->PushFuncParam(pICurEty->GetScriptObject());
			pIScriptSystem->PushFuncParam((int)1);
			pIScriptSystem->EndCall();
  		}
	}
	pEtyIt->Release();
}

void CXGame::ReloadWeaponScripts()
{
	if (m_pWeaponSystemEx)
	{
		std::vector<CPlayer*> vPlayers;
		
		CPlayer *pPlayer = nullptr;
		IEntity *pICurEty = nullptr;

		IEntityIt *pEtyIt = (m_pServer ? m_pServer->m_pISystem : m_pClient->m_pISystem)->GetEntities();	
		while (pICurEty = pEtyIt->Next())
		{
			IEntityContainer *pIContainer = pICurEty->GetContainer();
			if (pIContainer == nullptr)
				continue;
			if (pIContainer->QueryContainerInterface(CIT_IPLAYER, (void**) &pPlayer))
			{
				vPlayers.push_back(pPlayer);
			}
		}
		pEtyIt->Release();

		for (std::vector<CPlayer*>::iterator i = vPlayers.begin(); i != vPlayers.end(); ++i)
		{
			(*i)->SelectWeapon(-1, false);
		}

		m_pWeaponSystemEx->Reset();
		m_pWeaponSystemEx->Init(this, true);

		// reload weapons
		_SmartScriptObject soWeaponLoadedTable(m_pScriptSystem, true);

		if (m_pScriptSystem->GetGlobalValue("WeaponsLoaded", soWeaponLoadedTable))
		{
			soWeaponLoadedTable->BeginIteration();
			while(soWeaponLoadedTable->MoveNext())
			{
				const char *sClassName;
				if (soWeaponLoadedTable->GetCurrentKey(sClassName))
				{
					CWeaponClass *pWC = m_pWeaponSystemEx->GetWeaponClassByName(sClassName);
					if (pWC)
					{
						pWC->Load();
					}
				}
			}
			soWeaponLoadedTable->EndIteration();
		}

		for (std::vector<CPlayer*>::iterator i = vPlayers.begin(); i != vPlayers.end(); ++i)
		{
			pPlayer = (*i);
			pPlayer->SelectFirstWeapon();
			if (!pPlayer->IsMyPlayer())
				pPlayer->DrawThirdPersonWeapon(true);

			IScriptSystem *pIScriptSystem = GetScriptSystem();
			pIScriptSystem->BeginCall("BasicPlayer", "InitAllWeapons");
			pIScriptSystem->PushFuncParam(pPlayer->GetEntity()->GetScriptObject());
			pIScriptSystem->PushFuncParam((int)1);
			pIScriptSystem->EndCall();
		}
		vPlayers.clear();
	}
}
