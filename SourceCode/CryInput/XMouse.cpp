// XMouse.cpp: implementation of the CXMouse class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "XMouse.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
#include <math.h>
#include <stdio.h>
#include <ILog.h>
#include <ISystem.h>
#include <IConsole.h>
#include "InputDirectInput.h"
#include <IGame.h>

///////////////////////////////////////////
bool CXMouse::Init(CInputDirectInput* pInput, ISystem* pSystem, LPDIRECTINPUT8& g_pdi, HINSTANCE hinst, HWND hwnd, bool dinput)
{
	m_pInput = pInput;
	m_pSystem = pSystem;
	m_pLog = pSystem->GetILog();
	m_pTimer = pSystem->GetITimer();

	i_mouse_buffered = m_pSystem->GetIConsole()->CreateVariable("i_mouse_buffered", "0", 0,
		"Toggles mouse input buffering.\n"
		"Usage: i_mouse_buffered [0/1]\n"
		"Default is 0 (off). Set to 1 to process buffered mouse input.");

	//mouse accel
	i_mouse_accel = m_pSystem->GetIConsole()->CreateVariable("i_mouse_accel", "0.0", VF_DUMPTODISK,
		"Set mouse acceleration, 0.0 means no acceleration.\n"
		"Usage: i_mouse_accel [float number] (usually a small number, 0.1 is a good one)\n"
		"Default is 0.0 (off)");

	//mouse accel cap
	i_mouse_accel_max = m_pSystem->GetIConsole()->CreateVariable("i_mouse_accel_max", "100.0", VF_DUMPTODISK,
		"Set mouse max mouse delta when using acceleration.\n"
		"Usage: i_mouse_accel_max [float number]\n"
		"Default is 100.0");

	//mouse smooth
	i_mouse_smooth = m_pSystem->GetIConsole()->CreateVariable("i_mouse_smooth", "0.0", VF_DUMPTODISK,
		"Set mouse smoothing value, also if 0 (disabled) there will be a simple average between the old and the actual input.\n"
		"Usage: i_mouse_smooth [float number] (1.0 = very very smooth, 30 = almost istant)\n"
		"Default is 0.0");

	//mouse mirror
	i_mouse_mirror = m_pSystem->GetIConsole()->CreateVariable("i_mouse_mirror", "0", VF_DUMPTODISK,
		"Set mouse mirroring, if not 0 the mouse input will be mirrored.\n"
		"Usage: i_mouse_smooth [0 or 1]\n"
		"Default is 0");

	i_mouse_sensitivity_scale = m_pSystem->GetIConsole()->CreateVariable("i_mouse_sensitivity_scale", "1", VF_DUMPTODISK,
																		"Sets mouse sensitivity scale.\n"
																		"Usage: i_mouse_sensitivity_scale [0...1]\n"
																		"Default is 1");

	i_mouse_dynamic_sensitivity = m_pSystem->GetIConsole()->CreateVariable("i_mouse_dynamic_sensitivity", "1", VF_DUMPTODISK,
																			"Allows or disallows dynamic sensitivity changes (made by SetScale).\n"
																			"Usage: i_mouse_dynamic_sensitivity [0 or 1]\n"
																			"Default is 1");


	HINSTANCE	ghInstance = hinst;
	HWND		ghWnd = hwnd;

	HRESULT hr;

	m_pLog->Log("Initializing mouse\n");

	m_fVScreenX = 400.0f;
	m_fVScreenY = 300.0f;

	m_fDblClickTime = 0.2f;

	m_dinput = dinput;
	m_hwnd = hwnd;

	m_fSensitivity = 0.2f;
	m_fSensitivityScale = 1;

	m_fLastRelease[0] = m_fLastRelease[1] = m_fLastRelease[2] = 0.0f;

	if (m_dinput)
	{
		hr = g_pdi->CreateDevice(GUID_SysMouse, &m_pMouse, NULL);
		if (FAILED(hr))
		{
			m_pLog->Log("Cannot Create Mouse Device\n");
			return (false);
		}

		hr = m_pMouse->SetDataFormat(&c_dfDIMouse2);
		if (FAILED(hr))
		{
			m_pLog->Log("Cannot Set Mouse Data Format\n");
			return (false);
		}

		// Set property of mouse buffer.
		DIPROPDWORD Property;

		Property.diph.dwSize = sizeof(DIPROPDWORD);
		Property.diph.dwHeaderSize = sizeof(DIPROPHEADER);
		Property.diph.dwObj = 0;
		Property.diph.dwHow = DIPH_DEVICE;

		// Size of buffer.
		Property.dwData = 4096;

		hr = m_pMouse->SetProperty(DIPROP_BUFFERSIZE, &Property.diph);
		if (FAILED(hr))
		{
			CryError("CXMouse::Init SetProperty DIPROP_BUFFERSIZE faied.");
			return false;
		}

		// Set mouse axis mode.
		Property.dwData = DIPROPAXISMODE_REL;
		hr = m_pMouse->SetProperty(DIPROP_AXISMODE, &Property.diph);
		if (FAILED(hr))
		{
			CryError("CXMouse::Init SetProperty DIPROP_AXISMODE faied.");
			return false;
		}

		hr = m_pMouse->SetCooperativeLevel(ghWnd, DISCL_EXCLUSIVE | DISCL_FOREGROUND);
		if (FAILED(hr))
		{
			m_pLog->Log("Cannot Set Mouse Cooperative Level\n");
			return (false);
		}
	}
	else
	{
		SetCursorPos(GetSystemMetrics(SM_CXFULLSCREEN) / 2, GetSystemMetrics(SM_CYFULLSCREEN) / 2);
		m_previouspoint.x = GetSystemMetrics(SM_CXFULLSCREEN) / 2;
		m_previouspoint.y = GetSystemMetrics(SM_CYFULLSCREEN) / 2;
	}

	memset(m_Deltas, 0, sizeof(m_Deltas));
	//smooth
	memset(m_OldDeltas, 0, sizeof(m_OldDeltas));

	memset(m_DeltasInertia, 0, sizeof(m_Deltas));
	memset(m_Events, 0, sizeof(m_Events));

	if (m_dinput)
		Acquire();

	m_wheelChecked = false;

	return (true);
}

