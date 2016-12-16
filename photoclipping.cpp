#include "photoclipping.h"
#include "ui_photoclipping.h"

#define SAVEFLAG 0

photoclipping::photoclipping(QWidget *parent) :
    QWidget(parent),
    settings(QDir::homePath()+"/photoClipping.ini", QSettings::IniFormat),
    ui(new Ui::photoclipping)
{
    ui->setupUi(this);
    connectSignals();
    ui->graphicsImage->setScene(&scene);
    count = settings.value("SAVECOUNT",0).toInt();
    if(settings.value("IMAGEDIR").toString().size())
    {
        QString imagedir = settings.value("IMAGEDIR").toString();
        setFileList(imagedir);
    }
    if(settings.value("SAVEDIR",QDir::homePath() + "/Pictures").toString().size())
    {
        QString saveto = settings.value("SAVEDIR",QDir::homePath() + "/Pictures").toString();
        settings.remove("SAVEDIR");
        ui->comboSaveto->insertItem(0, saveto);
        ui->comboSaveto->setCurrentIndex(0);
        annos = new Annotations(QDir(saveto));
    }
    currentIndexChangedLabel();
}

photoclipping::~photoclipping()
{
    outputtxt();
    settings.setValue("SAVEDIR",ui->comboSaveto->currentText());
    settings.setValue("IMAGEDIR",ui->lineSelectFolder->text());
    settings.setValue("SAVECOUNT",count);
    delete ui;
}

void photoclipping::connectSignals()
{
    connect(ui->pushSelectFolder, SIGNAL(clicked()), this, SLOT(onPushSelectFolder()));
    connect(ui->graphicsImage, SIGNAL(mouseMoved(int,int ,Qt::MouseButton)), this, SLOT(onMouseMovedGraphicsImage(int,int ,Qt::MouseButton)));
    connect(ui->graphicsImage, SIGNAL(mouseReleased(int,int, Qt::MouseButton)), this, SLOT(onMouseReleasedGraphicImage(int,int, Qt::MouseButton)));
    connect(ui->graphicsImage, SIGNAL(mousePressed(int,int,Qt::MouseButton)), this, SLOT(onMousePressdGraphicsImage(int,int,Qt::MouseButton)));
    connect(ui->pushNext, SIGNAL(clicked()), this, SLOT(onPushNext()));
    connect(ui->pushBack, SIGNAL(clicked()), this, SLOT(onPushBack()));
    connect(ui->pushSaveto, SIGNAL(clicked()), this, SLOT(onPushSaveto()));
    connect(ui->comboLabel, SIGNAL(currentIndexChanged(int)),this, SLOT(currentIndexChangedLabel()));
}

void photoclipping::onPushSelectFolder()
{
    count = 0;
    QDir dir = myq.selectDir();
    if(dir.exists())
    {
        setFileList(dir.path());
    }
}

void photoclipping::onPushSaveto()
{
    QDir dir = myq.selectDir();
    ui->comboSaveto->insertItem(0,dir.path());
    ui->comboSaveto->setCurrentIndex(0);
    currentIndexChangedLabel();
    myq.makeDirectory(dir.path(), "Annotations");
    myq.makeDirectory(dir.path(), "YOLO_Annotations");
}

void photoclipping::setFileList(QString dirpath)
{
    ui->lineSelectFolder->setText(dirpath);
    working_directory.setCurrent(dirpath);
    imglist = myq.scanFiles(dirpath,"*.png");
    ui->labelImageNum->setText(QString("%1 / %2").arg(count).arg(imglist.size()));
    if(count < imglist.size())
    {
        drawImage(imglist[count].filePath());
    }
    RecentImg.resize(imglist.size());
    ui->labelImageNum->setText(QString("%1 / %2").arg(imglist.size()).arg(imglist.size()));
}

void photoclipping::onMouseMovedGraphicsImage(int x,int y ,Qt::MouseButton button)
{
    if(count < imglist.size())
    {
        CorrectCoordinatesOfOutside(x,y);
        c_point.x = x;
        c_point.y = y;
        ui->label_axis->setText(QString("x:%1 y:%2").arg(x).arg(y));
        updatescene();
    }
}

void photoclipping::onMousePressdGraphicsImage(int x, int y, Qt::MouseButton button)
{
    if(ui->comboMethod->currentText() == "Rect")
    {
        cp_start.x = x;
        cp_start.y = y;
        cp_start.button = 1;
    }
}

