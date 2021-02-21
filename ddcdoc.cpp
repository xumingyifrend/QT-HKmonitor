#include "ddcdoc.h"
#include <string.h>
#include <QtGlobal>
#include <QTime>

DdcPushButton::DdcPushButton(QWidget *parent)
{
    setStyleSheet(PUSHBUTTON_STYLE);
    setFixedSize(50, 25);
}

DdcDoc::DdcDoc()
{

}

DdcDoc::~DdcDoc()
{

}

QVector<CAMERA_ITEM_INFO>* DdcDoc::getCameraItemInfo()
{
    return &m_cameraItemInfo;
}
