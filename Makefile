build: so-cpp.exe

so-cpp.exe: so-cpp.obj
	cl so-cpp.obj

so-cpp.obj: so-cpp.c
	cl /MD /c so-cpp.c

clean:
	del /Q /F *.obj so-cpp.exe