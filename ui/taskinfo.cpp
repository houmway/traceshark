/*
 * Traceshark - a visualizer for visualizing ftrace and perf traces
 * Copyright (C) 2015, 2016  Viktor Rosendahl <viktor.rosendahl@gmail.com>
 *
 *   This program is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */


#include <QHBoxLayout>
#include <QIcon>
#include <QMap>
#include <QPushButton>
#include <QLabel>
#include <QLineEdit>
#include <QPixmap>
#include <QString>

#include "analyzer/task.h"
#include "ui/taskgraph.h"
#include "ui/taskinfo.h"
#include "misc/traceshark.h"
#include "qcustomplot/qcustomplot.h"

#define DEFINE_PIDMAP_ITERATOR(name) \
	QMap<unsigned int, TaskGraph*>::iterator name

#define ADD_TO_LEGEND_RESOURCE ":/traceshark/images/addtolegend30x30.png"
#define ADD_TASK_RESOURCE ":/traceshark/images/addtask30x30.png"
#define CLEAR_LEGEND_RESOURCE ":/traceshark/images/clearlegend30x30.png"
#define FIND_WAKEUP_RESOURCE ":/traceshark/images/wakeup30x30.png"
#define REMOVE_TASK_RESOURCE ":/traceshark/images/removetask30x30.png"

#define FIND_TOOLTIP \
	"Find the wakeup of this task that precedes the active cursor"
#define REMOVE_TASK_TOOLTIP \
	"Remove the unified graph for this task"

TaskInfo::TaskInfo(QWidget *parent):
	QWidget(parent), taskGraph(nullptr)
{
	QHBoxLayout *layout  = new QHBoxLayout(this);
	QLabel *colonLabel = new QLabel(tr(":"));

	nameLine = new QLineEdit(this);
	pidLine = new QLineEdit(this);

	nameLine->setReadOnly(true);
	pidLine->setReadOnly(true);

	layout->addWidget(nameLine);
	layout->addWidget(colonLabel);
	layout->addWidget(pidLine);

	createActions();
	createToolBar();

	layout->addWidget(taskToolBar);
	layout->addStretch();

	tsconnect(addToLegendAction, triggered(), this,
		addToLegendTriggered());
	tsconnect(addTaskGraphAction, triggered(), this,
		addTaskGraphTriggered());
	tsconnect(removeTaskGraphAction, triggered(), this,
		removeTaskGraphTriggered());
	tsconnect(clearAction, triggered(), this, clearTriggered());
	tsconnect(findAction, triggered(), this, findTriggered());
}

void TaskInfo::createActions()
{
	addTaskGraphAction = new QAction(tr("Add task graph"), this);
	addTaskGraphAction->setIcon(QIcon(ADD_TASK_RESOURCE));
	/* addTaskGraphAction->setShortcuts(I_dont_know); */
	addTaskGraphAction->setToolTip(tr("Add a unified graph for this task"));

	addToLegendAction = new QAction(tr("Add task to the legend"), this);
	addToLegendAction->setIcon(QIcon(ADD_TO_LEGEND_RESOURCE));
	/* addToLegendAction->setShortcuts(I_dont_know); */
	addToLegendAction->setToolTip(tr("Add this task to the legend"));

	clearAction = new QAction(tr("Clear the legend"), this);
	clearAction->setIcon(QIcon(CLEAR_LEGEND_RESOURCE));
	/* clearAction->setShortcuts(I_dont_know) */
	clearAction->setToolTip(tr("Remove all tasks from the legend"));

	findAction = new QAction(tr("Find wakeup"), this);
	findAction->setIcon(QIcon(FIND_WAKEUP_RESOURCE));
	/* findAction->setShortCuts(I_dont_know) */
	findAction->setToolTip(tr(FIND_TOOLTIP));

	removeTaskGraphAction = new QAction(tr("Remove task graph"), this);
	removeTaskGraphAction->setIcon(QIcon(REMOVE_TASK_RESOURCE));
	/* removeTaskGraphAction->setShortCuts(I_dont_know) */
	removeTaskGraphAction->setToolTip(tr(REMOVE_TASK_TOOLTIP));
}

