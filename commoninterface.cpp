#include "commoninterface.h"
#include "globaldef.h"

CommonInterface::CommonInterface()
{

}

//设置表格的列表头
void CommonInterface::setTableHeader(QTableWidget *tableWidget,
                                    QStringList &header)
{
    int count = header.size();
    tableWidget->setColumnCount(count);    
	tableWidget->setHorizontalHeaderLabels(header);
    return;
}

//设置表格的行表头
void CommonInterface::setTableVHeader(QTableWidget *tableWidget, QStringList &header)
{
    tableWidget->setVerticalHeaderLabels(header);
    return;
}

//设置表格的默认风格
void CommonInterface::setTableDefaultStyle(QTableWidget *tableWidget)
{
    QPalette pal;
    pal.setColor(QPalette::AlternateBase,TABLE_ALTERNATING_ROW_COLOR);
    tableWidget->setPalette(pal);
    
    tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
    tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
    tableWidget->setAlternatingRowColors(true);

    return;
}

//自动根据每列每行的宽高调整表格
void CommonInterface::resizeTableToContents(QTableWidget *tableWidget)
{
	tableWidget->resizeColumnsToContents();
	tableWidget->resizeRowsToContents();
}                
