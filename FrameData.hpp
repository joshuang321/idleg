#include <Windows.h>

struct FrameData
{
    WCHAR lpszFilename_[2*MAX_PATH+1];
    int nSeconds;
    int nFrames;
};

void GetFrameData(_Inout_ FrameData* lpFrameData);
void SaveFrameData(_In_ const FrameData* lpFrameData);