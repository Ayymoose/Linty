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

#include "MainWindow.h"
#include <QApplication>
#include <QScreen>
#include "Log.h"
#include "CodeEditor.h"
#include "Jenkins.h"
#include <QObject>
#include <QProcess>


int main(int argc, char *argv[])
{
    QApplication EditorApp(argc, argv);

    QCoreApplication::setOrganizationName("PC-Lint GUI");
    QCoreApplication::setApplicationName("PC-Lint GUI");

    Log::createLogFile(LOG_FILENAME);
    DEBUG_LOG("---------- Starting PC-Lint GUI ----------");
    DEBUG_LOG("PC-Lint GUI version: " BUILD_VERSION);

    MainWindow mainWindow;

    // Center the screen
    QScreen* screen = QGuiApplication::primaryScreen();
    QSize screenSize = screen->size();

    int x = (screenSize.width()-mainWindow.width()) / 2;
    int y = (screenSize.height()-mainWindow.height() - 40) / 2;
    mainWindow.move(x, y);
    mainWindow.showMaximized();
    mainWindow.setWindowTitle(APPLICATION_NAME " " BUILD_VERSION);

    return EditorApp.exec();
}
