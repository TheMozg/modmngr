#include "sortfilterproxymodel.h"

SortFilterProxyModel::SortFilterProxyModel(bool acceptMods)
{
	setDynamicSortFilter(false);
	m_acceptSubmods = acceptMods;
}

bool SortFilterProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
	if(sourceParent.child(sourceRow,0).data(Qt::UserRole+1).toString() == "mi")
		return true;
	if(sourceParent.isValid())
	{
		if(m_acceptSubmods)
			return sourceParent.child(sourceRow,0).flags().testFlag(Qt::ItemIsUserCheckable) &&
					!sourceParent.child(sourceRow,0).data(Qt::UserRole+2).toBool();
		else
			return !sourceParent.child(sourceRow,0).flags().testFlag(Qt::ItemIsUserCheckable);
	}
	else
	{
		return true;
	}
}

bool SortFilterProxyModel::filterAcceptsColumn(int sourceColumn, const QModelIndex &sourceParent) const
{
	if(m_acceptSubmods)
	{
		if(sourceColumn == 0 || sourceColumn == 4 || sourceColumn == 5)
			return true;
		else
			return false;
	}
	else
	{
		if(sourceColumn == 0 || sourceColumn == 1 || sourceColumn == 2 || sourceColumn == 3)
			return true;
		else
			return false;
	}
}