float CXMouse::GetDeltaX()
{
	float sensitivity = m_fSensitivity * i_mouse_sensitivity_scale->GetFVal();
	if (i_mouse_dynamic_sensitivity->GetIVal())
		sensitivity *= m_fSensitivityScale;

	return m_Deltas[0] * sensitivity;
}

float CXMouse::GetDeltaY()
{
	float sensitivity = m_fSensitivity * i_mouse_sensitivity_scale->GetFVal();
	if (i_mouse_dynamic_sensitivity->GetIVal())
		sensitivity *= m_fSensitivityScale;

	return m_Deltas[1] * sensitivity;
}

float CXMouse::GetDeltaZ()
{
	float sensitivity = m_fSensitivity * i_mouse_sensitivity_scale->GetFVal();
	if (i_mouse_dynamic_sensitivity->GetIVal())
		sensitivity *= m_fSensitivityScale;

	m_wheelChecked = true;
	return m_Deltas[2] * sensitivity;
}

///////////////////////////////////////////
void CXMouse::Update(bool bPrevFocus)
{
	if (m_dinput)
	{
		HRESULT hr;

		if (!m_pMouse)
			return;

		if (i_mouse_buffered->GetIVal() != 0)
		{
			memset(m_Deltas, 0, sizeof(m_Deltas));
			memcpy(m_oldEvents, m_Events, sizeof(m_Events));
			memset(m_Events, 0, sizeof(m_Events));

			float mouseDelta[6];
			memset(mouseDelta, 0, sizeof(mouseDelta));

			DIDEVICEOBJECTDATA Events[200];
			// // Buffer mouse input.
			DWORD nElements = 200;
			while (nElements > 0)
			{
				hr = m_pMouse->GetDeviceData(sizeof(DIDEVICEOBJECTDATA), Events, &nElements, 0);
				if (FAILED(hr))
				{
					m_pMouse->Acquire();
					nElements = 0;
					break;
				}
				if (nElements > 0)
				{
					for (int i = 0; i < (int)nElements; i++)
					{
						switch (Events[i].dwOfs)
						{
						case DIMOFS_X:
							mouseDelta[0] += float((int)Events[i].dwData);
							break;
						case DIMOFS_Y:
							mouseDelta[1] += float((int)Events[i].dwData);
							break;
						case DIMOFS_Z:
							mouseDelta[2] += float((int)Events[i].dwData);
							break;
						case DIMOFS_BUTTON0:
							m_Events[0] = (unsigned char)Events[i].dwData;
							break;
						case DIMOFS_BUTTON1:
							m_Events[1] = (unsigned char)Events[i].dwData;
							break;
						case DIMOFS_BUTTON2:
							m_Events[2] = (unsigned char)Events[i].dwData;
							break;
						case DIMOFS_BUTTON3:
							m_Events[3] = (unsigned char)Events[i].dwData;
							break;
						case DIMOFS_BUTTON4:
							m_Events[4] = (unsigned char)Events[i].dwData;
							break;
						case DIMOFS_BUTTON5:
							m_Events[5] = (unsigned char)Events[i].dwData;
							break;
						case DIMOFS_BUTTON6:
							m_Events[6] = (unsigned char)Events[i].dwData;
							break;
						case DIMOFS_BUTTON7:
							m_Events[7] = (unsigned char)Events[i].dwData;
							break;
						}
					}
				}
			}

			char str[256];
			sprintf(str, "Z: %g\n", mouseDelta[2]);
			OutputDebugString(str);

			m_Deltas[0] = mouseDelta[0];
			m_Deltas[1] = mouseDelta[1];
			m_Deltas[2] = mouseDelta[2];

			float mouseaccel = i_mouse_accel->GetFVal();

			//if(m_bAcceleration)
			if (mouseaccel > 0.0001f)
			{
				m_Deltas[0] = m_Deltas[0] * (float)fabs(m_Deltas[0] * mouseaccel/*m_fAccelerationScale*/);
				m_Deltas[1] = m_Deltas[1] * (float)fabs(m_Deltas[1] * mouseaccel/*m_fAccelerationScale*/);

				CapDeltas(i_mouse_accel_max->GetFVal());

				//SmoothDeltas(i_mouse_smooth->GetFVal(),i_mouse_smooth->GetFVal()*2.0f);
			}

			/*
			if (m_Deltas[0] != 0)
			{
				m_pLog->LogToConsole( "DeltaX=%.04f",m_Deltas[0] );
			}
			*/
		}
		else
		{
			// Not buffered.
			DIMOUSESTATE2 dims;
			hr = m_pMouse->GetDeviceState(sizeof(DIMOUSESTATE2), &dims);

			if (hr == DIERR_INPUTLOST)
			{
				hr = m_pMouse->Acquire();

				if (SUCCEEDED(hr))
				{
					hr = m_pMouse->GetDeviceState(sizeof(DIMOUSESTATE2), &dims);
					if (hr == DIERR_INPUTLOST)
						return;
				}
			}

			if (SUCCEEDED(hr))
			{

				memset(m_Deltas, 0, sizeof(m_Deltas));
				memcpy(m_oldEvents, m_Events, sizeof(m_Events));
				memset(m_Events, 0, sizeof(m_Events));
				//			m_pLog->Log("x=%d,y=%d,z=%d",dims.lX,dims.lY,dims.lZ);

				float mouseaccel = i_mouse_accel->GetFVal();

				//if(m_bAcceleration)
				if (mouseaccel > 0.0001f)
				{
					m_Deltas[0] = (float)dims.lX * (float)fabs((float)dims.lX * mouseaccel/*m_fAccelerationScale*/);
					m_Deltas[1] = (float)dims.lY * (float)fabs((float)dims.lY * mouseaccel/*m_fAccelerationScale*/);

					CapDeltas(i_mouse_accel_max->GetFVal());
				}
				else
				{
					m_Deltas[0] = (float)dims.lX;
					m_Deltas[1] = (float)dims.lY;
				}

				m_Deltas[2] = (float)dims.lZ;
				for (int but = 0; but < 8; but++)
				{
					m_Events[but] = dims.rgbButtons[but];
					//	m_pLog->LogToConsole("MOUSE %c %c %c %c",m_Events[0],m_Events[1],m_Events[2],m_Events[3]);

				}
				/*
				if (m_Deltas[0] != 0)
				{
					m_pLog->LogToConsole( "DeltaX=%.04f",m_Deltas[0] );
				}
				*/
			}
		}

		POINT p;
		p.x = GetSystemMetrics(SM_CXFULLSCREEN) / 2;
		p.y = GetSystemMetrics(SM_CYFULLSCREEN) / 2;
		SetCursorPos(p.x, p.y);
		SetCursor(NULL);
	}
	else
	{
		POINT tp;
		RECT r;

		m_Deltas[0] = 0;
		m_Deltas[1] = 0;
		if (m_wheelChecked)
		{
			m_wheelChecked = false;
			m_Deltas[2] = 0;
		}

		memcpy(m_oldEvents, m_Events, sizeof(m_Events));
		memset(m_Events, 0, sizeof(m_Events));
		/* On commence par regarder l'etat des boutons */
		m_Events[0] = GetAsyncKeyState(VK_LBUTTON) ? 0x80 : 0;
		m_Events[1] = GetAsyncKeyState(VK_RBUTTON) ? 0x80 : 0;
		m_Events[2] = GetAsyncKeyState(VK_MBUTTON) ? 0x80 : 0;
		m_Events[3] = 0;

		/* Pi, on regarde la position de la souris */
		GetCursorPos(&tp);
		GetWindowRect(m_hwnd, &r);

		// For editor GetFocus() returns other window!!!
		//Timur[11/16/2001] if((tp.x < r.left) ||(tp.x > r.right) || (tp.y < r.top) || (tp.y > r.bottom) ||(GetFocus() != m_hwnd))
		if ((tp.x < r.left) || (tp.x > r.right) || (tp.y < r.top) || (tp.y > r.bottom))
		{
			return;
		}

		POINT p;
		p.x = GetSystemMetrics(SM_CXFULLSCREEN) / 2;
		p.y = GetSystemMetrics(SM_CYFULLSCREEN) / 2;
		SetCursorPos(p.x, p.y);
		SetCursor(NULL);

		if (bPrevFocus)
		{
			float d0 = float(tp.x - p.x);
			float d1 = float(tp.y - p.y);
			//m_Deltas[0] = float(tp.x - p.x);
			//m_Deltas[1] = float(tp.y - p.y);

			float mouseaccel = i_mouse_accel->GetFVal();

			//if(m_bAcceleration)
			if (mouseaccel > 0.0001f)
			{
				m_Deltas[0] = d0 * fabs(d0 * mouseaccel);
				m_Deltas[1] = d1 * fabs(d1 * mouseaccel);

				CapDeltas(i_mouse_accel_max->GetFVal());
			}
			else
			{
				m_Deltas[0] = d0;
				m_Deltas[1] = d1;
			}
		}
	}

	SmoothDeltas(i_mouse_smooth->GetFVal());

	//mouse mirror? only in game, not while using the menu.
	if (i_mouse_mirror->GetIVal())
	{
		IGame* p_game = m_pSystem->GetIGame();

#ifndef _ISNOTFARCRY
		if (!p_game || !GetIXGame(p_game)->IsInMenu())
		{
			m_Deltas[0] = -m_Deltas[0];
			m_Deltas[1] = -m_Deltas[1];
		}
#endif
	}

	//////////////////////////////////////////////////////////////////////////
	// Post mouse events.
	//////////////////////////////////////////////////////////////////////////
	if (m_Deltas[0] != 0)
	{
		PostEvent(XKEY_MAXIS_X, SInputEvent::MOUSE_MOVE, m_Deltas[0]);
	}
	if (m_Deltas[1] != 0)
	{
		PostEvent(XKEY_MAXIS_Y, SInputEvent::MOUSE_MOVE, m_Deltas[1]);
	}
	if (m_Deltas[2] > 0)
	{
		PostEvent(XKEY_MWHEEL_UP, SInputEvent::MOUSE_MOVE, m_Deltas[2]);
	}
	if (m_Deltas[2] < 0)
	{
		PostEvent(XKEY_MWHEEL_DOWN, SInputEvent::MOUSE_MOVE, m_Deltas[2]);
	}

	int nkey = XKEY2IDX(XKEY_MOUSE1);
	if (m_Events[nkey] != m_oldEvents[nkey])
	{
		if (m_Events[nkey] & 0x80)
			PostEvent(XKEY_MOUSE1, SInputEvent::KEY_PRESS);
		else
			PostEvent(XKEY_MOUSE1, SInputEvent::KEY_RELEASE);
	}
	nkey = XKEY2IDX(XKEY_MOUSE2);
	if (m_Events[nkey] != m_oldEvents[nkey])
	{
		if (m_Events[nkey] & 0x80)
			PostEvent(XKEY_MOUSE2, SInputEvent::KEY_PRESS);
		else
			PostEvent(XKEY_MOUSE2, SInputEvent::KEY_RELEASE);
	}
	nkey = XKEY2IDX(XKEY_MOUSE3);
	if (m_Events[nkey] != m_oldEvents[nkey])
	{
		if (m_Events[nkey] & 0x80)
			PostEvent(XKEY_MOUSE3, SInputEvent::KEY_PRESS);
		else
			PostEvent(XKEY_MOUSE3, SInputEvent::KEY_RELEASE);
	}
	nkey = XKEY2IDX(XKEY_MOUSE4);
	if (m_Events[nkey] != m_oldEvents[nkey])
	{
		if (m_Events[nkey] & 0x80)
			PostEvent(XKEY_MOUSE4, SInputEvent::KEY_PRESS);
		else
			PostEvent(XKEY_MOUSE4, SInputEvent::KEY_RELEASE);
	}
	nkey = XKEY2IDX(XKEY_MOUSE5);
	if (m_Events[nkey] != m_oldEvents[nkey])
	{
		if (m_Events[nkey] & 0x80)
			PostEvent(XKEY_MOUSE5, SInputEvent::KEY_PRESS);
		else
			PostEvent(XKEY_MOUSE5, SInputEvent::KEY_RELEASE);
	}
	nkey = XKEY2IDX(XKEY_MOUSE6);
	if (m_Events[nkey] != m_oldEvents[nkey])
	{
		if (m_Events[nkey] & 0x80)
			PostEvent(XKEY_MOUSE6, SInputEvent::KEY_PRESS);
		else
			PostEvent(XKEY_MOUSE6, SInputEvent::KEY_RELEASE);
	}
	nkey = XKEY2IDX(XKEY_MOUSE7);
	if (m_Events[nkey] != m_oldEvents[nkey])
	{
		if (m_Events[nkey] & 0x80)
			PostEvent(XKEY_MOUSE7, SInputEvent::KEY_PRESS);
		else
			PostEvent(XKEY_MOUSE7, SInputEvent::KEY_RELEASE);
	}
	nkey = XKEY2IDX(XKEY_MOUSE8);
	if (m_Events[nkey] != m_oldEvents[nkey])
	{
		if (m_Events[nkey] & 0x80)
			PostEvent(XKEY_MOUSE8, SInputEvent::KEY_PRESS);
		else
			PostEvent(XKEY_MOUSE8, SInputEvent::KEY_RELEASE);
	}
	//////////////////////////////////////////////////////////////////////////

   //GENERATE EVENTS FOR WHEEL AND AXES
   //mouse wheel
	if (m_Deltas[2] > 0.0f)
	{
		m_Events[XKEY2IDX(XKEY_MWHEEL_UP)] = 0x80;
	}
	else
	{
		m_Events[XKEY2IDX(XKEY_MWHEEL_UP)] = 0;
	}

	if (m_Deltas[2] < 0.0f)
	{
		m_Events[XKEY2IDX(XKEY_MWHEEL_DOWN)] = 0x80;
	}
	else
	{
		m_Events[XKEY2IDX(XKEY_MWHEEL_DOWN)] = 0;
	}

	nkey = XKEY2IDX(XKEY_MWHEEL_UP);
	if (m_Events[nkey] != m_oldEvents[nkey])
	{
		if (m_Events[nkey] & 0x80)
			PostEvent(XKEY_MWHEEL_UP, SInputEvent::KEY_PRESS);
		else
			PostEvent(XKEY_MWHEEL_UP, SInputEvent::KEY_RELEASE);
	}

	nkey = XKEY2IDX(XKEY_MWHEEL_DOWN);
	if (m_Events[nkey] != m_oldEvents[nkey])
	{
		if (m_Events[nkey] & 0x80)
			PostEvent(XKEY_MWHEEL_DOWN, SInputEvent::KEY_PRESS);
		else
			PostEvent(XKEY_MWHEEL_DOWN, SInputEvent::KEY_RELEASE);
	}

	//mouse axis
	if (fabs(m_Deltas[0]) > 1e-5f)
		m_Events[XKEY2IDX(XKEY_MAXIS_X)] = 0x80;
	if (fabs(m_Deltas[1]) > 1e-5f)
		m_Events[XKEY2IDX(XKEY_MAXIS_Y)] = 0x80;

	m_kInertia = 0;		// NO INERTIA ANYMORE...
	// mouse inertia
	if (m_kInertia > 0)
	{
		float dt = m_pTimer->GetFrameTime();
		if (dt > 0.1f) dt = 0.1f;
		for (int i = 0; i < 2; i++)
			m_Deltas[i] = (m_DeltasInertia[i] += (m_Deltas[i] - m_DeltasInertia[i]) * m_kInertia * dt);
	}

	// Update the Virtual Screen position
	m_fVScreenX += GetDeltaX() * 4.0f;
	m_fVScreenY += GetDeltaY() * 4.0f;

	if (m_fVScreenX < 0)
		m_fVScreenX = 0;
	else if (m_fVScreenX > 800 - 1)
		m_fVScreenX = 800 - 1;

	if (m_fVScreenY < 0)
		m_fVScreenY = 0;
	else if (m_fVScreenY > 600 - 1)
		m_fVScreenY = 600 - 1;

	int nIdx;
	nIdx = XKEY2IDX(XKEY_MOUSE1);
	if (((m_Events[nIdx] & 0x80) == 0) && ((m_oldEvents[nIdx] & 0x80) != 0))
	{
		if (!m_bDblClick[nIdx])
			m_fLastRelease[nIdx] = m_pTimer->GetAsyncCurTime();
		else
			m_bDblClick[nIdx] = false;
	}
	nIdx = XKEY2IDX(XKEY_MOUSE2);
	if (((m_Events[nIdx] & 0x80) == 0) && ((m_oldEvents[nIdx] & 0x80) != 0))
	{
		if (!m_bDblClick[nIdx])
			m_fLastRelease[nIdx] = m_pTimer->GetAsyncCurTime();
		else
			m_bDblClick[nIdx] = false;
	}
	nIdx = XKEY2IDX(XKEY_MOUSE3);
	if (((m_Events[nIdx] & 0x80) == 0) && ((m_oldEvents[nIdx] & 0x80) != 0))
	{
		if (!m_bDblClick[nIdx])
			m_fLastRelease[nIdx] = m_pTimer->GetAsyncCurTime();
		else
			m_bDblClick[nIdx] = false;
	}
	nIdx = XKEY2IDX(XKEY_MOUSE4);
	if (((m_Events[nIdx] & 0x80) == 0) && ((m_oldEvents[nIdx] & 0x80) != 0))
	{
		if (!m_bDblClick[nIdx])
			m_fLastRelease[nIdx] = m_pTimer->GetAsyncCurTime();
		else
			m_bDblClick[nIdx] = false;
	}
	nIdx = XKEY2IDX(XKEY_MOUSE5);
	if (((m_Events[nIdx] & 0x80) == 0) && ((m_oldEvents[nIdx] & 0x80) != 0))
	{
		if (!m_bDblClick[nIdx])
			m_fLastRelease[nIdx] = m_pTimer->GetAsyncCurTime();
		else
			m_bDblClick[nIdx] = false;
	}
	nIdx = XKEY2IDX(XKEY_MOUSE6);
	if (((m_Events[nIdx] & 0x80) == 0) && ((m_oldEvents[nIdx] & 0x80) != 0))
	{
		if (!m_bDblClick[nIdx])
			m_fLastRelease[nIdx] = m_pTimer->GetAsyncCurTime();
		else
			m_bDblClick[nIdx] = false;
	}
	nIdx = XKEY2IDX(XKEY_MOUSE7);
	if (((m_Events[nIdx] & 0x80) == 0) && ((m_oldEvents[nIdx] & 0x80) != 0))
	{
		if (!m_bDblClick[nIdx])
			m_fLastRelease[nIdx] = m_pTimer->GetAsyncCurTime();
		else
			m_bDblClick[nIdx] = false;
	}
	nIdx = XKEY2IDX(XKEY_MOUSE8);
	if (((m_Events[nIdx] & 0x80) == 0) && ((m_oldEvents[nIdx] & 0x80) != 0))
	{
		if (!m_bDblClick[nIdx])
			m_fLastRelease[nIdx] = m_pTimer->GetAsyncCurTime();
		else
			m_bDblClick[nIdx] = false;
	}
}

