#include "FrameData.hpp"
#include "Shared.hpp"

#include <fstream>
#include <string>

static LPCWSTR g_lpszFilename = L"Idleg.config";

void GetFrameData(_Inout_ FrameData* lpFrameData)
{
    std::wifstream ifFrameDataFile(g_lpszFilename);
    if (ifFrameDataFile.is_open())
    {
        std::wstring strline;
        std::getline(ifFrameDataFile, strline);
        
        std::wstring_view strview = strline.c_str(),
            strTemp;
        size_t ndelim = strview.find_first_of(L";");
        if (ndelim > 0)
        {
            strTemp = strview.substr(0, ndelim);
            wcsncpy(lpFrameData->lpszFilename_, strTemp.data(), strTemp.length());
        }
        
        strview = strview.substr(ndelim +1, strview.length() - ndelim-1);
        ndelim = strview.find_first_of(L";");
        if (ndelim > 0)
        {
            strTemp = strview.substr(0, ndelim);
            lpFrameData->nFrames = std::stoi(std::wstring(strTemp));
            lpFrameData->nFrames = max(lpFrameData->nFrames, NFRM_MIN);
            lpFrameData->nFrames = min(lpFrameData->nFrames, NFRM_MAX);
        }

        strview = strview.substr(ndelim +1, strview.length() - ndelim-1);
        ndelim = strview.find_first_of(L";");
        if (ndelim > 0)
        {
            strTemp = strview.substr(0, ndelim);
            lpFrameData->nSeconds = std::stoi(std::wstring(strTemp));
            lpFrameData->nSeconds = max(lpFrameData->nSeconds, TMO_SEC_MIN);
            lpFrameData->nSeconds = min(lpFrameData->nSeconds, TMO_SEC_MAX);
        }
    }
}

void SaveFrameData(_In_ const FrameData* lpFrameData)
{
    std::wofstream ofFrameDataFile(g_lpszFilename);
    if (ofFrameDataFile.is_open())
    {
        ofFrameDataFile << lpFrameData->lpszFilename_ << L";" << lpFrameData->nFrames << L";"
            << lpFrameData->nSeconds;
    }
}