// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "environmentwidget.h"

#include <DFrame>
#include <DCheckBox>
#include <DTableView>

#include <QVBoxLayout>
#include <QHeaderView>

DWIDGET_USE_NAMESPACE

class EnvironmentModelPrivate
{
    friend class EnvironmentModel;
    QMap<QString, QString> envs;
};

EnvironmentModel::EnvironmentModel(QObject *parent)
    : QAbstractTableModel(parent)
    , d (new EnvironmentModelPrivate())
{

}

EnvironmentModel::~EnvironmentModel()
{
    if (d)
        delete d;
}

int EnvironmentModel::rowCount(const QModelIndex &) const
{
    return d->envs.keys().size();
}

int EnvironmentModel::columnCount(const QModelIndex &) const
{
    return kColumnCount;
}

QVariant EnvironmentModel::data(const QModelIndex &index, int role) const
{
    if (role == Qt::DisplayRole || role == Qt::EditRole) {
        auto var = d->envs.keys()[index.row()];
        switch (index.column()) {
        case kVaribale:
            return var;
        case kValue:
            return d->envs.value(var);
        default:
            break;
        }
    }
    return {};
}

bool EnvironmentModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (!index.isValid() || role != Qt::EditRole)
        return false;

    if (data(index, role) == value)
        return true;

    const QString oldName = data(this->index(index.row(), 0, QModelIndex()), Qt::EditRole).toString();
    const QString oldValue = data(this->index(index.row(), 1, QModelIndex()), Qt::EditRole).toString();
    QMap<QString, QString> map = d->envs;
    if (index.column() == kVaribale) {
        const QString newName = value.toString();
        if (newName.isEmpty() || newName.contains("=") || map.contains(newName))
            return false;
        map.remove(oldName);
        map.insert(value.toString(), oldValue);
    } else if (index.column() == kValue) {
        const QString stringValue = value.toString();
        auto var = map.keys()[index.row()];
        map[var] = stringValue;
    }
    update(map);
    emit dataChanged(index, index);
    return true;
}

Qt::ItemFlags EnvironmentModel::flags(const QModelIndex &index) const
{
    Q_UNUSED(index);
    return Qt::ItemIsEnabled | Qt::ItemIsEditable | Qt::ItemIsSelectable;
}

QVariant EnvironmentModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        switch (section) {
        case kVaribale:
            return QObject::tr("Variable");
        case kValue:
            return QObject::tr("Value");
        default:
            break;
        }
    }
    return {};
}

void EnvironmentModel::append(const QString &key, const QString &value)
{
    beginInsertRows({}, d->envs.keys().count(), d->envs.keys().count());
    d->envs.insert(key, value);
    endInsertRows();
}

void EnvironmentModel::update(const QMap<QString, QString> &data)
{
    beginResetModel();
    d->envs.clear();
    d->envs = data;
    endResetModel();
}

const QMap<QString, QString> EnvironmentModel::getEnvironment() const
{
    return d->envs;
}

class EnvironmentWidgetPrivate
{
    friend class EnvironmentWidget;

    QVBoxLayout *vLayout{nullptr};
    DTableView *tableView{nullptr};
    DCheckBox *checkBox{nullptr};
    EnvironmentModel *model{nullptr};

    config::EnvironmentItem *envShadow{nullptr};
};


EnvironmentWidget::EnvironmentWidget(QWidget *parent)
    : DFrame(parent)
    , d(new EnvironmentWidgetPrivate)
{
    setAutoFillBackground(true);

    if (!d->vLayout)
        d->vLayout = new QVBoxLayout();
    this->setLayout(d->vLayout);

    if (!d->tableView) {
        d->tableView = new DTableView(this);
        d->tableView->setAlternatingRowColors(true);
        d->tableView->setFrameShape(QFrame::NoFrame);

        // Initialize view
        d->tableView->setShowGrid(false);
        QHeaderView* headerView = d->tableView->horizontalHeader();
        headerView->setDefaultAlignment(Qt::AlignLeft);
        headerView->setSectionResizeMode(QHeaderView::Stretch);

        d->tableView->verticalHeader()->hide();
    }

    if (!d->model)
        d->model = new EnvironmentModel();

    d->tableView->setModel(d->model);

    if (!d->checkBox)
        d->checkBox = new DCheckBox(this);

    connect(d->checkBox, &DCheckBox::clicked, [this](){
        if (d->envShadow)
            d->envShadow->enable = d->checkBox->isChecked();
    });

    d->checkBox->setText(tr("Enable All Environment"));
    d->checkBox->setChecked(true);
    d->vLayout->setSpacing(0);
    d->vLayout->setMargin(0);
    d->vLayout->addWidget(d->tableView);
    d->vLayout->addWidget(d->checkBox);
}

EnvironmentWidget::~EnvironmentWidget()
{
    if(d)
        delete d;
}

void EnvironmentWidget::getValues(config::EnvironmentItem &env)
{
    env.enable = d->checkBox->isChecked();
    env.environments = d->model->getEnvironment();
}

void EnvironmentWidget::setValues(const config::EnvironmentItem &env)
{
    d->checkBox->setChecked(env.enable);
    d->model->update(env.environments);
}

void EnvironmentWidget::bindValues(config::EnvironmentItem *env)
{
    d->envShadow = env;
    d->checkBox->setChecked(env->enable);
    d->model->update(env->environments);
}


