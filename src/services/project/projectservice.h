// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef PROJECTSERVICE_H
#define PROJECTSERVICE_H

#include "projectgenerator.h"
#include "common/project/projectinfo.h"

#include <QTabWidget>

#define Parsing_State_Role Qt::UserRole + 100
enum ParsingState {
    Wait,
    Done
};
Q_DECLARE_METATYPE(ParsingState);

namespace dpfservice {

struct Target {
    QString name;
    QString srcPath;
    QString targetID;

    QString buildCommand;
    QStringList buildArguments;
    QString buildTarget;

    QString workingDir;
    QString output;

    // TODO(mozart):tempory
    bool enableEnv;

    bool operator==(const Target &other) const
    {
        if (name == other.name
                && srcPath == other.srcPath)
            return true;

        return false;
    }
};
using Targets = QVector<Target>;

enum TargetType {
    kUnknown,
    kBuildTarget,
    kRebuildTarget,
    kCleanTarget,
    kActiveExecTarget
};

class ProjectService final : public dpf::PluginService,
        dpf::AutoServiceRegister<ProjectService>,
        dpf::QtClassFactory<ProjectGenerator>,
        dpf::QtClassManager<ProjectGenerator>
{
    Q_OBJECT
    Q_DISABLE_COPY(ProjectService)
    typedef dpf::QtClassManager<ProjectGenerator> GeneratorProManager;
    typedef dpf::QtClassFactory<ProjectGenerator> GeneratorProFactory;
public:
    static QString name()
    {
        return "org.deepin.service.ProjectService";
    }

    explicit ProjectService(QObject *parent = nullptr)
        : dpf::PluginService (parent)
    {

    }

    /*!
     * \brief supportGenerator 獲取支持的工程名稱
     * \return
     */
    template<class T>
    QStringList supportGeneratorName()
    {
        if (std::is_same<ProjectGenerator, T>())
            return GeneratorProFactory::createKeys();
        else {
            qCritical() << "must ProjectGenerator, "
                        << "not support " << typeid (T).name();
            abort();
        }
    }

    /*!
     * \brief implGenerator 導入工程文件生成器對象 T = Project::Generator類泛化對象
     * \param name 生成器對象名稱(唯一鍵)
     * \param errorString 錯誤信息
     */
    template<class T>
    bool implGenerator(const QString &name, QString *errorString = nullptr)
    {
        if (std::is_base_of<ProjectGenerator, T>())
            return GeneratorProFactory::regClass<T>(name, errorString);
        else {
            qCritical() << "must base class ProjectGenerator, "
                        << "not support " << typeid (T).name();
            abort();
        }
    }

    /*!
     * \brief createGenerator
     * \param name 名稱
     * \param errorString 錯誤信息
     * \return 生成器對象實例
     */
    template<class T>
    T *createGenerator(const QString &name, QString *errorString = nullptr)
    {
        if (std::is_base_of<ProjectGenerator, T>()) {
            Generator *generator = GeneratorProManager::value(name);
            if (!generator) {
                generator = GeneratorProFactory::create(name, errorString);
                if (generator)
                    GeneratorProManager::append(name, dynamic_cast<ProjectGenerator*>(generator));
            }
            return dynamic_cast<T*>(generator);
        } else {
            qCritical() << "must base class or ProjectGenerator, "
                        << "not support "<< typeid (T).name();
            abort();
        }
    }

    /*!
     * \brief name
     */
    template<class T>
    QString name(T* value) const{
        return QtClassManager<T>::key(value);
    }

    template<class T>
    QList<T*> values() const {
        return QtClassManager<T>::values();
    }

    /*!
     * \brief getAllProjectInfo
     */
    DPF_INTERFACE(QList<dpfservice::ProjectInfo>, getAllProjectInfo, void);

    /*!
     * \brief getProjectInfo
     * \param kitName workspace
     */
    DPF_INTERFACE(dpfservice::ProjectInfo, getProjectInfo, const QString &kitName, const QString &workspace);

    /*!
     * \brief updateProjectInfo
     * \param projectInfo
     */
    DPF_INTERFACE(bool, updateProjectInfo, ProjectInfo &projectInfo);

    /*!
     * \brief expandItemByFile
     * \param filepath
     */
    DPF_INTERFACE(void, expandItemByFile, const QStringList &filePaths);

    /**
     * @brief getActiveProjectInfo
     */
    DPF_INTERFACE(dpfservice::ProjectInfo, getActiveProjectInfo);

    /**
     * @brief DPF_INTERFACE
     * @param projectInfo
     */
    DPF_INTERFACE(bool, hasProjectInfo, const dpfservice::ProjectInfo &projectInfo);

    /*!
     * \brief getActiveTarget
     * \param TargetType
     * \return Target
     */
    DPF_INTERFACE(Target, getActiveTarget, TargetType);

    /////////////////////////project view interface////////////////////////////

    /*!
     * \brief addRootItem Add project root data node
     * \param aitem
     */
    DPF_INTERFACE(void, addRootItem, QStandardItem *aitem);

    /*!
     * \brief removeRootItem Delete the project root data node
     * \param aitem
     */
    DPF_INTERFACE(void, removeRootItem, QStandardItem *aitem);

    /*!
     * \brief takeRootItem Removes but does not delete the project node from the View
     * \param aitem
     */
    DPF_INTERFACE(void, takeRootItem, QStandardItem *aitem);

    /*!
     * \brief expandedDepth Expand project sub-items based on depth
     * \param aitem
     * \param depth
     */
    DPF_INTERFACE(void, expandedDepth, QStandardItem *aitem, int depth);

    /*!
     * \brief expandedAll Expand all sub-items of the project
     * \param aitem
     */
    DPF_INTERFACE(void, expandedAll, QStandardItem *aitem);
};

/* MainWindow codeediter workspace title,
 * use in window service swtich workspace
 */
inline const QString MWCWT_PROJECTS {QTabWidget::tr("Projects")};

} //namespace dpfservice

#endif // PROJECTSERVICE_H
