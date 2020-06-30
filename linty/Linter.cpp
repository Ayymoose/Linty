#include "Linter.h"
#include <QDebug>
#include <QProcess>
#include <QXmlStreamReader>
#include "Log.h"
#include <QFileInfo>
#include <QMessageBox>
#include <windows.h>
#include <QRegularExpression>
#include <chrono>
#include <QThread>
#include <QThread>

Linter::Linter()
{
    m_supportedVersions.insert("PC-lint for C/C++ (NT) Vers. 9.00a, Copyright Gimpel Software 1985-2009");
    m_supportedVersions.insert("PC-lint for C/C++ (NT) Vers. 9.00b, Copyright Gimpel Software 1985-2009");
    m_supportedVersions.insert("PC-lint for C/C++ (NT) Vers. 9.00c, Copyright Gimpel Software 1985-2009");
    m_supportedVersions.insert("PC-lint for C/C++ (NT) Vers. 9.00d, Copyright Gimpel Software 1985-2009");
    m_supportedVersions.insert("PC-lint for C/C++ (NT) Vers. 9.00e, Copyright Gimpel Software 1985-2010");
    m_supportedVersions.insert("PC-lint for C/C++ (NT) Vers. 9.00f, Copyright Gimpel Software 1985-2010");
    m_supportedVersions.insert("PC-lint for C/C++ (NT) Vers. 9.00g, Copyright Gimpel Software 1985-2011");
    m_supportedVersions.insert("PC-lint for C/C++ (NT) Vers. 9.00h, Copyright Gimpel Software 1985-2011");
    m_supportedVersions.insert("PC-lint for C/C++ (NT) Vers. 9.00i, Copyright Gimpel Software 1985-2012");
    m_supportedVersions.insert("PC-lint for C/C++ (NT) Vers. 9.00j, Copyright Gimpel Software 1985-2012");
    m_supportedVersions.insert("PC-lint for C/C++ (NT) Vers. 9.00k, Copyright Gimpel Software 1985-2013");
    m_supportedVersions.insert("PC-lint for C/C++ (NT) Vers. 9.00L, Copyright Gimpel Software 1985-2014");
    m_supportedVersions.insert("PC-lint Plus 1.3 for Windows, Copyright Gimpel Software LLC 1985-2019");
    m_supportedVersions.insert("PC-lint Plus 1.3.5 for Windows, Copyright Gimpel Software LLC 1985-2020");

    m_numberOfErrors = 0;
    m_numberOfInfo = 0;
    m_numberOfWarnings = 0;
}

QSet<LintMessage> Linter::getLinterMessages() const
{
    return m_linterMessages;
}

void Linter::setLinterFile(const QString& lintFile)
{
    m_lintFile = lintFile;
}

void Linter::setLinterExecutable(const QString& linterExecutable)
{
    m_linterExecutable = linterExecutable;
}

void Linter::setLintFiles(const QList<QString>& files)
{
    m_filesToLint = files;
}

int Linter::numberOfInfo() const
{
    return m_numberOfInfo;
}

int Linter::numberOfWarnings() const
{
    return m_numberOfWarnings;
}

int Linter::numberOfErrors() const
{
    return m_numberOfErrors;
}

void Linter::slotGetLinterData(const LintData& lintData)
{
    setLintFiles(lintData.lintFiles);
    setLinterFile(lintData.lintOptionFile);
    setLinterExecutable(lintData.linterExecutable);
}

void Linter::setLinterMessages(const QSet<LintMessage>& lintMessages)
{
    m_linterMessages = lintMessages;
}

void Linter::setNumberOfErrors(int numberOfErrors)
{
    m_numberOfErrors = numberOfErrors;
}

void Linter::setNumberOfWarnings(int numberOfWarnings)
{
    m_numberOfWarnings = numberOfWarnings;
}

void Linter::setNumberOfInfo(int numberOfInfo)
{
    m_numberOfInfo = numberOfInfo;
}

