#ifndef DDCDOC_H
#define DDCDOC_H

#include <QObject>
#include <QList>
#include <QStringList>
#include <QPushButton>
#include <QMessageBox>
#include "globaldef.h"

#if (QT_VERSION >= QT_VERSION_CHECK(5,0,0))
#pragma execution_character_set("utf-8")
#endif

class DdcPushButton : public QPushButton
{
public:
    DdcPushButton(QWidget *parent = NULL);
};

class DdcDoc:public QObject
{
	Q_OBJECT
public:
    DdcDoc();
    ~DdcDoc();

    QVector<CAMERA_ITEM_INFO>* getCameraItemInfo();

protected:
    QVector<CAMERA_ITEM_INFO> m_cameraItemInfo;
};

#endif // DDCDOC_H