void CXMouse::ClearKeyState()
{
	memset(m_fLastRelease, 0, sizeof(m_fLastRelease));
	memset(m_Events, 0, sizeof(m_Events));
	memset(m_oldEvents, 0, sizeof(m_oldEvents));
	memset(m_bDblClick, 0, sizeof(m_bDblClick));
}

///////////////////////////////////////////	
bool CXMouse::Acquire()
{
	if (m_dinput)
	{
		if (m_pMouse->Acquire())
			return (true);
	}

	return (false);
}

///////////////////////////////////////////
bool CXMouse::UnAcquire()
{
	if (m_dinput)
	{
		if (m_pMouse && m_pMouse->Unacquire())
			return (true);
	}

	return (false);
}

///////////////////////////////////////////
int CXMouse::XKEY2IDX(int nKey)
{
	switch (nKey)
	{
	case XKEY_MOUSE1: return 0;
	case XKEY_MOUSE2: return 1;
	case XKEY_MOUSE3: return 2;
	case XKEY_MOUSE4: return 3;
	case XKEY_MOUSE5: return 4;
	case XKEY_MOUSE6: return 5;
	case XKEY_MOUSE7: return 6;
	case XKEY_MOUSE8: return 7;
	case XKEY_MWHEEL_UP: return 8;
	case XKEY_MWHEEL_DOWN: return 9;
	case XKEY_MAXIS_X: return 10;
	case XKEY_MAXIS_Y: return 11;
	};

	return -1;
}

