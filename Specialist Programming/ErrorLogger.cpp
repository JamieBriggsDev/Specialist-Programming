#include "errorlogger.h"
#include <math.h>

const char ErrorLogger::Filename[]="error.log";
ErrorLogger ErrorLogger::instance;
int ErrorLogger::LineCount=0;


ErrorLogger::ErrorLogger()
{
#ifdef LOGGING
	file.open(Filename);
#endif
}

ErrorLogger::~ErrorLogger()
{
#ifdef LOGGING
	file.close();
#endif
}

void ErrorLogger::Writeln(const wchar_t text[])
{
	Write(text);
	Write(L"\n");
}

void ErrorLogger::Writeln(HRESULT hr) 
{
  //Short and sweet, and it works.
  LPWSTR output;
  if(FAILED(hr)) {
    FormatMessage( FORMAT_MESSAGE_FROM_SYSTEM |
    FORMAT_MESSAGE_ALLOCATE_BUFFER |
    FORMAT_MESSAGE_IGNORE_INSERTS, NULL, hr,
    MAKELANGID(LANG_NEUTRAL,SUBLANG_DEFAULT),
    (LPTSTR) &output, 0, NULL);
  } 
  //output this using something that supports wchar_t unicode
  Writeln(output);
}

void ErrorLogger::Write(const wchar_t text[])
{
#ifdef LOGGING
	if(LineCount<MAXLINES)
	{
		OutputDebugString(text);
		instance.file << text;
		if(++LineCount == MAXLINES)
		{
			OutputDebugString(TEXT("\nErrorLogger limit reached. Who taught you to progam?"));
			instance.file << "\nErrorLogger limit reached. Who taught you to progam?";
			instance.file.flush();
		}
	}
#endif
#ifdef SLOWLOG
	instance.file.flush();
#endif
}

void ErrorLogger::Writeln(double num)
{
	Write(num);
	Write(L"\n");
}

void ErrorLogger::Write(double num)
{
#ifdef LOGGING
	wchar_t buffer[32];
	swprintf_s( buffer,32, L"%.8g", num );
	Write(buffer);
#endif
}

