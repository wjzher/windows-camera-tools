/*
 *		Win-camera-tools: Generic camera tools for Windows.
 *		Copyright (C) 2015 Chew Esmero
 *
 *		This file is part of win-camera-tools.
 *
 *		Win-camera-tools is free software: you can redistribute it and/or modify
 *		it under the terms of the GNU General Public License as published by
 *		the Free Software Foundation, either version 3 of the License, or
 *		(at your option) any later version.
 *
 *		Win-camera-tools is distributed in the hope that it will be useful,
 *		but WITHOUT ANY WARRANTY; without even the implied warranty of
 *		MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *		GNU General Public License for more details.
 *
 *		You should have received a copy of the GNU General Public License
 *		along with win-camera-tools. If not, see <http://www.gnu.org/licenses/>.
 */

#include "stdafx.h"
#include <Windows.h>
#include "..\trace\trace.h"
#include <Windows.h>
#include <DShow.h>
#include <Ks.h>
#include <KsMedia.h>
#include <ksproxy.h>
#include <string>
using namespace std;
#include <strsafe.h>
#include <comdef.h>
#include <atlbase.h>
#include "..\include\libcore.h"
#include "..\include\libcamera.h"
#include "appcontext.h"
#include "flash.h"
#include "privacy.h"
#include "issyscam.h"
#include "fnames.h"
#include "common.h"

#if _DEBUG
#pragma comment(lib, "..\\..\\out\\x86\\Debug\\libcore.lib")
#else
#pragma comment(lib, "..\\..\\out\\x86\\Release\\libcore.lib")
#endif

static CContext ct;

int Help(wchar_t *pszParam, wchar_t *pszSubParam, PVOID pContext)
{
	CContext *pCt = (CContext*)pContext;

	_tprintf(L"Synopsis:\n\n");
	_tprintf(L"    camera-cmd.exe option <params...>\n\n");
	_tprintf(L"Options:\n\n");
	_tprintf(L"    fnames\n\n");
	_tprintf(L"        List the available/attached camera(s) in the system.\n\n");
	_tprintf(L"    privacy\n\n");
	_tprintf(L"        Control the camera privacy properties. On query, it will\n");
	_tprintf(L"        return the current state. On set, it will return 0 on\n");
	_tprintf(L"        success, 255 on failure.\n\n");
	_tprintf(L"        Supported parameters:\n\n");
	_tprintf(L"        -fname:<camera_friendly_name>\n\n");
	_tprintf(L"        camera_friendly_name\n\n");
	_tprintf(L"            Friendly name of the camera device/driver. You can use\n");
	_tprintf(L"            the 'fnames' parameter for the friendly name(s) or see\n");
	_tprintf(L"            Device Manager -> Imaging devices.\n\n");
	_tprintf(L"        -state:<0|1>\n\n");
	_tprintf(L"        0 - turn privacy mode off.\n");
	_tprintf(L"        1 - turn privacy mode on.\n\n");
	_tprintf(L"        Examples:\n\n");
	_tprintf(L"        camera-cmd.exe privacy -fname:Integrated Camera\n");
	_tprintf(L"        camera-cmd.exe privacy -fname:Integrated Camera -state:1\n\n");
	_tprintf(L"    flash\n\n");
	_tprintf(L"        Control the camera flash properties. On query, it will\n");
	_tprintf(L"        return the current state. On set, it will return 0 on\n");
	_tprintf(L"        success, 255 on failure.\n\n");
	_tprintf(L"        Supported parameters:\n\n");
	_tprintf(L"        -fname:<camera_friendly_name>\n\n");
	_tprintf(L"        camera_friendly_name\n\n");
	_tprintf(L"            Friendly name of the camera device/driver. You can use\n");
	_tprintf(L"            the 'fnames' parameter for the friendly name(s) or see\n");
	_tprintf(L"            Device Manager -> Imaging devices.\n\n");
	_tprintf(L"        -state:<0|1|2>\n\n");
	_tprintf(L"        0 - turn camera flash off.\n");
	_tprintf(L"        1 - turn camera flash on.\n");
	_tprintf(L"        2 - set camera flash to auto flash.\n\n");
	_tprintf(L"        Examples:\n\n");
	_tprintf(L"        camera-cmd.exe flash -fname:Integrated Camera\n");
	_tprintf(L"        camera-cmd.exe flash -fname:Integrated Camera -state:2\n\n");	
	_tprintf(L"    issyscam\n\n");
	_tprintf(L"        Return the index of the camera friendly name being queried.\n");
	_tprintf(L"        Returns 255 on failure, or when the camera is not found.\n\n");
	_tprintf(L"        Supported parameters:\n\n");
	_tprintf(L"        -fname:<camera_friendly_name>\n\n");
	_tprintf(L"        camera_friendly_name\n\n");
	_tprintf(L"            Friendly name of the camera device/driver. You can use\n");
	_tprintf(L"            the 'fnames' parameter for the friendly name(s) or see\n");
	_tprintf(L"            Device Manager -> Imaging devices.\n\n");
	_tprintf(L"        Example:\n\n");
	_tprintf(L"        camera-cmd.exe issyscam -fname:Integrated Camera\n\n");

	*pCt->m_pCmdSupported = TRUE;

	return NOERROR;
}

