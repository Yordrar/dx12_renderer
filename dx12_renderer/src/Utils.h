#pragma once

#include <Windows.h>
#include <string>

std::string WideStrToStr( const std::wstring& wstr );
std::wstring StrToWideStr( const std::string& str );