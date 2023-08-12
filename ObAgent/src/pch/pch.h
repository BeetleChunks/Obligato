#pragma once

//#define WIN32_LEAN_AND_MEAN // Exclude rarely-used stuff from Windows headers
#define UMDF_USING_NTSTATUS
#include <ntstatus.h>

#include <Windows.h>
#include <WtsApi32.h>
#include <AclAPI.h>
#include <intsafe.h>
#include <lm.h>
#include <NTSecAPI.h>

#include <iostream>
#include <string>
#include <format>
#include <unordered_map>
#include <any>
#include <algorithm>
#include <vector>