///////////////////////////////////////////
bool CXMouse::MouseDown(int p_numButton)
{
	return ((m_Events[XKEY2IDX(p_numButton)] & 0x80) != 0);
}

///////////////////////////////////////////
bool CXMouse::MousePressed(int p_numButton)
{
	int nIdx = XKEY2IDX(p_numButton);
	return ((m_Events[nIdx] & 0x80) != 0) && ((m_oldEvents[nIdx] & 0x80) == 0);
}

///////////////////////////////////////////
bool CXMouse::MouseDblClick(int p_numButton)
{
	int nIdx = XKEY2IDX(p_numButton);
	if ((nIdx <= 2) && ((m_Events[nIdx] & 0x80) != 0) && ((m_oldEvents[nIdx] & 0x80) == 0))
	{
		if ((m_pTimer->GetAsyncCurTime() - m_fLastRelease[nIdx]) <= m_fDblClickTime)
		{
			m_fLastRelease[nIdx] = 0.0f;
			m_bDblClick[nIdx] = true;
			return true;
		}
	}
	return false;
}

///////////////////////////////////////////
bool CXMouse::MouseReleased(int p_numButton)
{
	int nIdx = XKEY2IDX(p_numButton);
	return ((m_Events[nIdx] & 0x80) == 0) && ((m_oldEvents[nIdx] & 0x80) != 0);
}

