cl /I "..\..\lib" /I "..\..\include\libconfig-1.7.3\lib" /I "..\..\include\lua5.3\include" /c events.c 
cl /I "..\..\lib" /I "..\..\include\libconfig-1.7.3\lib" /I "..\..\include\lua5.3\include" /c handlers.c 
cl /I "..\..\lib" /I "..\..\include\libconfig-1.7.3\lib" /I "..\..\include\lua5.3\include" /c wrappers.c
cl /I "..\..\lib" /I "..\..\include\libconfig-1.7.3\lib" /I "..\..\include\lua5.3\include" /c lua.c 
link /DLL /out:..\lua.dll events.obj handlers.obj wrappers.obj lua.obj ..\..\Debug\xbot.lib ..\..\include\libconfig.lib ..\..\include\lua5.3\lua53.lib
