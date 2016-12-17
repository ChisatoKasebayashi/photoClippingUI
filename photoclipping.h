#ifndef PHOTOCLIPPING_H
#define PHOTOCLIPPING_H

#include <QWidget>
#include <MyQclass.h>
#include <MyQView.h>
#include <QGraphicsObject>
#include <QGraphicsScene>
#include <QSettings>
#include <fstream>
#include <Annotations.h>

namespace Ui {
class photoclipping;
}

class photoclipping : public QWidget
{
    Q_OBJECT

public:
    explicit photoclipping(QWidget *parent = 0);
    ~photoclipping();
    int drawImage(QString filepath);
    void CorrectCoordinatesOfOutside(int &x, int &y);
    void photoSaveImage(cv::Mat src);
    void setFileList(QString dirpath);

private slots:
    void onPushSelectFolder();
    void onPushSaveto();
    void onMouseMovedGraphicsImage(int x, int y ,Qt::MouseButton button);
    void onMouseReleasedGraphicImage(int x, int y ,Qt::MouseButton button);
    void onMousePressdGraphicsImage(int x, int y, Qt::MouseButton button);
    void onPushNext();
    void onPushBack();
    void onPushClear();

private:
    Ui::photoclipping *ui;
    void connectSignals();
    void outputtxt();
    MyQclass myq;
    int count;
    int save_count;
    cv::Mat img_now;
    QFileInfoList imglist;
    QGraphicsScene scene;
    std::vector<QString> RecentImg;
    QSettings settings;
    std::vector<QString> points;
    void wheelEvent(QWheelEvent *pEvent);
    struct cursur_point
    {
        int x;
        int y;
        int button;
    }c_point;
    struct object_bbox
    {
        QString name;
        int cls;
        int x1;
        int y1;
        int x2;
        int y2;
    };
    std::vector<object_bbox> boxes;
    cursur_point cp_start;
    QDir working_directory;
    void updatescene();
    Annotations *annos;
};

#endif // PHOTOCLIPPING_H