bool CXMouse::GetOSKeyName(int nKey, wchar_t* szwKeyName, int iBufSize)
{
	if (IS_MOUSE_KEY(nKey) && m_pMouse)
	{
		DIDEVICEOBJECTINSTANCE dido;

		ZeroMemory(&dido, sizeof(DIDEVICEOBJECTINSTANCE));
		dido.dwSize = sizeof(DIDEVICEOBJECTINSTANCE);
		int iOffset = 0;

		switch (nKey)
		{
		case XKEY_MOUSE1:
			iOffset = DIMOFS_BUTTON0;
			break;
		case XKEY_MOUSE2:
			iOffset = DIMOFS_BUTTON1;
			break;
		case XKEY_MOUSE3:
			iOffset = DIMOFS_BUTTON2;
			break;
		case XKEY_MOUSE4:
			iOffset = DIMOFS_BUTTON3;
			break;
		case XKEY_MOUSE5:
			iOffset = DIMOFS_BUTTON4;
			break;
		case XKEY_MOUSE6:
			iOffset = DIMOFS_BUTTON5;
			break;
		case XKEY_MOUSE7:
			iOffset = DIMOFS_BUTTON6;
			break;
		case XKEY_MOUSE8:
			iOffset = DIMOFS_BUTTON7;
			break;
		case XKEY_MWHEEL_UP:
			iOffset = DIMOFS_Z;
			break;
		case XKEY_MWHEEL_DOWN:
			iOffset = DIMOFS_Z;
			break;
		case XKEY_MAXIS_X:
			iOffset = DIMOFS_X;
			break;
		case XKEY_MAXIS_Y:
			iOffset = DIMOFS_Y;
			break;
		}

		if (m_pMouse->GetObjectInfo(&dido, iOffset, DIPH_BYOFFSET) != DI_OK)
		{
			return false;
		}

		wchar_t szwMyKeyName[256] = { 0 };

		switch (nKey)
		{
		case XKEY_MWHEEL_UP:
			swprintf(szwMyKeyName, L"%S Up", dido.tszName);
			break;
		case XKEY_MWHEEL_DOWN:
			swprintf(szwMyKeyName, L"%S Down", dido.tszName);
			break;
		default:
			swprintf(szwMyKeyName, L"%S", dido.tszName);
		}

		wcsncpy(szwKeyName, szwMyKeyName, iBufSize);

		return true;
			}

	return false;
		}

