#pragma once

#include "../df/df.h"
#include <commctrl.h>

#ifdef _MSC_VER
#pragma comment(lib, "comctl32")
#pragma comment(lib, "Msimg32")
#pragma comment(lib, "gdiplus")

// Embed visual style XML manifest
#pragma comment(linker,                           \
    "\"/manifestdependency:type='Win32'"          \
    "   name='Microsoft.Windows.Common-Controls'" \
    "   version='6.0.0.0'"                        \
    "   processorArchitecture='*'"                \
    "   publicKeyToken='6595b64144ccf1df'"        \
    "   language='*'\""                           \
)

// Link common controls library
#pragma comment(lib, "ComCtl32.lib")

#endif // _MSC_VER


enum class AlignType : uint8_t {
    start,
    center,
    end,
};

#include "Font.h"

#include "Gdi.h"
#include "gdiP.h"
#include "Bitmap.h"

namespace sdf {
    class DrawBuffer {
    public:
        Bitmap buttonBmp_;
        char *buttonBmpBuf_ = nullptr;
        Gdiplus::Graphics *graph_ = nullptr;

        void newBmp(int32_t w, int32_t h);

        ~DrawBuffer() {
            if (graph_)
                delete graph_;
        }
    };


    class ControlStyle {
    public:
        uint32_t color = 0;
        uint32_t backColor = 0;
        int32_t shadowSize = 0;

        uint16_t borderLeft = 0;
        uint16_t borderTop = 0;
        uint16_t borderRight = 0;
        uint16_t borderBottom = 0;
        int16_t radius = 0;
        uint32_t borderColor = 0;
      
        std::shared_ptr<Bitmap> backImage;
        BitmapScaleType scaleType = BitmapScaleType::center;
        FontType font;

        void border(uint16_t size) {
            borderLeft = size;
            borderTop = size;
            borderRight = size;
            borderBottom = size;
        }
    };


    class ControlPos {
    public:
        int32_t x = 0;
        int32_t y = 0;


        //内容宽度,不包括右侧滚动条宽
        int32_t w = -1;
        int32_t h = -1;


        //实际高宽
        int32_t controlW = -1;
        int32_t controlH = -1;
        int32_t maxW = -1;
        int32_t maxH = -1;

        int32_t paddingLeft = 0;
        int32_t paddingTop = 0;
        int32_t paddingRight = 0;
        int32_t paddingBottom = 0;

        int32_t marginLeft = 0;
        int32_t marginTop = 0;
        int32_t marginRight = 0;
        int32_t marginBottom = 0;

        int16_t flexX = 0;
        int16_t flexY = 0;

        bool wrapX = false;
        bool wrapY = false;

        //是否垂直显示
        bool vector = false;

        //使用绝对坐标
        bool absolute = false;

        //是否已进行过dpi scale转换
        bool scaleMeasured = false;
        
        bool textMutiline = false;

        void flex(int32_t flex) {
            flexX = flex;
            flexY = flex;
        }

        void paddingX(int32_t val) {
            paddingLeft = val;
            paddingRight = val;
        }

        void paddingY(int32_t val) {
            paddingTop = val;
            paddingBottom = val;
        }

        void padding(int32_t val) {
            paddingLeft = val;
            paddingRight = val;
            paddingTop = val;
            paddingBottom = val;
        }


        void marginX(int32_t val) {
            marginLeft = val;
            marginRight = val;
        }

        void marginY(int32_t val) {
            marginTop = val;
            marginBottom = val;
        }

        void margin(int32_t val) {
            marginLeft = val;
            marginRight = val;
            marginTop = val;
            marginBottom = val;
        }


        //指定所有子成员居中
        void center() {
            alignX = AlignType::center;
            alignY = AlignType::center;
        }

        void alignEnd() {
            alignX = AlignType::end;
            alignY = AlignType::end;
        }

        void alignXEnd() {
            alignX = AlignType::end;
        }

        void alignYEnd() {
            alignY = AlignType::end;
        }

        void centerX() {
            alignX = AlignType::center;
        }

        void centerY() {
            alignY = AlignType::center;
        }

        //在父容器中居中
        void centerInParent() {
            alignInParentX = AlignType::center;
            alignInParentY = AlignType::center;
        }

        void centerInParentX() {
            alignInParentX = AlignType::center;
        }

        void centerInParentY() {
            alignInParentY = AlignType::center;
        }

        void alignInParentEnd() {
            alignInParentX = AlignType::end;
            alignInParentY = AlignType::end;
        }

        void alignInParentXEnd() {
            alignInParentX = AlignType::end;
        }

        void alignInParentYEnd() {
            alignInParentY = AlignType::end;
        }

        void textAlignStart() {
            AlignType textAlignX = AlignType::start;
            AlignType textAlignY = AlignType::start;
        }

      

        AlignType alignX = AlignType::start;
        AlignType alignY = AlignType::start;


        AlignType textAlignX = AlignType::center;
        AlignType textAlignY = AlignType::center;


        AlignType alignInParentX = AlignType::start;
        AlignType alignInParentY = AlignType::start;
    };


}


