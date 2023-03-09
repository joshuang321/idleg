debug-GuiWindow:
	rc resource.rc
	cl /std:c++20 /W4 /EHsc /c FrameData.cpp
	cl /std:c++20 /W4 /Fe:"idleg-gui" /W4 GuiWindow.cpp /link user32.lib gdi32.lib Comctl32.lib Gdiplus.lib Comdlg32.lib FrameData.obj resource.res
	del resource.res
	del *.obj

debug-MsgWindow:
	cl /std:c++20 /W4 /EHsc /c FrameData.cpp
	cl /std:c++20 /DWIN32_LEAN_AND_MEAN /W4 /LD /Fe:"idleg-dll.dll" idle-dll.cpp /link user32.lib kernel32.lib
	cl /std:c++20 /DWIN32_LEAN_AND_MEAN /W4 /EHsc /c MsgFrameWindow.cpp
	cl /std:c++20 /DWIN32_LEAN_AND_MEAN /W4 /Fe:"msg-idleg-serv" MsgWindow.cpp /link user32.lib gdi32.lib Gdiplus.lib MsgFrameWindow.obj FrameData.obj
	del *.obj
	del idleg-dll.exp
	del idleg-dll.lib

release-GuiWindow:
	rc resource.rc
	cl /std:c++20 /DNDEBUG /W4 /EHsc /c FrameData.cpp
	cl /std:c++20 /DNDEBUG /Fe:"idleg-gui" /W4 GuiWindow.cpp /link user32.lib gdi32.lib Comctl32.lib Gdiplus.lib Comdlg32.lib FrameData.obj resource.res
	del resource.res
	del *.obj

release-MsgWindow:
	cl /std:c++20 /DNDEBUG /W4 /EHsc /c FrameData.cpp
	cl /std:c++20 /DNDEBUG /DWIN32_LEAN_AND_MEAN /W4 /LD /Fe:"idleg-dll.dll" idle-dll.cpp /link user32.lib kernel32.lib
	cl /std:c++20 /DNDEBUG /DWIN32_LEAN_AND_MEAN /W4 /EHsc /c MsgFrameWindow.cpp
	cl /std:c++20 /DNDEBUG /DWIN32_LEAN_AND_MEAN /W4 /Fe:"msg-idleg-serv" MsgWindow.cpp /link user32.lib gdi32.lib Gdiplus.lib MsgFrameWindow.obj FrameData.obj
	del *.obj
	del idleg-dll.exp
	del idleg-dll.lib

clean:
	del *.exe
	del *.obj
	del resource.res
	del *.exp
	del *.lib
	del *.config
	del *.dll