///////////////////////////////////////////
void CXMouse::Shutdown()
{
	m_pLog->LogToFile("Mouse Shutdown\n");
	UnAcquire();

	if (m_pMouse)
		m_pMouse->Release();
	m_pMouse = NULL;

}

///////////////////////////////////////////
bool CXMouse::SetExclusive(bool value, void* hwnd)
{

	if (hwnd)
		m_hwnd = (HWND)hwnd;

	if (m_dinput)
	{
		if (!m_pMouse)
			return (false);

		HRESULT hr;

		HWND wind = m_hwnd;

		if (value)
		{
			UnAcquire();
			hr = m_pMouse->SetCooperativeLevel(wind, DISCL_EXCLUSIVE | DISCL_FOREGROUND);

			if (FAILED(hr))
			{
				m_pLog->Log("Cannot Set Mouse Exclusive Mode\n");
				Acquire();
				return (false);
			}
		}
		else
		{
			UnAcquire();
			hr = m_pMouse->SetCooperativeLevel(wind, DISCL_NONEXCLUSIVE | DISCL_FOREGROUND);
			if (FAILED(hr))
			{
				m_pLog->Log("Cannot Set Mouse Non-Exclusive Device\n");
				return (false);
			}
		}

		if (!Acquire())
			return (false);

		m_exmode = value;
	}

	return (true);
}

