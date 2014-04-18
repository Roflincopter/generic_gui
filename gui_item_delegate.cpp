#include "meta_types.hpp"

#include "gui_item_delegate.hpp"

GuiItemDelegate::GuiItemDelegate(QObject *parent) :
QStyledItemDelegate(parent)
{
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
