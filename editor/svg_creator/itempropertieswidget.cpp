#include "itempropertieswidget.h"

#include <QFormLayout>
#include <QCheckBox>
#include <QSpinBox>
#include <QLineEdit>
#include <QComboBox>
#include <QLabel>

#include <QVBoxLayout>
#include <QScrollArea>
#include <QDialogButtonBox>

QWidget *propertyWidgetFactory(const AbstractLayoutItem::PropertyDescr& descr)
{
    QWidget *w = nullptr;

    switch (descr.type)
    {
    case QVariant::Type::Bool:
    {
        QCheckBox *box = new QCheckBox;
        w = box;
        break;
    }
    case QVariant::Type::Int:
    {
        QSpinBox *spin = new QSpinBox;
        spin->setRange(descr.min, descr.max);
        w = spin;
        break;
    }
    case QVariant::Type::Double:
    {
        QDoubleSpinBox *spin = new QDoubleSpinBox;
        spin->setRange(descr.min, descr.max);
        w = spin;
        break;
    }
    case QVariant::Type::String:
    {
        QLineEdit *edit = new QLineEdit;
        w = edit;
        break;
    }
    case QVariant::Type::UserType:
    {
        QComboBox *combo = new QComboBox;
        combo->addItems(descr.enumNames);
        w = combo;
        break;
    }
    default:
        return nullptr;
    }

    w->setToolTip(descr.description);
    return w;
}

QVariant getPropValue(QWidget *w, const AbstractLayoutItem::PropertyDescr& descr)
{
    switch (descr.type)
    {
    case QVariant::Type::Bool:
    {
        QCheckBox *box = static_cast<QCheckBox *>(w);
        return QVariant(box->isChecked());
    }
    case QVariant::Type::Int:
    {
        QSpinBox *spin = static_cast<QSpinBox *>(w);
        return QVariant(spin->value());
    }
    case QVariant::Type::Double:
    {
        QDoubleSpinBox *spin = static_cast<QDoubleSpinBox *>(w);
        return QVariant(spin->value());
    }
    case QVariant::Type::String:
    {
        QLineEdit *edit = static_cast<QLineEdit *>(w);
        return QVariant(edit->text());
    }
    case QVariant::Type::UserType:
    {
        QComboBox *combo = static_cast<QComboBox *>(w);
        return QVariant(combo->currentIndex());
    }
    default:
        break;
    }

    return QVariant();
}

bool setPropValue(QWidget *w, const AbstractLayoutItem::PropertyDescr& descr, const QVariant& val)
{
    switch (descr.type)
    {
    case QVariant::Type::Bool:
    {
        QCheckBox *box = static_cast<QCheckBox *>(w);
        box->setChecked(val.toBool());
        break;
    }
    case QVariant::Type::Int:
    {
        QSpinBox *spin = static_cast<QSpinBox *>(w);
        spin->setValue(val.toInt());
        break;
    }
    case QVariant::Type::Double:
    {
        QDoubleSpinBox *spin = static_cast<QDoubleSpinBox *>(w);
        spin->setValue(val.toDouble());
        break;
    }
    case QVariant::Type::String:
    {
        QLineEdit *edit = static_cast<QLineEdit *>(w);
        edit->setText(val.toString());
        break;
    }
    case QVariant::Type::UserType:
    {
        QComboBox *combo = static_cast<QComboBox *>(w);
        combo->setCurrentIndex(val.toInt());
        break;
    }
    default:
        return false;
    }

    return true;
}

ItemPropertiesWidget::ItemPropertiesWidget(QWidget *parent)
    : QWidget{parent}
{
    QVBoxLayout *lay = new QVBoxLayout(this);

    scrollArea = new QScrollArea;
    scrollArea->setWidgetResizable(false);

    scrollViewPort = new QWidget(scrollArea);
    m_layout = new QFormLayout(scrollViewPort);
    scrollArea->setWidget(scrollViewPort);
    lay->addWidget(scrollArea);

    typeLabel = new QLabel;
    m_layout->addRow(tr("Type"), typeLabel);

    QDialogButtonBox *box = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel,
                                                 Qt::Horizontal);
    lay->addWidget(box);

    connect(box, &QDialogButtonBox::rejected, this, &ItemPropertiesWidget::loadProperties);
    connect(box, &QDialogButtonBox::accepted, this, &ItemPropertiesWidget::storeProperties);
}

AbstractLayoutItem *ItemPropertiesWidget::item() const
{
    return m_item;
}

void ItemPropertiesWidget::setItem(AbstractLayoutItem *newItem)
{
    m_item = newItem;

    loadPropertyDescriptors();
    loadProperties();
}

void ItemPropertiesWidget::clear()
{
    for(const PropRow& r : qAsConst(rows))
    {
        m_layout->removeRow(r.w);
        delete r.w;
    }
    rows.clear();

    typeLabel->setText(tr("No Item"));
}

void ItemPropertiesWidget::loadPropertyDescriptors()
{
    clear();

    if(!m_item)
        return;

    typeLabel->setText(m_item->itemType());

    QVector<AbstractLayoutItem::PropertyDescr> propDescr = m_item->getPropertyDescriptors();

    rows.reserve(propDescr.size());

    for(const auto& descr : propDescr)
    {
        PropRow r;
        r.w = propertyWidgetFactory(descr);
        r.descr = descr;
        rows.append(r);

        m_layout->addRow(r.descr.name, r.w);
    }
}

void ItemPropertiesWidget::loadProperties()
{
    if(!m_item)
        return;

    QVariantMap map = m_item->getProperties();

    for(const PropRow& r : qAsConst(rows))
    {
        QVariant val = map.value(r.descr.name);
        setPropValue(r.w, r.descr, val);
    }
}

void ItemPropertiesWidget::storeProperties()
{
    if(!m_item)
        return;

    QVariantMap map;

    for(const PropRow& r : qAsConst(rows))
    {
        QVariant val = getPropValue(r.w, r.descr);
        map.insert(r.descr.name, val);
    }

    m_item->setProperties(map);
}