///////////////////////////////////////////
void CXMouse::SetMouseWheelRotation(int value)
{
	if (!m_dinput)
		m_Deltas[2] = (float)(value);

	m_wheelChecked = false;
}

void CXMouse::SetVScreenX(float fX)
{
	m_fVScreenX = fX;
}

void CXMouse::SetVScreenY(float fY)
{
	m_fVScreenY = fY;
}

///////////////////////////////////////////////
float CXMouse::GetVScreenX()
{
	return m_fVScreenX + 0.5f;
}

///////////////////////////////////////////////
float CXMouse::GetVScreenY()
{
	return m_fVScreenY + 0.5f;
}

//////////////////////////////////////////////////////////////////////////
void CXMouse::PostEvent(int key, SInputEvent::EType type, float value, unsigned int timestamp)
{
	// Post Input events.
	SInputEvent event;
	event.key = key;
	event.type = type;
	if (timestamp)
		event.timestamp = timestamp;
	else
		event.timestamp = GetTickCount();
	event.moidifiers = m_pInput->GetModifiers();
	event.keyname = m_pInput->GetKeyName(event.key, event.moidifiers);
	event.value = value;
	m_pInput->PostInputEvent(event);
}

#define GETLEN2D(v) (v[0]*v[0]+v[1]*v[1])

