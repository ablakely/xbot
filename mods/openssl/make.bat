"%PROGRAMFILES%\Microsoft Visual Studio 2010\VC\vcvarsall.bat"

cl /I "..\..\lib" /I "..\..\include\openssl-3.2.1\include" /c openssl.c
link /DLL /out:..\openssl.dll openssl.obj ..\..\Debug\xbot.lib ..\..\include\openssl-3.2.1\lib\libssl.lib ..\..\include\openssl-3.2.1\lib\libcrypto.lib
del *.obj


