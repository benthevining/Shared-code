
#pragma once


namespace ableton
{

/*
 This class implements the low-level interface with the Push2 device, preparing the raw pixel data to be transmitted via USB.
 */
class PushDisplayInterface
{
public:
    using pixel_t = PushBitmap::pixel_t;

    PushDisplayInterface()
    {
        auto* pData = data;
        for (uint8_t line = 0; line < kDataSourceHeight; line++)
        {
            memset (pData, 0, kDataSourceWidth * sizeof (pixel_t));
            pData += kDataSourceWidth;
        }
    }

    //bool init()
    //{
    //return usb.init();
    //}

    /* original comment from ableton: */
    // Transfers the bitmap into the output buffer sent to
    // the push display. The push display buffer has a larger stride
    // as the given bitmap
    void flip (const PushBitmap& b)
    {
        const auto* src = b.pixelData();
        auto*       dst = data;

        const auto graphicsWidth = b.getWidth();
        jassert (b.getHeight() == kDataSourceHeight);

        const auto chunkSize = (unsigned long) graphicsWidth * sizeof (pixel_t);

        for (uint8_t line = 0; line < kDataSourceHeight; line++)
        {
            memcpy (dst, src, chunkSize);
            src += graphicsWidth;
            dst += kDataSourceWidth;
        }
    }

private:
    static const int kDataSourceWidth  = 1024;
    static const int kDataSourceHeight = 160;

    pixel_t data[kDataSourceWidth * kDataSourceHeight];

    //PushUSB usb;
};

}  // namespace ableton