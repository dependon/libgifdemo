#ifndef GITDEMO_H
#define GITDEMO_H

#include <QWidget>
#include <QImage>
#include <QDateTime>
#include <QThread>
#include "gif_lib.h"
namespace Ui {
class gifDemo;
}

class gifDemo : public QWidget
{
    Q_OBJECT

public:
    explicit gifDemo(QWidget *parent = nullptr);
    ~gifDemo();
    /**
     * @brief GifLoadFile
     * 加载gif文件，分配资源
     * @return int32_t
     */
    int32_t GifLoadFile(void);
    /**
     * @brief GifLoadFile
     * 释放gif文件，释放资源
     * @return
     */
    void GifFreeFile(void);
    /**
     * @brief GifFrameShow
     * 读取gif图片
     * @return
     */
    int32_t GifFrameShow(void);
    /**
     * @brief GifScreenBufferToRgb888
     * 读取buffer转化为rgb，逐个对点绘制
     * @return
     */
    void GifScreenBufferToRgb888(ColorMapObject *ColorMap, uint8_t *inRgb,
                                 GifRowType *ScreenBuffer, int32_t ScreenWidth, int32_t ScreenHeight,
                                 int alphaIndex = 0);

signals:
    void sigGifImageRe();
private slots:
    void on_pushButton_clicked();


private:
    Ui::gifDemo *ui;

    GifRecordType gRecordType = UNDEFINED_RECORD_TYPE;
    GifRowType *gpScreenBuffer = nullptr;
    GifFileType *gpGifFile = nullptr;
    QString m_gEffectGifFile;
    bool m_bRetThread = true;
    bool m_fisrtImage = true;
    QThread *m_th = nullptr;
    QTimer *m_pTimer = nullptr;
    QImage m_image;
    int m_tras=0;//透明色
};

#endif // GITDEMO_H
