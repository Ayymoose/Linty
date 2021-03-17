// PC-Lint GUI
// Copyright (C) 2021  Ayymooose

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include <QPoint>
#include <QMainWindow>
#include <QToolBar>
#include <QToolButton>
#include <QAction>
#include <QMenu>
#include <QMap>
#include <QTreeWidgetItem>
#include <memory>

#include "ProgressWindow.h"
#include "Preferences.h"
#include "Lint.h"
#include "Log.h"
#include "CodeEditor.h"
#include "Highlighter.h"
#include "ModifiedFileThread.h"
#include "About.h"

class ProgressWindow;

QT_BEGIN_NAMESPACE
namespace Ui
{
class MainWindow;
}
QT_END_NAMESPACE

namespace
{
constexpr int LINT_TABLE_FILE_COLUMN = 0;
constexpr int LINT_TABLE_NUMBER_COLUMN = 1;
constexpr int LINT_TABLE_DESCRIPTION_COLUMN = 2;
constexpr int LINT_TABLE_LINE_COLUMN = 3;
};

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

signals:
    void signalUpdateTypes();

    void signalSetLinterData(const PCLint::LintData& lintData);
    void signalRemoveFile(const QString& deletedFile);
    void signalKeepFile(const QString& keepFile);


    void signalSetLintData(const PCLint::LintData& lintData);


    void signalStartLint();

public slots:
    void handleContextMenu(const QPoint& pos);
    void slotGetLinterData();


    void slotLintComplete(const PCLint::LintStatus& lintStatus, const QString& errorMessage) noexcept;


    void slotLintVersion(const PCLint::Version& version) noexcept;
    void slotGetLintData() noexcept;

    void slotProcessLintMessageGroup(const PCLint::LintMessageGroup& lintMessageGroup) noexcept;



private slots:
    void save();
    void on_aboutLinty_triggered();
    void on_actionRefresh_triggered();
    void on_actionLog_triggered();
    void on_actionLintProject_triggered();
    void on_actionPreferences_triggered();
    void on_actionLint_triggered();
    void on_lintTable_itemClicked(QTreeWidgetItem *item, int column);

public:
    void startLint(QString title);
    bool filterMessageType(const QString& type) const noexcept;

private:
    Ui::MainWindow* m_ui;
    std::unique_ptr<QToolBar> m_lowerToolbar;
    std::unique_ptr<QToolButton> m_buttonErrors;
    std::unique_ptr<QToolButton> m_buttonWarnings;
    std::unique_ptr<QToolButton> m_buttonInformation;
    std::unique_ptr<QAction> m_actionError;
    std::unique_ptr<QAction> m_actionWarning;
    std::unique_ptr<QAction> m_actionInformation;
    bool m_toggleError;
    bool m_toggleWarning;
    bool m_toggleInformation;
    QString m_lastProjectLoaded;
    QSet<QString> m_directoryFiles;
    std::unique_ptr<Preferences> m_preferences;
    std::unique_ptr<PCLint::Highlighter> m_highlighter;

    std::unique_ptr<QMenu> m_lintTableMenu;
    QMap<QString, QString> m_projectLintMap;
    int m_linterStatus;


    PCLint::LintMessages m_lintTreeMessages;
    int m_lintTreeErrors;
    int m_lintTreeWarnings;
    int m_lintTreeInformation;

    void clearLintTree() noexcept;
    void applyLintTreeFilter() noexcept;
    void addTreeMessageGroup(const PCLint::LintMessageGroup& lintMessageGroup) noexcept;

    bool verifyLint();
    QSet<QString> recursiveBuildSourceFileSet(const QString& directory);
    PCLint::About m_about;

    PCLint::Lint m_lint;


    void testLintTreeFilter() noexcept;

};
