/*
 * Traceshark - a visualizer for visualizing ftrace and perf traces
 * Copyright (C) 2016-2018  Viktor Rosendahl <viktor.rosendahl@gmail.com>
 *
 * This file is dual licensed: you can use it either under the terms of
 * the GPL, or the BSD license, at your option.
 *
 *  a) This program is free software; you can redistribute it and/or
 *     modify it under the terms of the GNU General Public License as
 *     published by the Free Software Foundation; either version 2 of the
 *     License, or (at your option) any later version.
 *
 *     This program is distributed in the hope that it will be useful,
 *     but WITHOUT ANY WARRANTY; without even the implied warranty of
 *     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *     GNU General Public License for more details.
 *
 *     You should have received a copy of the GNU General Public
 *     License along with this library; if not, write to the Free
 *     Software Foundation, Inc., 51 Franklin St, Fifth Floor, Boston,
 *     MA 02110-1301 USA
 *
 * Alternatively,
 *
 *  b) Redistribution and use in source and binary forms, with or
 *     without modification, are permitted provided that the following
 *     conditions are met:
 *
 *     1. Redistributions of source code must retain the above
 *        copyright notice, this list of conditions and the following
 *        disclaimer.
 *     2. Redistributions in binary form must reproduce the above
 *        copyright notice, this list of conditions and the following
 *        disclaimer in the documentation and/or other materials
 *        provided with the distribution.
 *
 *     THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND
 *     CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
 *     INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 *     MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 *     DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 *     CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 *     SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 *     NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 *     LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 *     HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 *     CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 *     OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 *     EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef _STATSLIMITEDMODEL_H
#define _STATSLIMITEDMODEL_H

#include "abstracttaskmodel.h"
#include "vtl/avltree.h"
#include "misc/traceshark.h"

namespace vtl {
       template<class T> class TList;
}


class Task;
class TaskHandle;

QT_BEGIN_NAMESPACE
class QStringList;
QT_END_NAMESPACE

class StatsLimitedModel : public AbstractTaskModel
{
	Q_OBJECT
public:
	StatsLimitedModel(QObject *parent = 0);
	~StatsLimitedModel();
	void setTaskMap(vtl::AVLTree<int, TaskHandle> *map,
			unsigned int nrcpus);
	int rowCount(const QModelIndex &parent) const;
	int columnCount(const QModelIndex &parent) const;
	QVariant data(const QModelIndex &index, int role) const;
	bool setData(const QModelIndex &index, const QVariant &value,
		     int role);
	QVariant headerData(int section, Qt::Orientation orientation,
			    int role) const;
	int rowToPid(int row, bool &ok) const;
	const QString &rowToName(int row, bool &ok) const;
	void rowToPct(QString &str, int row, bool &ok) const;
	void rowToTime(QString &str, int row, bool &ok) const;
	void beginResetModel();
	void endResetModel();
	Qt::ItemFlags flags(const QModelIndex &index) const;
private:
	vtl::TList<const Task*> *taskList;
	QString *errorStr;
	Task *idleTask;
};

#endif /* _STATSLIMITEDMODEL_H */