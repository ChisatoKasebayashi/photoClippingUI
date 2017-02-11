#include "Annotations.h"

Annotations::Annotations(QDir dir)
{
    SaveDir = dir;
    setFlags();
    if(YOLO)
    {
        mq.makeDirectory(dir.path(), "Annotation_YOLO");
    }
    if(FasterRCNN)
    {
        mq.makeDirectory(dir.path(), "Annotation_FasterRCNN");
    }
    if(SSD);
}

Annotations::~Annotations()
{
    flashAll();
}

void Annotations::setFlags()
{
    TXT = true;
    YOLO = true;
    FasterRCNN = true;
    SSD = false;
}

void Annotations::popBack()
{
    if(TXT)
    {
        if(txt_db.size())
            txt_db.pop_back();
    }
    if(YOLO)
    {
        if(yolo_db.size())
            yolo_db.pop_back();
    }
    if(FasterRCNN)
    {
        if(frcnn_db.size())
            frcnn_db.pop_back();
    }
    if(SSD);
}

void Annotations::pushBack()
{
    if(YOLO)
    {
        yolo_db.push_back(*yolo_hdr);
        delete(yolo_hdr);
    }
    if(FasterRCNN)
    {
        frcnn_db.push_back(*frcnn_hdr);
        delete(frcnn_hdr);
    }
    if(SSD);
}

void Annotations::setHeader(cv::Mat src, QString imgname, std::vector<QString> labels)
{
    QString fn = imgname;
    fn.chop(4);
    if(YOLO)
    {
        yolo_hdr = new FORMAT_YOLO_HEADER;
        yolo_hdr->fn_out = fn + ".txt";
    }
    if(FasterRCNN)
    {
        frcnn_hdr = new FORMAT_FRCNN_HEADER;
        frcnn_hdr->fn_out = fn + ".txt";
        frcnn_hdr->filename    = QString("Image filename : \"%1\"").arg(fn);
        frcnn_hdr->size        = QString("Image size (X x Y x C) : %1 x %2 x %3").arg(src.cols).arg(src.rows).arg(src.channels());
        frcnn_hdr->database    = QString("Database : \"RoboCup Soccer Object Detection by CITBrains\"");
        QString gt = QString("Objects with ground truth : %1 {").arg(labels.size());
        for(int i=0; i<labels.size();i++)
        {
            gt += QString("\"%1\", ").arg(labels[i]);
        }
        gt.chop(2);
        gt = gt + " }";
        frcnn_hdr->groundtruth = gt;
    }
}

void Annotations::addObject(int cls, float cx, float cy, float w, float h) // YOLO
{
    if(YOLO)
    {
        yolo_obj = new FORMAT_YOLO_OBJECT;
        yolo_obj->cls = cls;
        yolo_obj->x = cx;
        yolo_obj->y = cy;
        yolo_obj->w = w;
        yolo_obj->h = h;
        yolo_hdr->obj.push_back(*yolo_obj);
    }
    else
    {
        std::cout << "Error:status YOLO=false" << std::endl;
    }
}

void Annotations::addObject(QString objectName, int cls, int x1, int y1, int x2, int y2) // FasterRCNN, txt
{
    qDebug() << "addObject - START";
    if(TXT)
    {
        QString qst = QString("%1 %2 %3 %4 %5 %6").arg(objectName).arg(cls).arg(x1).arg(y1).arg(x2).arg(y2);
        txt_db.push_back(qst);
    }
    if(FasterRCNN)
    {
        frcnn_obj = new FORMAT_FRCNN_OBJECT;
        frcnn_obj->label      = QString("Original label for object %1 \"%2\" : \"2016%2\"").arg(cls+1).arg(objectName);
        frcnn_obj->center     = QString("Center point on object %1 \"%2\" (X, Y) : (%3, %4)").arg(cls+1).arg(objectName).arg((x1+x2)/2).arg((y1+y2)/2);
        frcnn_obj->box        = QString("Bounding box for object %1 \"%2\" (Xmin, Ymin) - (Xmax, Ymax) : (%3, %4) - (%5, %6)\n").arg(cls+1).arg(objectName).arg(x1).arg(y1).arg(x2).arg(y2);
        frcnn_hdr->obj.push_back(*frcnn_obj);
    }
    else
    {
        std::cout << "Error:status FasterRCNN=false" << std::endl;
    }
}

void Annotations::flashAll()
{
    if(TXT)
    {
        QString filename;
        filename = SaveDir.path() + "/Rects.txt";
        for(int i=0; i<txt_db.size(); i++)
        {
            std::ofstream txt_ofs(filename.toLocal8Bit(),std::ios_base::app);
            txt_ofs << txt_db[i].toStdString() << std::endl;
            txt_ofs.close();
        }
        txt_db.clear();
    }
    if(YOLO)
    {
        for(int i=0; i<yolo_db.size(); i++){
            std::ofstream ofs;
            ofs.open(QString(SaveDir.path() + "/Annotation_YOLO/" + yolo_db[i].fn_out).toLocal8Bit(), std::ios_base::app);
            for(int j=0; j<yolo_db[i].obj.size(); j++)
            {
                QString str = QString("%1 %2 %3 %4 %5")
                        .arg(yolo_db[i].obj[j].cls)
                        .arg(yolo_db[i].obj[j].x)
                        .arg(yolo_db[i].obj[j].y)
                        .arg(yolo_db[i].obj[j].w)
                        .arg(yolo_db[i].obj[j].h);
                ofs << str.toStdString() << std::endl;
            }
            ofs.close();
        }
        yolo_db.clear();
    }
    if(FasterRCNN)
    {
        for(int i=0; i<frcnn_db.size(); i++){
            std::ofstream ofs;
            ofs.open(QString(SaveDir.path() + "/Annotation_FasterRCNN/" + frcnn_db[i].fn_out).toLocal8Bit(), std::ios_base::app);
            ofs << frcnn_db[i].filename.toStdString() << std::endl;
            ofs << frcnn_db[i].size.toStdString() << std::endl;
            ofs << frcnn_db[i].database.toStdString() << std::endl;
            ofs << frcnn_db[i].groundtruth.toStdString() << std::endl;
            for(int j=0; j<frcnn_db[i].obj.size();j++){
                ofs << frcnn_db[i].obj[j].label.toStdString() << std::endl;
                ofs << frcnn_db[i].obj[j].center.toStdString() << std::endl;
                ofs << frcnn_db[i].obj[j].box.toStdString() << std::endl;
            }
            ofs.close();
        }
        frcnn_db.clear();
    }
    if(SSD);
}