void TaskInfo::createToolBar()
{
	taskToolBar = new QToolBar(tr("Task Toolbar"), this);
	taskToolBar->addAction(addToLegendAction);
	taskToolBar->addAction(clearAction);
	taskToolBar->addAction(findAction);
	taskToolBar->addAction(addTaskGraphAction);
	taskToolBar->addAction(removeTaskGraphAction);
}

TaskInfo::~TaskInfo()
{
}


void TaskInfo::setTaskGraph(TaskGraph *graph)
{
	TaskGraph *legendGraph;
	Task *task = graph->getTask();
	if (task == nullptr)
		return;
	QString nameStr = task->getLastName();
	QString pidStr = QString::number(task->pid);
	nameLine->setText(nameStr);
	pidLine->setText(pidStr);
	/* taskGraph will be used for displaying the legend in case the user
	 * pushes that button. For that reason we will check if this TaskGraph
	 * has a pointer to another TaskGraph that is to be used for legend
	 * displaying purposes. In practice this happens when a unfied TaskGraph
	 * is set here, because that one might get deleted by user action, it
	 * instead has a pointer to a per CPU TaskGraph */
	legendGraph = graph->getTaskGraphForLegend();
	taskGraph = legendGraph != nullptr ? legendGraph : graph;
}

void TaskInfo::removeTaskGraph()
{
	taskGraph = nullptr;
	nameLine->setText(tr(""));
	pidLine->setText(tr(""));
}

void TaskInfo::clear()
{
	removeTaskGraph();
	legendPidMap.clear();
}

void TaskInfo::addToLegendTriggered()
{
	if (taskGraph == nullptr)
		return;

	addTaskGraphToLegend(taskGraph);
}


void TaskInfo::addTaskGraphToLegend(TaskGraph *graph)
{
	QObject *obj;
	QCustomPlot *plot;
	Task *task;

	task = graph->getTask();
	if (task == nullptr)
		return;

	/* We use the legendPidMap to keep track of pids that have been added
	 * to the legend, otherwise it could happen that someone added the same
	 * task multiple times to the legend and that looks a bit crazy. While
	 * QCustomPlot prevents the same the same LegendGraph object being
	 * added twice we can have multiple identical objects, since tasks can
	 * migrate between CPUs */
	if (legendPidMap.contains(task->pid))
		return;

	legendPidMap[task->pid] = graph;
	graph->addToLegend();
	obj = graph->parent();
	plot = qobject_cast<QCustomPlot *>(obj);
	if (plot != nullptr)
		plot->replot();
}


void TaskInfo::clearTriggered()
{
	QCustomPlot *plot = nullptr;
	QObject *obj;
	DEFINE_PIDMAP_ITERATOR(iter) = legendPidMap.begin();
	while(iter != legendPidMap.end()) {
		TaskGraph *&graph = iter.value();
		graph->removeFromLegend();
		if (plot == nullptr) {
			obj = graph->parent();
			plot = qobject_cast<QCustomPlot *>(obj);
		}
		iter++;
	}
	if (plot != nullptr)
		plot->replot();
	legendPidMap.clear();
}

void TaskInfo::findTriggered()
{
	Task *task;

	if (taskGraph == nullptr)
		return;

	task = taskGraph->getTask();
	if (task == nullptr)
		return;

	emit findWakeup(task->pid);
}

void TaskInfo::addTaskGraphTriggered()
{
	Task *task;

	if (taskGraph == nullptr)
		return;

	task = taskGraph->getTask();
	if (task == nullptr)
		return;

	emit addTaskGraph(task->pid);
}

void TaskInfo::removeTaskGraphTriggered()
{
	Task *task;

	if (taskGraph == nullptr)
		return;

	task = taskGraph->getTask();
	if (task == nullptr)
		return;

	emit removeTaskGraph(task->pid);
}

void TaskInfo::checkGraphSelection()
{
	if (taskGraph == nullptr)
		return;
	if (taskGraph->selected())
		return;
	removeTaskGraph();
}

void TaskInfo::pidRemoved(unsigned int pid)
{
	legendPidMap.remove(pid);
}
