#include "gui_item_delegate.hpp"

#include "meta_types.hpp"

#include <QMetaProperty>

GuiItemDelegate::GuiItemDelegate(QObject *parent) :
QStyledItemDelegate(parent)
{
}

void GuiItemDelegate::setModelData(QWidget* widget, QAbstractItemModel* model, const QModelIndex& index) const
{
	static int string_id = qMetaTypeId<std::string>();
	QByteArray n = widget->metaObject()->userProperty().name();
	QVariant variant = widget->property(n);
	
	if(variant.userType() == QVariant::String && variant.userType() != QMetaType::Float) {
		model->setData(index, QVariant::fromValue(variant.toString().toStdString()), Qt::EditRole);
	} else {
		QStyledItemDelegate::setModelData(widget, model, index);
	}
}

void GuiItemDelegate::setEditorData(QWidget* widget, const QModelIndex& index) const
{
	static int string_id = qMetaTypeId<std::string>();
	QVariant variant = index.data(Qt::EditRole);
	QByteArray n = widget->metaObject()->userProperty().name();
	
	if(string_id == variant.userType()) {
		widget->setProperty(n, QVariant(QString::fromStdString(variant.value<std::string>())));
	} else {
		QStyledItemDelegate::setEditorData(widget, index);
	}
}

QString GuiItemDelegate::displayText(const QVariant &value, const QLocale &locale) const
{
	QString ret;
	static int string_id = qMetaTypeId<std::string>();
	
	if(string_id == value.userType()) {
		ret = QString::fromStdString(value.value<std::string>());
	} else {
		ret = QStyledItemDelegate::displayText(value, locale);
	}
	return ret;
}
