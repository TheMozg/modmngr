#ifndef SORTFILTERPROXYMODEL_H
#define SORTFILTERPROXYMODEL_H

#include <QtCore>

class SortFilterProxyModel : public QSortFilterProxyModel
{
	Q_OBJECT
public:
	SortFilterProxyModel(bool acceptMods);
protected:
	bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const;
	bool filterAcceptsColumn(int sourceColumn, const QModelIndex &sourceParent) const;
	bool m_acceptSubmods;

};

#endif // SORTFILTERPROXYMODEL_H
