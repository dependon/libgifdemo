#include "gifdemo.h"
#include "ui_gifdemo.h"

gifDemo::gifDemo(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::gifDemo)
{

    ui->setupUi(this);
    connect(this,&gifDemo::sigGifImageRe,this,[=]{
        if(m_image.width()>1050)
        {
           ui->label->setPixmap(QPixmap::fromImage(m_image.scaled(1910,950)));
        }
        else
        {
            ui->label->setPixmap(QPixmap::fromImage(m_image));
        }
    });
    this->move(0,0);
}

gifDemo::~gifDemo()
{
    m_bRetThread=false;
    delete ui;
}

int32_t gifDemo::GifLoadFile()
{
    int32_t error = 0;
    int32_t size = 0;
    int32_t idx = 0;
    int32_t ret = 0;

    do {
        if (!m_bRetThread) {
            return -1;
        }
        if (nullptr == m_gEffectGifFile) {
            ret = -1;
            break;
        }

        gpGifFile = DGifOpenFileName(m_gEffectGifFile.toStdString().c_str(), &error);
        if (nullptr == gpGifFile) {
            ret = -2;
            break;
        }

        if ((gpGifFile->SHeight == 0) || (gpGifFile->SWidth == 0)) {
            ret = -3;
            break;
        }

        gpScreenBuffer = static_cast<GifRowType *>(malloc(static_cast<size_t>(gpGifFile->SHeight * static_cast<int32_t>((sizeof(GifRowType))))));
        if (nullptr == gpScreenBuffer) {
            ret = -4;
            break;
        }

        /* Size in bytes one row */
        size = gpGifFile->SWidth * static_cast<int32_t>(sizeof(GifPixelType));
        gpScreenBuffer[0] = static_cast<GifRowType>(malloc(static_cast<size_t>(size)));
        if (nullptr == gpScreenBuffer[0]) {
            ret = -5;
            break;
        }

        /* Set its color to BackGround */
        for (idx = 0; idx < gpGifFile->SWidth; idx++) {
            gpScreenBuffer[0][idx] = static_cast<unsigned char>(gpGifFile->SBackGroundColor);
        }

        /* Allocate the other rows, and set their color to background too */
        for (idx = 1; idx < gpGifFile->SHeight; idx++) {
            gpScreenBuffer[idx] = static_cast<GifRowType>(malloc(static_cast<size_t>(size)));
            if (nullptr == gpScreenBuffer[idx]) {
                ret = -6;
                break;
            }
            memcpy(gpScreenBuffer[idx], gpScreenBuffer[0], static_cast<size_t>(size));
        }

        if (0 > ret) {
            break;
        }
    } while (0);

    if (0 > ret) {
        GifFreeFile();
    }

    return ret;
}

void gifDemo::GifFreeFile()
{
    int32_t idx = 0;
    int32_t error = 0;
    if (!m_bRetThread || nullptr == gpGifFile) {
        m_bRetThread = false;
        return;
    }
    for (idx = 0; idx < gpGifFile->SHeight; idx++) {
        if (nullptr != gpScreenBuffer[idx]) {
            free(gpScreenBuffer[idx]);
            gpScreenBuffer[idx] = nullptr;
        }
    }

    if (nullptr != gpScreenBuffer) {
        free(gpScreenBuffer);
        gpScreenBuffer = nullptr;
    }

    if (nullptr != gpGifFile) {
        DGifCloseFile(gpGifFile, &error);
        gpGifFile = nullptr;
    }
}

