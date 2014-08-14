#include "sortfilterproxymodel.h"

SortFilterProxyModel::SortFilterProxyModel(bool acceptMods)
{
	setDynamicSortFilter(false);
	m_acceptSubmods = acceptMods;
}

bool SortFilterProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
	if(sourceParent.isValid())
	{
		bool isSubmod = sourceParent.child(sourceRow,0).data(Qt::UserRole+1).toString() == "submod";
		if(m_acceptSubmods)
			if(isSubmod)
				return !sourceParent.child(sourceRow,0).data(Qt::UserRole+2).toBool();
			else
				return false;
		else
			return !isSubmod;
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