int Junk(wchar_t *pszParam, wchar_t *pszSubParam, PVOID pContext)
{
	CContext *pCt = (CContext*)pContext;

	ICameraMf *pCamMf = NULL;
	HRESULT hr = E_FAIL;

	hr = CreateCameraMfInstance(&pCamMf);

	if (SUCCEEDED(hr) && pCamMf)
	{
		wchar_t *pszNames = NULL;
		LONG cbSize = 0;

		hr = pCamMf->GetFriendlyNames(&pszNames, &cbSize);

		if (pszNames)
		{
			// _tprintf(L"Names (%d) = %s\n", cbSize, pszNames);

			std::vector<std::wstring> names;
			std::wstring wstrnames(pszNames);

			split(wstrnames, L';', names);

			for (int i = 0; i < names.size(); i++)
			{
				_tprintf(L"Name = %s\n", names.at(i).c_str());
			}

			free(pszNames);
		}

		pCamMf->Release();
	}

	*pCt->m_pCmdSupported = TRUE;

	return NOERROR;
}

static ARG_DISPATCH_TABLE pdt[] = {
	{ L"junk", Junk },
	{ L"help", Help },
	{ L"fnames", DispatchFriendlyNames },
	{ L"flash", DispatchFlash },
	{ L"privacy", DispatchPrivacy },
	{ L"issyscam", DispatchIsSystemCamera },
};

//
// Main entry.
//
int _tmain(int argc, _TCHAR* argv[])
{
	wchar_t szCtrl[MAX_PATH] = { 0 };
	wchar_t szParam[MAX_PATH] = { 0 };
	int retcode = 255;
	BOOL bSupportedCmd = FALSE;
	HRESULT hr = S_OK;

	ct.m_pCmdSupported = &bSupportedCmd;
	ct.m_argc = argc;
	ct.m_ppargv = argv;

	hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);

	if (argc >= 2)
	{
		StringCchPrintf(szCtrl, MAX_PATH, L"%s", argv[1]);

		for (int itr = 0; itr < ARRAYSIZE(pdt); itr++)
		{
			if (_wcsicmp(szCtrl, pdt[itr].szParam) == 0)
			{
				for (int i = 2; i < argc; i++)
				{
					StringCchCat(szParam, MAX_PATH, argv[i]);
					StringCchCat(szParam, MAX_PATH, L" ");
				}

				StringCchLength(szParam, MAX_PATH, &ct.m_cchlen);

				//
				// Call dispatch function.
				//
				retcode = pdt[itr].pfnDispatch(pdt[itr].szParam, szParam, &ct);

				break;
			}
		}
	}

	CoUninitialize();

	if (!bSupportedCmd)
	{
		_tprintf(L"\nCommand not supported.\n\n");
		Help(NULL, NULL, &ct);
	}

	return retcode;
}