void photoclipping::onMouseReleasedGraphicImage(int x, int y ,Qt::MouseButton button)
{
    if(ui->comboMethod->currentText() == "Point")
    {
        if(count < imglist.size() && button == Qt::LeftButton)
        {
            CorrectCoordinatesOfOutside(x,y);
/*            scene.addRect(x - ui->spinSize->value()/2
                          ,y - ui->spinSize->value()/2
                          ,ui->spinSize->value(),ui->spinSize->value()
                          ,QPen(QColor(255,0,0))
                          );

            points.push_back(QString("%1 %2 %3").arg(imglist[count].fileName()).arg(x).arg(y));
            ui->labelPreview->setPixmap(myq.MatBGR2pixmap(img_now).copy(x - ui->spinSize->value()/2
                                                                        ,y - ui->spinSize->value()/2
                                                                        ,ui->spinSize->value(),ui->spinSize->value())
                                        );
            cv::Mat clp(img_now,cv::Rect(x - ui->spinSize->value()/2
                                         , y - ui->spinSize->value()/2
                                         , ui->spinSize->value()
                                         ,ui->spinSize->value())
                        );
            int cls = ui->comboLabel->currentIndex();
            photoSaveImage(clp);            
*/
            object_bbox bbox;
            bbox.cls = ui->comboLabel->currentIndex();
            bbox.name= ui->comboLabel->currentText();
            bbox.x1 = x - ui->spinSize->value()/2;
            bbox.y1 = y - ui->spinSize->value()/2;
            bbox.x2 = x + ui->spinSize->value()/2;
            bbox.y2 = y + ui->spinSize->value()/2;
            boxes.push_back(bbox);
        }
        else if(button == Qt::RightButton)
        {
            onPushNext();
        }
    }
    if(ui->comboMethod->currentText() == "Rect")
    {
        if(cp_start.button == 1)
        {
            object_bbox bbox;
            bbox.cls = ui->comboLabel->currentIndex();
            bbox.name= ui->comboLabel->currentText();
            bbox.x1 = x < cp_start.x ?x :cp_start.x;
            bbox.y1 = y < cp_start.y ?y :cp_start.y;
            bbox.x2 = x < cp_start.x ?cp_start.x :x;
            bbox.y2 = y < cp_start.y ?cp_start.y :y;
            boxes.push_back(bbox);
            cp_start.button = 0;
        }
    }
}

void photoclipping::updatescene()
{
    scene.clear();
    drawImage(imglist[count].filePath());
    if(ui->comboMethod->currentText() == "Point")
    {
        scene.addRect(c_point.x - ui->spinSize->value()/2
                      ,c_point.y - ui->spinSize->value()/2
                      ,ui->spinSize->value(),ui->spinSize->value()
                      ,QPen(QColor(255,0,0)));
        scene.addLine(c_point.x-5, c_point.y, c_point.x+5, c_point.y,QPen(QColor(255,0,0)));
        scene.addLine(c_point.x,c_point.y-5, c_point.x, c_point.y+5, QPen(QColor(255,0,0)));
        scene.addEllipse(c_point.x-ui->spinSize->value()/2
                         ,c_point.y-ui->spinSize->value()/2
                         ,ui->spinSize->value(),ui->spinSize->value()
                         ,QPen(QColor(100,150,250),2));
        if(ui->spinSize->value()==2)
        {
            scene.addLine(c_point.x-80, c_point.y, c_point.x+80, c_point.y,QPen(QColor(100,150,250)));
            scene.addLine(c_point.x,c_point.y-80, c_point.x, c_point.y+80, QPen(QColor(100,150,250)));
            scene.addLine(c_point.x-80, c_point.y-80, c_point.x+80, c_point.y+80,QPen(QColor(100,150,250)));
            scene.addLine(c_point.x-80,c_point.y+80, c_point.x+80, c_point.y-80, QPen(QColor(100,150,250)));
        }
        ui->labelPreview->setPixmap(myq.MatBGR2pixmap(img_now).copy(c_point.x - ui->spinSize->value()/2
                                                                    ,c_point.y - ui->spinSize->value()/2
                                                                    ,ui->spinSize->value(),ui->spinSize->value()));
    }
    if(ui->comboMethod->currentText() == "Rect")
    {
        ui->labelPreview->setText("Rect Mode");
        scene.addEllipse(c_point.x, c_point.y, 2, 2, QPen(QColor(Qt::yellow),3));
        if(cp_start.button == 1)
        {
            scene.addRect(QRect(QPoint(cp_start.x, cp_start.y), QPoint(c_point.x,c_point.y)),QPen(Qt::yellow,2));
        }
    }
    for(int i=0; i<boxes.size();i++)
    {
        int r = ((boxes[i].cls+1)%3)==1 ?255:0;
        int g = ((boxes[i].cls+2)%3)==1 ?255:0;
        int b = ((boxes[i].cls+3)%3)==1 ?255:0;
        if(boxes[i].cls > 2)
        {
            r = r/(boxes[i].cls+3/3);
            g = g/(boxes[i].cls+3/3);
            b = b/(boxes[i].cls+3/3);
        }
        scene.addRect(QRect(QPoint(boxes[i].x1, boxes[i].y1),QPoint(boxes[i].x2,boxes[i].y2)),QPen(QColor(r,g,b),2));
        //scene.addText(boxes[i].name,QFont("Arial",12,QFont::Bold));
    }
}

