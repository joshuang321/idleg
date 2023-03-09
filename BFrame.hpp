#include "Shared.hpp"

#include <objidl.h>
#include <gdiplus.h>
#include <string>
#include <cassert>

class BFrame
{
    std::wstring wstrFilename_;
    Gdiplus::Image* pImage_;

public:
    BFrame() : pImage_(NULL) { }

    BFrame(std::wstring&& wstrFilename) : wstrFilename_(wstrFilename), pImage_(NULL)
    {
        if (this->wstrFilename_.length())
        {
            this->pImage_ = new Gdiplus::Image(this->wstrFilename_.c_str()); 
            assert(this->pImage_);
        }
    }

    ~BFrame()
    {
        if (this->pImage_)
            delete this->pImage_;
    }

    void ChangeFilename(HWND hWndOwner)
    {
        this->wstrFilename_.resize(MAX_PATH);
        OPENFILENAMEW opfnW = { .lStructSize = sizeof(OPENFILENAMEW), .hwndOwner = hWndOwner,
            .lpstrFilter = L".BMP;.JPEG;.PNG;.GIF;.PNG;.TIFF;.WMF;.EMF", .lpstrFile = this->wstrFilename_.data(),
            .nMaxFile = MAX_PATH, .Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR };
        if (GetOpenFileNameW(&opfnW))
        {
            this->wstrFilename_.resize(wcslen(opfnW.lpstrFile));
            if (this->pImage_)
                delete this->pImage_;
            this->pImage_ = new Gdiplus::Image(this->wstrFilename_.c_str());
        }
    }

    void PaintWindow(LPDRAWITEMSTRUCT lpDis)
    {
        RECT rcFrameCtl;
        GetClientRect(lpDis->hwndItem, &rcFrameCtl);
        
        Gdiplus::Rect rcPaintArea(0, 0, rcFrameCtl.right, rcFrameCtl.bottom);
        Gdiplus::Graphics graphics(lpDis->hDC);
        Gdiplus::HatchBrush hatchBrush(Gdiplus::HatchStyle::HatchStyleLargeCheckerBoard,
            Gdiplus::Color::White,
            Gdiplus::Color::LightGray);
        assert(graphics.FillRectangle(&hatchBrush, rcPaintArea) == Gdiplus::Ok);

        if (pImage_)
        {
            int x = (rcFrameCtl.right - this->pImage_->GetWidth())/2,
                y = (rcFrameCtl.bottom - this->pImage_->GetHeight())/2;
            Gdiplus::Rect rcImageArea(max(x, 0), max(y, 0), min(this->pImage_->GetWidth(), BFRM_DEFAULT),
                min(this->pImage_->GetHeight(), BFRM_DEFAULT));
            assert(graphics.DrawImage(this->pImage_, rcImageArea) == Gdiplus::Ok);
        }
    }
    
    void getFilename(_Inout_z_ WCHAR lpszFilename[2*MAX_PATH+1]) { wcscpy(lpszFilename, this->wstrFilename_.c_str()); }
};