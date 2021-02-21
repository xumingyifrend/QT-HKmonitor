#ifndef COMMONINTERFACE_H
#define COMMONINTERFACE_H

#include <QStringList>
#include <QTableWidget>

#define TABLE_ALTERNATING_ROW_COLOR    QColor(179,216,255)

class CommonInterface
{
public:
    CommonInterface();

    static void setTableHeader(QTableWidget *tableWidget, QStringList &header);
    static void setTableVHeader(QTableWidget *tableWidget, QStringList &header);
    static void setTableDefaultStyle(QTableWidget *tableWidget);
    static void resizeTableToContents(QTableWidget *tableWidget);
};

#endif // COMMONINTERFACE_H
