#pragma once

#pragma region //Windows Includes
#include <iostream>
#include <string>
#include <sstream>
#include <math.h>
#include <fstream>
#include <random>
#include <stdio.h>
#include <vector>
#include <list>
#include <queue>
#include <algorithm>
#include <functional>
#include <memory>
#include <map>

//mmgr specific includes
#include <stdlib.h>
#include <assert.h>
#include <time.h>
#include <stdarg.h>
#include <new>

#ifndef	_WIN32
#include <unistd.h>
#endif
#pragma endregion

#pragma region //Third-Pary Includes
#include <GL/gl3w.h>
#include <ImGui/imgui.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_syswm.h>

#include "EliteMath/EMath.h"
#include "EliteInput/EInputCodes.h"
#include "EliteInput/EInputData.h"
#include "framework/EliteData/EBlackboard.h"
#include "framework/EliteDecisionMaking/EDecisionMaking.h"
#include "framework/EliteHelpers/ESingleton.h"
#include "framework/EliteInput/EInputManager.h"

#pragma endregion

#define SAFE_DELETE(p) if (p) { delete (p); (p) = nullptr; }

#define INPUTMANAGER Elite::EInputManager::GetInstance()
#define DEBUGRENDERER2D EliteDebugRenderer2D::GetInstance()
