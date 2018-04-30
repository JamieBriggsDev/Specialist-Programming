// Errorlogger.h
// Chris Rook
// Version 10 Created 28/4/05
// Version 10.1 9/5/05
// Added maximum line limit for logs
// Version 10.2 12/5/05
// Corrected errors in Writeln(double)
// Version 10.3 20/3/07
// Use sprintf for WriteLn(double) instead of my hacky code.

#pragma once

#include <fstream>
#include <windows.h>

//using namespace std;

#define LOGGING			// Remove to prevent logging
#define SLOWLOG			// Remove to prevent log flush (will log faster, but may not write to file in the event of a program crash


class ErrorLogger
{
private:
	static ErrorLogger instance;
	ErrorLogger();
	~ErrorLogger();
	std::wofstream file;
	const static char Filename[];
	const static int MAXLINES=100;
	static int LineCount;

public:
	static void Writeln(const wchar_t text[]);
	static void Writeln(double num);
	static void Write(const wchar_t text[]);
	static void Write(double num);
	static void Writeln(HRESULT hr);

};
