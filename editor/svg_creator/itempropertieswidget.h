#ifndef ITEMPROPERTIESWIDGET_H
#define ITEMPROPERTIESWIDGET_H

#include <QWidget>
#include "abstractlayoutitem.h"

class AbstractLayoutItem;
class QFormLayout;
class QScrollArea;
class QLabel;

class ItemPropertiesWidget : public QWidget
{
    Q_OBJECT
public:
    explicit ItemPropertiesWidget(QWidget *parent = nullptr);

    AbstractLayoutItem *item() const;
    void setItem(AbstractLayoutItem *newItem);

    void clear();
    void loadPropertyDescriptors();

private slots:
    void loadProperties();
    void storeProperties();

private:
    AbstractLayoutItem *m_item = nullptr;

    QScrollArea *scrollArea;
    QWidget *scrollViewPort;
    QFormLayout *m_layout;
    QLabel *typeLabel;

    struct PropRow
    {
        QWidget *w;
        AbstractLayoutItem::PropertyDescr descr;
    };
    QVector<PropRow> rows;
};

#endif // ITEMPROPERTIESWIDGET_H
