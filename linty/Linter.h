#ifndef LINTER_H
#define LINTER_H

#include <QString>
#include <QMetaType>
#include <QSet>

enum LINTER_STATUS
{
    LINTER_OK = 0,
    LINTER_UNSUPPORTED_VERSION,
    LINTER_FAIL,
    LINTER_ERROR,
};

typedef struct
{
    QString file;
    QString line;
    QString type;
    QString code;
    QString description;
} lintMessage;

// XML related definitions
#define XML_DOC "doc"
#define XML_MESSAGE "message"
#define XML_FILE "file"
#define XML_LINE "line"
#define XML_TYPE "type"
#define XML_CODE "code"
#define XML_DESCRIPTION "description"

// Linter output related
#define TYPE_ERROR "Error"
#define TYPE_INFORMATION "Info"
#define TYPE_WARNING "Warning"


class ProjectSolution
{
public:
     virtual QList<QString> buildSourceFiles(const QString& projectFileName) = 0;
};

class AtmelStudio7ProjectSolution : public ProjectSolution
{
public:

     AtmelStudio7ProjectSolution() = default;
     ~AtmelStudio7ProjectSolution() = default;
     QList<QString> buildSourceFiles(const QString& projectFileName) override;
};

class Linter
{
public:
    Linter();
    void setLinterExecutable(const QString& linterExecutable);
    void setLinterFile(const QString& lintFile);
    void setLinterDirectory(const QString& linterDirectory);
    void setLintFiles(const QList<QString>& files);

    // Lint a directory or some files
    LINTER_STATUS lint(QSet<lintMessage>& lintOutputMessages);

    QString getLintingDirectory() const;


private:
    QString m_lintingDirectory;
    QSet<QString> m_supportedVersions;
    QStringList m_arguments;
    QString m_linterExecutable;
    QString m_lintFile;
    QList<QString> m_filesToLint;
    void addArgument(QString argument);
};

inline bool operator==(const lintMessage &e1, const lintMessage &e2)
{
    return (e1.code == e2.code) && (e1.file == e2.file) && (e1.line == e2.line) && (e1.type == e2.type) && (e1.description == e2.description);
}

inline uint qHash(const lintMessage &key, uint seed)
{
    return qHash(key.code + key.line, seed) ^ qHash(key.file + key.type + key.description, seed);
}


#endif // LINTER_H
