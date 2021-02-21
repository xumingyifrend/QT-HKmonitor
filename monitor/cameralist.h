#ifndef CAMERALIST_H
#define CAMERALIST_H
#include <QListWidget>
#include <QDrag>
#include <QDragEnterEvent>
#include <QMimeData>
#include "ddcdoc.h"

class cameraList : public QListWidget
{
    Q_OBJECT
public:
    explicit cameraList(DdcDoc *ddcDoc, QWidget *parent = 0);
    explicit cameraList(QWidget *parent = 0);
    void addCamera(QPixmap pixmap, QString name , QString ip);
    static QString cameraMimeType() { return QStringLiteral("image/userId"); }

protected:
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dragMoveEvent(QDragMoveEvent *event) override;
    void startDrag(Qt::DropActions supportedActions) override;

    int m_PieceSize;

private:
    DdcDoc *m_ddcDoc;
};

#endif // CAMERALIST_H
