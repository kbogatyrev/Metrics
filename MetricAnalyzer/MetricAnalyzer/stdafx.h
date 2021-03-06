// stdafx.h: включаемый файл для стандартных системных включаемых файлов
// или включаемых файлов для конкретного проекта, которые часто используются, но
// не часто изменяются
//

#pragma once

#include "targetver.h"

#include <stdio.h>
#include <tchar.h>

#define WIN32_LEAN_AND_MEAN             // Исключите редко используемые компоненты из заголовков Windows
// Файлы заголовков Windows:
#include <windows.h>
#include <vector>
#include <string>
#include <map>
#include <set>
#include <algorithm>
#include <iostream>
#include <sstream>
#include <strstream>
#include <ctime>
#include <tchar.h>

//
// TR1
//
#include <regex>

using namespace std::tr1;

//
//
// CRT
#include <io.h>

//#include <atlbase.h>

//
// Zal
//
#include "Logging.h"

// TODO: Установите здесь ссылки на дополнительные заголовки, требующиеся для программы