int32_t gifDemo::GifFrameShow()
{
    ColorMapObject *colorMap = nullptr;
    GifByteType *extension = nullptr;

    int32_t InterlacedOffset[] = {0, 4, 2, 1}; // The way Interlaced image should
    int32_t InterlacedJumps[] = {8, 8, 4, 2}; // be read - offsets and jumps...
    uint8_t rgbBuf[240 * 320] = {0};
    int32_t extCode = 0;
    int32_t row = 0;
    int32_t col = 0;
    int32_t width = 0;
    int32_t height = 0;
    int32_t iW = 0;
    int32_t iH = 0;
    int32_t ret = 0;
    static int temp = 0;
    do {
        if (!m_bRetThread) {
            return -1;
        }
        if (DGifGetRecordType(gpGifFile, &gRecordType) == GIF_ERROR) {
            ret = -1;
            break;
        }

        switch (gRecordType) {
        case IMAGE_DESC_RECORD_TYPE: {
            if (DGifGetImageDesc(gpGifFile) == GIF_ERROR) {
                ret = -2;
                break;
            }

            row = gpGifFile->Image.Top;
            col = gpGifFile->Image.Left;
            width = gpGifFile->Image.Width;
            height = gpGifFile->Image.Height;

            if (gpGifFile->Image.Interlace) {
                for (iH = 0; iH < 4; iH++) {
                    for (iW = row + InterlacedOffset[iH]; iW < row + height; iW += InterlacedJumps[iH]) {
                        DGifGetLine(gpGifFile, &gpScreenBuffer[iW][col], width);
                    }
                }
            } else {
                for (iH = 0; iH < height; iH++) {
                    DGifGetLine(gpGifFile, &gpScreenBuffer[row++][col], width);
                }
            }

            colorMap = (gpGifFile->Image.ColorMap ? gpGifFile->Image.ColorMap : gpGifFile->SColorMap);
            if (colorMap == nullptr) {
                ret = -3;
                break;
            }
            GifScreenBufferToRgb888(colorMap, rgbBuf, gpScreenBuffer,
                                    gpGifFile->SWidth, gpGifFile->SHeight, m_tras);
            break;
        }
        case EXTENSION_RECORD_TYPE:

            /* Skip any extension blocks in file: */
            if (DGifGetExtension(gpGifFile, &extCode, &extension) == GIF_ERROR) {
                ret = -4;
                break;
            }
            if (extension != nullptr) {
                if (extension[0] & 0x01)
                    m_tras = NO_TRANSPARENT_COLOR;
                else
                    m_tras = static_cast<int>(extension[4]);
            }

            while (extension != nullptr) {
                temp++;
                if (DGifGetExtensionNext(gpGifFile, &extension) == GIF_ERROR) {
                    ret = -5;
                    break;
                }
                if (extension != nullptr) {
                    if (extension[0] & 0x01)
                        m_tras = NO_TRANSPARENT_COLOR;
                    else
                        m_tras = static_cast<int>(extension[4]);
                }
            }

            break;

        case TERMINATE_RECORD_TYPE:
            break;

        default:
            break;
        }

        if (0 < ret) {
            break;
        }
    } while (gRecordType != TERMINATE_RECORD_TYPE);

    return ret;
}

void gifDemo::GifScreenBufferToRgb888(ColorMapObject *ColorMap, uint8_t *inRgb, GifRowType *ScreenBuffer, int32_t ScreenWidth, int32_t ScreenHeight, int alphaIndex)
{
    Q_UNUSED(inRgb);
    if (m_fisrtImage) {
        m_image = QImage(ScreenWidth, ScreenHeight, QImage::Format_RGB32);
        m_fisrtImage = false;
    }
    GifColorType *ColorMapEntry = nullptr;
    // GifRowType GifRow = nullptr;
    QByteArray byte;
    int32_t idxH = 0;
    int32_t idxW = 0;
    int startTime = static_cast<int>(QDateTime::currentMSecsSinceEpoch());
    // QImage img(ScreenWidth, ScreenHeight, QImage::Format_ARGB32);
    for (idxH = 0; idxH < ScreenHeight; idxH++) {

        for (idxW = 0; idxW < ScreenWidth; idxW++) {
            ColorMapEntry = &ColorMap->Colors[ScreenBuffer[idxH][idxW]];
            //如果是透明色
            if (alphaIndex == ScreenBuffer[idxH][idxW] || m_image.pixel(idxW, idxH) == qRgb(ColorMapEntry->Red, ColorMapEntry->Green, ColorMapEntry->Blue)) {
                //img.setPixel(idxW, idxH, qRgba(ColorMapEntry->Red, ColorMapEntry->Green, ColorMapEntry->Blue, 0));
            } else {
                // img.setPixel(idxW, idxH, qRgba(ColorMapEntry->Red, ColorMapEntry->Green, ColorMapEntry->Blue, 255));
                m_image.setPixel(idxW, idxH, qRgba(ColorMapEntry->Red, ColorMapEntry->Green, ColorMapEntry->Blue, 255));
            }
        }
    }

    if (!m_bRetThread) {
        return;
    }

    int endTime = static_cast<int>(QDateTime::currentMSecsSinceEpoch());
    int tolTime = endTime - startTime;

    if (tolTime < 100) {
        QThread::msleep(static_cast<unsigned long>((100 - tolTime)));
    }

    emit sigGifImageRe();
}
#include <QFileDialog>
void gifDemo::on_pushButton_clicked()
{
    m_gEffectGifFile=QFileDialog::getOpenFileName();
    m_bRetThread=false;
    QThread::msleep(100);
    m_bRetThread=true;
    m_th = QThread::create([=]() {
        while (m_bRetThread) {
            GifLoadFile();
            GifFrameShow();
            GifFreeFile();
        }

    });

    m_th->start();
}