void CXMouse::SmoothDeltas(float accel, float decel)
{
	if (accel < 0.0001f)
	{
		//do nothing ,just like it was before.
		return;
	}
	else if (accel < 0.9999f)//mouse smooth, average the old and the actual delta by the delta ammount, less delta = more smooth speed.
	{
		float delta[2];

		delta[0] = m_Deltas[0] - m_OldDeltas[0];
		delta[1] = m_Deltas[1] - m_OldDeltas[1];

		float len = sqrt(GETLEN2D(delta));

		float amt = 1.0f - (crymin(10.0f, len) / 10.0f * crymin(accel, 0.9f));

		//m_pLog->Log("amt:%f\n",amt);

		m_Deltas[0] = m_OldDeltas[0] + delta[0] * amt;
		m_Deltas[1] = m_OldDeltas[1] + delta[1] * amt;
	}
	else if (accel < 1.0001f)//mouse smooth, just average the old and the actual delta.
	{
		m_Deltas[0] = (m_Deltas[0] + m_OldDeltas[0]) * 0.5f;
		m_Deltas[1] = (m_Deltas[1] + m_OldDeltas[1]) * 0.5f;
	}
	else//mouse smooth with acceleration
	{
		float dt = crymin(m_pTimer->GetFrameTime(), 0.1f);

		float delta[2];

		float amt = 0.0;

		//if the input want to stop use twice of the acceleration.
		if (GETLEN2D(m_Deltas) < 0.0001f)
			if (decel > 0.0001f)//there is a custom deceleration value? use it.
				amt = crymin(1, dt * decel);
			else//otherwise acceleration * 2 is the default.
				amt = crymin(1, dt * accel * 2.0f);
		else
			amt = crymin(1, dt * accel);

		delta[0] = m_Deltas[0] - m_OldDeltas[0];
		delta[1] = m_Deltas[1] - m_OldDeltas[1];

		m_Deltas[0] = m_OldDeltas[0] + delta[0] * amt;
		m_Deltas[1] = m_OldDeltas[1] + delta[1] * amt;
	}

	m_OldDeltas[0] = m_Deltas[0];
	m_OldDeltas[1] = m_Deltas[1];
}

void CXMouse::CapDeltas(float cap)
{
	/*float len = GETLEN2D(m_Deltas);

	if (len < cap)
		return;

	float mult;

	if (len > 0.0f)
		mult = 1.0f/len;
	else
		mult = 0.0001f;

	m_Deltas[0] *= mult * cap;
	m_Deltas[1] *= mult * cap;*/

	//just cap deltas at certain value, used for mouse acceleration
	float temp;

	temp = fabs(m_Deltas[0]) / cap;
	if (temp > 1.0f) m_Deltas[0] /= temp;

	temp = fabs(m_Deltas[1]) / cap;
	if (temp > 1.0f) m_Deltas[1] /= temp;
}
