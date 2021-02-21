#ifndef GLOBALDEF_H
#define GLOBALDEF_H

#include <QString>
#include <QColor>
#include <QDebug>
#include <QCoreApplication>
#include <QSettings>
#include <QFile>
#include <QProcess>
#include <QDateTime>
//#include "rspsglobaldef.h"

#pragma execution_character_set("utf-8")

#define PUSHBUTTON_STYLE 			"QPushButton{border:2px groove gray;"\
					               			"color: rgb(255, 255, 255);border-radius:10px; "\
					               			"background-color: rgb(51, 122, 183);}"\
					               			"QPushButton:pressed{color:rgb(0, 0, 255); background-color: rgb(0, 255, 0);}"
#define PUSHBUTTON_DISABLE_STYLE	"QPushButton{border:2px groove gray;"\
                           					"color: rgb(255, 255, 255);border-radius:10px; "\
                           					"background-color: rgb(127, 127, 127);}"

#define ________________info________________
typedef struct STRUCT_RSPS_SYSTEM_PARA
{
	int raderAlarmDistance;
	QString raderCom;	
	QString ledAlarmCom;
	QString batteryCom;
	QString serverIP;
	QString hostIP;
	QString hostID;	
	QString ledIP;
	QString cameraIP;	
	QString rfIP;
}RSPS_SYSTEM_PARA;


#define ________________camera________________
typedef struct STRUCT_CAMERA_PLAYBACK_ITEM_INFO
{
    QDateTime start;        //回放开始时间
    QDateTime end;          //回放结束时间
    bool flag;              //标志是否有视频
}CAMERA_PLAYBACK_ITEM_INFO;

typedef struct STRUCT_CAMERA_ITEM_INFO
{
    QString ip;                 //摄像头IP
    QString adapterId;          //所属适配器ID
    QString adapterName;        //所属适配器名
    bool status;                //离在线状态（0：离线   1：在线）
    long userId;                //连接的唯一标识（默认-1）
    long IRealPlayHandle;       //播放的唯一标识（默认-1）
    bool change;                //状态是否变化标志位
    long lVoiceHanle;           //语音对讲通道号
}CAMERA_ITEM_INFO;
#endif // GLOBALDEF_H
