#ifndef ANNOTATIONS_H
#define ANNOTATIONS_H

#include <iostream>
#include <fstream>
#include <vector>
#include <QDir>
#include <QString>
#include <MyQclass.h>
#include <opencv2/opencv.hpp>

class Annotations
{
public:
    Annotations(QDir dir);
    ~Annotations();
    QDir SaveDir;
    bool YOLO;
    bool FasterRCNN;
    bool SSD;
    void addObject(QString filename, int cls, float cx, float cy, float w, float h);
    void addObject(QString objectName, int x1, int y1, int x2, int y2);
    void setHeader(cv::Mat src, QString imgname, std::vector<QString> labels);
    void pushBack();
    void popBack();
    void flashAll();

private:
    void setFlags();
    struct FORMAT_FRCNN_OBJECT
    {
        QString label;
        QString center;
        QString box;
    };
    struct FORMAT_FRCNN_HEADER
    {
        QString fn_out;
        QString filename;
        QString size;
        QString database;
        QString groundtruth;
        std::vector<FORMAT_FRCNN_OBJECT> obj;
    };
    struct FORMAT_YOLO_OBJECT
    {
        int cls;
        float x;
        float y;
        float w;
        float h;
    };
    struct FORMAT_YOLO_HEADER
    {
        QString fn_out;
        std::vector<FORMAT_YOLO_OBJECT> obj;
    };
    FORMAT_FRCNN_HEADER *frcnn_hdr;
    FORMAT_FRCNN_OBJECT *frcnn_obj;
    FORMAT_YOLO_HEADER *yolo_hdr;
    FORMAT_YOLO_OBJECT *yolo_obj;
    std::vector<FORMAT_YOLO_HEADER> yolo_db;
    std::vector<FORMAT_FRCNN_HEADER> frcnn_db;
    MyQclass mq;
};

#endif // ANNOTATIONS_H
