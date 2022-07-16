// Copyright (c) Andrew Butson.
// Licensed under the MIT License.

#pragma once

// yes, this was developed on WINXP... left for posterity
#define WINVER			0x0500
#define _WIN32_WINNT	0x0501
#define _WIN32_IE		0x0501
#define _RICHEDIT_VER	0x0500

#include <atlbase.h>
#include <atlapp.h>
#include <atlwin.h>
#include <atlframe.h>
#include <sapi.h>
#include <atlctrls.h>
#include <atldlgs.h>
extern CAppModule app_module;

#include <string>
#include <locale>
#include <queue>
#include <thread>
#include <stdexcept>

#include "taskbar-icon.h"
#include "speech.h"
#include "resource.h"
#include "dialog.h"