void photoclipping::onPushNext()
{
    count++;
    if(count < imglist.size())
    {
        RecentImg.clear();
        drawImage(imglist[count].filePath());
        ui->labelImageNum->setText(QString("%1 / %2")
                                   .arg(imglist.size()-count)
                                   .arg(imglist.size())
                                   );
    }
    else
    {
        scene.clear();
        ui->labelPreview->setText("exit");
        ui->labelImageNum->setText(QString("%1 / %2")
                                   .arg(imglist.size()-count)
                                   .arg(imglist.size())
                                   );
    }
    ui->pushBack->setEnabled(TRUE);
    boxes.clear();
}

void photoclipping::onPushBack()
{
    count--;
    if(count <= 0)
    {
        ui->pushBack->setDisabled(TRUE);
        count=0;
    }
    else if(count > imglist.size()+1)
    {
        ui->pushBack->setDisabled(TRUE);
    }
    else
    {
        QFile::remove(RecentImg[count]);
        annos->popBack();
        drawImage(imglist[count].filePath());
    }
    boxes.clear();
}

void photoclipping::currentIndexChangedLabel()
{
    QDir dir = myq.makeDirectory(ui->comboSaveto->currentText(),ui->comboLabel->currentText());
    save_count = myq.scanFiles(dir.path(), "*.png").size();
}

int photoclipping::drawImage(QString filepath)
{
    scene.clear();
    img_now = cv::imread(filepath.toStdString());
    if(img_now.empty())
        return 1;
    scene.addPixmap(myq.MatBGR2pixmap(img_now));
    ui->graphicsImage->setMinimumSize(img_now.cols+200, img_now.rows+200);
    ui->graphicsImage->setMaximumSize(img_now.cols+200, img_now.rows+200);
    ui->spinSize->setMaximum(img_now.cols<img_now.rows?img_now.cols:img_now.rows);
    ui->labelFilename->setText(imglist[count].fileName());
    return 0;
}


void photoclipping::CorrectCoordinatesOfOutside(int &x, int &y)
{
/*
    x = (x >= ui->spinSize->value()/2) ?x:ui->spinSize->value()/2;
    y = (y >= ui->spinSize->value()/2) ?y:ui->spinSize->value()/2;
    x = (x < img_now.cols - ui->spinSize->value()/2) ?x:img_now.cols - ui->spinSize->value()/2-1;
    y = (y < img_now.rows - ui->spinSize->value()/2) ?y:img_now.rows - ui->spinSize->value()/2-1;
*/
    x = (x >= 0) ?x:0;
    y = (y >= 0) ?y:0;
    x = (x < img_now.cols) ?x:img_now.cols-1;
    y = (y < img_now.rows) ?y:img_now.rows-1;
}

void photoclipping::photoSaveImage(cv::Mat src)
{
    QDir dir = myq.makeDirectory(ui->comboSaveto->currentText(),ui->comboLabel->currentText());
    RecentImg[count] = dir.path() + "/" + myq.filenameGen(ui->comboLabel->currentText(),save_count);
    cv::imwrite(RecentImg[count].toStdString(),src);
    qDebug() << "Saved :" << RecentImg[count];
    count++;
    save_count++;
    ui->pushBack->setEnabled(TRUE);
    if(count < imglist.size())
    {
        drawImage(imglist[count].filePath());
        ui->labelImageNum->setText(QString("%1 / %2").arg(imglist.size()-count).arg(imglist.size()));
    }
    else
    {
        scene.clear();
        ui->labelPreview->setText("exit");
        ui->labelImageNum->setText(QString("%1 / %2").arg(imglist.size()-count).arg(imglist.size()));
    }
}

void photoclipping::outputtxt()
{
    annos->flashAll();
}

void photoclipping::wheelEvent(QWheelEvent *pEvent)
{
    double dSteps = (double)pEvent->delta() / 120.0 * 4;
    ui->spinSize->setValue(ui->spinSize->value()+(int)dSteps);
    updatescene();
}
