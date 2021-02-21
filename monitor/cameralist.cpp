#include "cameralist.h"
#include <QDebug>

#pragma execution_character_set("utf-8")
cameraList::cameraList(DdcDoc *ddcDoc, QWidget *parent):m_ddcDoc(ddcDoc)
{
    m_PieceSize = 30;
    setDragEnabled(true);
    setIconSize(QSize(m_PieceSize, m_PieceSize));
    setAcceptDrops(true);
    setDropIndicatorShown(true);

    setFixedWidth(200);
}

cameraList::cameraList(QWidget *parent)
{
    m_PieceSize = 30;
    setDragEnabled(true);
    setIconSize(QSize(m_PieceSize, m_PieceSize));
    setAcceptDrops(true);
    setDropIndicatorShown(true);

    setFixedWidth(200);    
}

void cameraList::dragEnterEvent(QDragEnterEvent *event)
{
    if (event->mimeData()->hasFormat(cameraList::cameraMimeType()))
        event->accept();
    else
        event->ignore();
}

void cameraList::dragMoveEvent(QDragMoveEvent *event)
{
    if (event->mimeData()->hasFormat(cameraList::cameraMimeType()))
    {
        event->setDropAction(Qt::MoveAction);
        event->accept();
    }
    else
    {
        event->ignore();
    }
}

void cameraList::addCamera(QPixmap pixmap, QString name , QString ip)
{
    QListWidgetItem *pieceItem = new QListWidgetItem;
    pieceItem->setText(name);
    pieceItem->setIcon(QIcon(pixmap));
    pieceItem->setData(Qt::UserRole, QVariant(pixmap));
    pieceItem->setData(Qt::UserRole+1, QString("-1"));
    pieceItem->setData(Qt::UserRole+2, ip);
    pieceItem->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsDragEnabled);

    addItem(pieceItem);
}

void cameraList::startDrag(Qt::DropActions /*supportedActions*/)
{
    QListWidgetItem *item = currentItem();

    QByteArray itemData;
    QDataStream dataStream(&itemData, QIODevice::WriteOnly);
    QPixmap pixmap = qvariant_cast<QPixmap>(item->data(Qt::UserRole));
    QString userId = item->data(Qt::UserRole+1).toString();
//    QString IRealPlayHandle = item->data(Qt::UserRole+2).toString();
//    dataStream << pixmap << location << IRealPlayHandle;
    dataStream << pixmap << userId;

    QMimeData *mimeData = new QMimeData;
    mimeData->setData(cameraList::cameraMimeType(), itemData);

    QDrag *drag = new QDrag(this);
    drag->setMimeData(mimeData);
    drag->setHotSpot(QPoint(pixmap.width()/2, pixmap.height()/2));
    drag->setPixmap(pixmap);

    if (drag->exec(Qt::MoveAction) == Qt::MoveAction)
    {
        //delete takeItem(row(item));
    }
}
