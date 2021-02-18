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

#include <QList>
#include <QString>

class ProjectSolution
{
public:
    ProjectSolution() = default;
    virtual ~ProjectSolution() = default;
    virtual QSet<QString> buildSourceFiles(const QString& projectFileName) = 0;
    virtual void setDirectory(const QString&) = 0;
};

class AtmelStudio7ProjectSolution : public ProjectSolution
{
public:
     void setDirectory(const QString&) override {}
     QSet<QString> buildSourceFiles(const QString& projectFileName) override;
};

// .vcxproj
class VisualStudioProject : public ProjectSolution
{
public:
     void setDirectory(const QString&) override {}
     QSet<QString> buildSourceFiles(const QString& projectFileName) override;
};

// .slns
class VisualStudioProjectSolution : public ProjectSolution
{
public:

     void setDirectory(const QString&) override;
     QSet<QString> buildSourceFiles(const QString& projectFileName) override;
private:
     QString m_directory;
};