void Linter::slotStartLint()
{
    LintResponse lintResponse;
    lintResponse.status = lint();
    lintResponse.lintMessages = m_linterMessages;
    lintResponse.numberOfErrors = m_numberOfErrors;
    lintResponse.numberOfWarnings = m_numberOfWarnings;
    lintResponse.numberOfInfo = m_numberOfInfo;
    emit signalLintFinished(lintResponse);
}

LINTER_STATUS Linter::lint()
{
    LINTER_STATUS status = LINTER_OK;
    qDebug() << "Linter::Thread ID: " << QThread::currentThreadId();
    DEBUG_LOG("Setting working directory to: " + QFileInfo(m_lintFile).canonicalPath());

    QProcess lintProcess;
    lintProcess.setWorkingDirectory(QFileInfo(m_lintFile).canonicalPath());

    // stderr has the module (file lint) progress
    // sttout has the actual data

    QByteArray lintData;

    connect(&lintProcess, &QProcess::errorOccurred, this, [&](const QProcess::ProcessError& error)
    {
        status = LINTER_PROCESS_ERROR;
        DEBUG_LOG("### Process error: " + error);
    });

    connect(&lintProcess, &QProcess::readyReadStandardOutput, this, [&]()
    {
        if (QThread::currentThread()->isInterruptionRequested())
        {
            status = LINTER_CANCEL;
            lintProcess.closeReadChannel(QProcess::StandardOutput);
            lintProcess.closeReadChannel(QProcess::StandardError);
        }

        QByteArray readStdOut = lintProcess.readAllStandardOutput();
        lintData.append(readStdOut);
    });

    QList<QString> lintedFiles;
    QByteArray pclintVersion;

    int processedFiles = 0;
    int totalFiles = m_filesToLint.size();
    std::chrono::steady_clock::time_point lintStartTime;
    float processingTimeTotal = 0;

    connect(&lintProcess, &QProcess::started, this, [&]()
    {
        lintStartTime = std::chrono::steady_clock::now();
    });

    connect(&lintProcess, &QProcess::readyReadStandardError, this,[&,this]()
    {

        if (QThread::currentThread()->isInterruptionRequested())
        {
            status = LINTER_CANCEL;
            lintProcess.closeReadChannel(QProcess::StandardOutput);
            lintProcess.closeReadChannel(QProcess::StandardError);
        }

        QByteArray readStdErr = lintProcess.readAllStandardError();

        // Example of module information received:
        //
        // "\r\n--- Module:   D:\\Users\\Ayman\\Desktop\\MerlinEmbedded\\Buhler Sortex Shared Code\\ATxmega\\Camera\\Cam_debug.c (C)\r\n"
        // "\r\n--- Module:   D:\\Users\\Ayman\\Desktop\\MerlinEmbedded\\Buhler Sortex Shared Code\\ATxmega\\Camera\\Cam_debug.c (CPP)\r\n"

        QRegularExpression fileRegularExpression("([A-Za-z]:\\\\(?:[^\\\\/:*?\"<>|\r\n]+\\\\)*[^\\\\/:*?\"<>|\r\n]*)");
        QRegularExpressionMatchIterator it = fileRegularExpression.globalMatch(readStdErr);

        // Get the file name + path by trimming the string
        if (pclintVersion.length() == 0)
        {
            // PC-Lint version is always the first line included in stderr
            int versionEnd = readStdErr.indexOf("\r\n");
            pclintVersion = readStdErr.left(versionEnd);

            // Some unknown executable or lint version we don't know about
            // TODO: License error check
            if (!m_supportedVersions.contains(QString(pclintVersion)))
            {
                status = LINTER_UNSUPPORTED_VERSION;
                lintProcess.closeReadChannel(QProcess::StandardOutput);
                lintProcess.closeReadChannel(QProcess::StandardError);
                DEBUG_LOG("### Failed to start lint because version unsupported: " + QString(pclintVersion));
            }
            // But it sometimes drags module information into the first read
            // Find any module information that sneaked into this chunk
            while (it.hasNext())
            {
                QRegularExpressionMatch match = it.next();
                QString file = match.captured(0);
                // -4 is to get rid of the " (C)" that the matcher thinks is part of the file name
                // But it's something lint adds to the end for some reason
                file = file.left(file.length()-4);
                if (!lintedFiles.contains(file))
                {
                    qDebug() << "Processed chunk: " << file;
                    emit signalUpdateProgress(++processedFiles);
                    lintedFiles.append(file);
                }
            }
        }
        else if (readStdErr.length() > 0)
        {
            int nowProcessedFiles = processedFiles;
            while (it.hasNext())
            {
                QRegularExpressionMatch match = it.next();
                QString file = match.captured(0);
                file = file.left(file.length()-4);

                // Linter can perform multiple passes over the same file
                // TODO: Support multiple passes
                // TODO: Fix process error when too many files
                if (!lintedFiles.contains(file))
                {
                    qDebug() << "Processed: " << file;
                    emit signalUpdateProgress(++processedFiles);
                    emit signalUpdateProcessedFiles(processedFiles,totalFiles);
                    lintedFiles.append(file);
                }
            }

            // TODO: Fix ETA unreliability
            // TODO: Throw useful error messages

            //if (processingTimeTotal == 0)
            {
                // In case we process more than 1 file in a single read of stderr
                int newProcessedFiles = (processedFiles - nowProcessedFiles);

                std::chrono::steady_clock::time_point lintEndTime = std::chrono::steady_clock::now();
                // Total processing time (in ms) for newProcessedFiles files
                processingTimeTotal = std::chrono::duration_cast<std::chrono::milliseconds>(lintEndTime - lintStartTime).count();

                // Processing time for 1 file
                float processingTime = (processingTimeTotal / 1000.0f) / newProcessedFiles;
                qDebug() << "Processed: " << newProcessedFiles << " file in " << (processingTimeTotal / 1000.0f) << "s";
                float eta = 1 + (processingTime * (m_filesToLint.size() - processedFiles));
                qDebug() << "ETA: " << eta << "s";
                lintStartTime = std::chrono::steady_clock::now();

                static float etaMax = 0;
                if (etaMax < eta)
                {
                    etaMax = eta;
                    emit signalUpdateETA(eta);
                }

            }


        }

    });

    QString cmdString = m_linterExecutable;

    // Clear existing arguments
    m_arguments.clear();

    // Extra arguments to produce XML output
    // Enable verbosity and module information displayed so we know progress
    m_arguments << ("+vm");
    // Control the message height and force file information per message
    m_arguments << ("-hFs1"); // was b
    // No line breaks in output
    m_arguments << ("-width(0)");

    // XML specific arguments
    // Put open/closing <doc> tags in document
    m_arguments << ("+xml(doc)");
    // Use minimal element names for faster output
    m_arguments << ("-format=<m><f>%f</f><l>%l</l><t>%t</t><n>%n</n><d>%m</d></m>");
    // Surpress specific walk messages
    m_arguments << ("-format_specific= ");

    // TODO: Test with various lint files

    // TODO: Support PC-Lint Plus
    // PC-Lint Plus argument
    //m_arguments << ("-max_threads=8");
    /*
     *
     * warning -> supplemental
     * info -> supplemental
     *
     */

   // m_arguments << ("-passes(6)");

    for (QString str : m_arguments)
    {
        cmdString += " \"" + str + "\"";
    }


    // Add the lint file
    m_arguments << (m_lintFile);

    cmdString += " \"" + m_lintFile + "\"";

    // Add all files to lint
    for (const QString& file : m_filesToLint)
    {
       m_arguments << (file);
       //DEBUG_LOG("Adding file to lint: " + file);
       cmdString += " \"" + file + "\"";
    }

    DEBUG_LOG("Lint path: " + m_linterExecutable);
    DEBUG_LOG("Lint file: " + m_lintFile);

    // Display arguments
    for (const QString& argument : m_arguments)
    {
        //DEBUG_LOG("Lint argument: " + argument);
    }

    // TODO: Temporary debug information
    QFile file("D:\\Users\\Ayman\\Desktop\\Linty\\test\\xmldata.xml");
    file.open(QIODevice::WriteOnly);
    file.write(cmdString.toLocal8Bit());
    file.close();
    //

    lintProcess.setProgram(m_linterExecutable);
    lintProcess.setArguments(m_arguments);
    lintProcess.start();

    m_numberOfErrors = 0;
    m_numberOfInfo = 0;
    m_numberOfWarnings = 0;

    if (!lintProcess.waitForStarted())
    {
        DEBUG_LOG("### " + lintProcess.errorString());
        return LINTER_PROCESS_ERROR;
    }

    // Estimate progress of lint
    // PC-Lint gives no progress indication other than which modules were processed
    // Therefore we have to estimate how long the entire lint process can possibly take
    // A simple way to do this is by
    // Calculating how long it look to process 1 module (tM) in seconds
    // Assume each module will take the same time tM so the total time is tM * number of files
    // Update tM by averaging it over the number of processed modules

    emit signalUpdateProgressMax(totalFiles);
    emit signalUpdateProcessedFiles(processedFiles,totalFiles);

    // Wait forever until finished
    if (!lintProcess.waitForFinished(-1))
    {
        DEBUG_LOG("### Lint finished unexpectedly");
        return LINTER_PROCESS_ERROR;
    }

    // Check linter version (if we can't determine the version then return error)
    if (status == LINTER_CANCEL)
    {
        return LINTER_CANCEL;
    }

    // They must be the same or something went horribly wrong internally
    // We must expect all the output files to be in our list
    //Q_ASSERT(lintedFiles.size() == m_filesToLint.size());

    //qDebug() << "###";
    //qDebug() << "Command line argument: " << cmdString;
    //qDebug() << "###";


    // Show lint version used
    DEBUG_LOG("Lint version: " + QString(pclintVersion));

    QString lintDataAsString = QString(lintData);

    // For PC-Lint, it generates useful "wrap-up messages" but provides no way to wrap them in the XML tags
    // So we must manually do that here

    // Method 1
    // Split byte array into qlist of byte array by \n
    // For each line, if not <doc> </doc> or <f> then escape it
    // Append to string

    /*QList<QByteArray> lines = lintData.split('\n');
    // The error we need to insert
    QString errorToInsert;
    // The XML message to insert into
    QString xmlMessage;

    QString xmlStringNew;
    xmlStringNew.reserve(lintDataAsString.size());
    xmlStringNew = XML_TAG_DOC_OPEN;
    xmlStringNew +="\n";

    for(const QByteArray& line : lines)
    {
        if (xmlMessage.length() > 0)
        {
            xmlMessage.insert(xmlMessage.indexOf(XML_TAG_DESCRIPTION_CLOSED), "\n" + errorToInsert);
            xmlStringNew += xmlMessage;
            xmlMessage.clear();
            errorToInsert.clear();
        }
        if (!(line.startsWith(XML_TAG_DOC_OPEN) || line.startsWith(XML_TAG_DOC_CLOSED) || line.startsWith(XML_TAG_MESSAGE_OPEN)))
        {
            // Escape the error message
            errorToInsert+= QString(line).toHtmlEscaped();
        }
        else if (line.startsWith(XML_TAG_MESSAGE_OPEN))
        {
            xmlMessage = line;
        }
    }

    xmlStringNew += "</doc>";*/

    qDebug() << "XML data size: " << lintData.size();

    // TODO: Temporary debug information
    QFile file2("D:\\Users\\Ayman\\Desktop\\Linty\\test\\xmldata.xml");
    file2.open(QIODevice::WriteOnly);
    file2.write(cmdString.toLocal8Bit());
    file2.close();
    //

    QSet<LintMessage> lintMessages;
    QXmlStreamReader lintXML(lintData);
    LintMessage message;

    // Start XML parsing
    // Parse the XML until we reach end of it
    while(!lintXML.atEnd() && !lintXML.hasError())
    {

        // Read next element
        QXmlStreamReader::TokenType token = lintXML.readNext();
        //If token is just StartDocument - go to next
        if(token == QXmlStreamReader::StartDocument)
        {
            continue;
        }
        //If token is StartElement - read it
        if(token == QXmlStreamReader::StartElement)
        {
            if(lintXML.name() == XML_ELEMENT_DOC || lintXML.name() == XML_ELEMENT_MESSAGE)
            {
                continue;
            }

            if(lintXML.name() == XML_ELEMENT_FILE)
            {
                message.file = lintXML.readElementText();
                //Q_ASSERT(message.file.length() > 0);
            }
            if(lintXML.name() == XML_ELEMENT_LINE)
            {
                message.line = lintXML.readElementText();
                Q_ASSERT(message.line.length() > 0);
            }
            if(lintXML.name() == XML_ELEMENT_MESSAGE_TYPE)
            {
                message.type = lintXML.readElementText();
                Q_ASSERT(message.type.length() > 0);
            }
            if(lintXML.name() == XML_ELEMENT_MESSAGE_NUMBER)
            {
                message.number = lintXML.readElementText();
                Q_ASSERT(message.number.length() > 0);
            }
            if(lintXML.name() == XML_ELEMENT_DESCRIPTION)
            {
                message.description = lintXML.readElementText();
                Q_ASSERT(message.description.length() > 0);
            }

        }

        if((token == QXmlStreamReader::EndElement) && (lintXML.name() == XML_ELEMENT_MESSAGE))
        {
            // Lint can spit out duplicates for some reason
            // This is a quick way to check if it was inserted or not
            int size = lintMessages.size();
            lintMessages.insert(message);
            if (lintMessages.size() > size)
            {
                // Ascertain type
                if (!QString::compare(message.type, LINT_TYPE_ERROR, Qt::CaseInsensitive))
                {
                    m_numberOfErrors++;
                }
                else if (!QString::compare(message.type, LINT_TYPE_WARNING, Qt::CaseInsensitive))
                {
                    m_numberOfWarnings++;
                }
                else if (!QString::compare(message.type, LINT_TYPE_INFO, Qt::CaseInsensitive))
                {
                    m_numberOfInfo++;
                }
                else
                {
                    // TODO: Fix for supplemental messages
                    // Unknown types are treated as informational messages with '?' icon
                    m_numberOfInfo++;
                }
            }
            //progress += (message.number + message.description + message.type + message.line + message.file).size();
            //emit signalUpdateProgress(progress);
            message = {};
        }
    }

    if (lintXML.hasError())
    {
        DEBUG_LOG("### XML parser error");
        DEBUG_LOG("Error Type:       " + QString(lintXML.error()));
        DEBUG_LOG("Error String:     " + lintXML.errorString());
        DEBUG_LOG("Line Number:      " + QString::number(lintXML.lineNumber()));
        DEBUG_LOG("Column Number:    " + QString::number(lintXML.columnNumber()));
        DEBUG_LOG("Character Offset: " + QString::number(lintXML.characterOffset()));
        return LINTER_PROCESS_ERROR;
    }


    m_linterMessages = lintMessages;

    return status;
}

QString Linter::getLintingDirectory() const
{
    return m_lintingDirectory;
}

QString Linter::getLinterExecutable() const
{
    return m_linterExecutable;
}

void Linter::removeAssociatedMessages(const QString& file)
{
    // Find the LintMessage who has the same file part
    QSet<LintMessage>::iterator it = m_linterMessages.begin();
    while (it != m_linterMessages.end())
    {
        if ((*it).file == file)
        {
            // Remove this message
            it = m_linterMessages.erase(it);
        }
        else
        {
            ++it;
        }
    }
}

void Linter::removeMessagesWithNumber(const QString& number)
{
    // Find the LintMessage who has the same code part
    QSet<LintMessage>::iterator it = m_linterMessages.begin();
    while (it != m_linterMessages.end())
    {
        if ((*it).number == number)
        {
            // Remove this message
            it = m_linterMessages.erase(it);
        }
        else
        {
            ++it;
        }
    }
}
