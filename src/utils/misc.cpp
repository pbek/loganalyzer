/*
 * Copyright (C) 2016 Patrizio Bekerle -- http://www.bekerle.com
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 2 of the License.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License
 * for more details.
 *
 */

#include <QtCore>
#include <QProcess>
#include <QDesktopServices>
#include <QDir>
#include <QUrl>
#include <QRegularExpression>
#include <QtCore/QCoreApplication>
#include <QtWidgets/QApplication>
#include <cmath>
#include <stdexcept>
#include <QtGui/QIcon>
#include "misc.h"
#include "libraries/miniz/tinfl.c"
#include "version.h"

#ifdef Q_OS_WIN
#include <windows.h>
#endif



static struct { const char *source; const char *comment; } units[] = {
        QT_TRANSLATE_NOOP3("misc", "B", "bytes"),
        QT_TRANSLATE_NOOP3("misc", "kB", "kibibytes (1024 bytes)"),
        QT_TRANSLATE_NOOP3("misc", "MB", "mebibytes (1024 kibibytes)"),
        QT_TRANSLATE_NOOP3("misc", "GB", "gibibytes (1024 mibibytes)"),
        QT_TRANSLATE_NOOP3("misc", "TB", "tebibytes (1024 gibibytes)"),
        QT_TRANSLATE_NOOP3("misc", "PB", "pebibytes (1024 tebibytes)"),
        QT_TRANSLATE_NOOP3("misc", "EB", "exbibytes (1024 pebibytes)")
};

static tinfl_decompressor inflator;

static QByteArray result(TINFL_LZ_DICT_SIZE, 0);


/**
 * Open the given path with an appropriate application
 * (thank you to qBittorrent for the inspiration)
 */
void Utils::Misc::openPath(const QString& absolutePath)
{
    const QString path = QDir::fromNativeSeparators(absolutePath);
    // Hack to access samba shares with QDesktopServices::openUrl
    if (path.startsWith("//"))
        QDesktopServices::openUrl(QDir::toNativeSeparators("file:" + path));
    else
        QDesktopServices::openUrl(QUrl::fromLocalFile(path));
}

/**
 * Opens the parent directory of the given path with a file manager and select
 * (if possible) the item at the given path
 * (thank you to qBittorrent for the inspiration)
 */
void Utils::Misc::openFolderSelect(const QString& absolutePath)
{
    const QString path = QDir::fromNativeSeparators(absolutePath);
#ifdef Q_OS_WIN
    if (QFileInfo(path).exists()) {
        // Syntax is: explorer /select, "C:\Folder1\Folder2\file_to_select"
        // Dir separators MUST be win-style slashes

        // QProcess::startDetached() has an obscure bug. If the path has
        // no spaces and a comma(and maybe other special characters) it doesn't
        // get wrapped in quotes. So explorer.exe can't find the correct path
        // anddisplays the default one. If we wrap the path in quotes and pass
        // it to QProcess::startDetached() explorer.exe still shows the default
        // path. In this case QProcess::startDetached() probably puts its
        // own quotes around ours.

        STARTUPINFO startupInfo;
        ::ZeroMemory(&startupInfo, sizeof(startupInfo));
        startupInfo.cb = sizeof(startupInfo);

        PROCESS_INFORMATION processInfo;
        ::ZeroMemory(&processInfo, sizeof(processInfo));

        QString cmd = QString("explorer.exe /select,\"%1\"")
            .arg(QDir::toNativeSeparators(absolutePath));
        LPWSTR lpCmd = new WCHAR[cmd.size() + 1];
        cmd.toWCharArray(lpCmd);
        lpCmd[cmd.size()] = 0;

        bool ret = ::CreateProcessW(
            NULL, lpCmd, NULL, NULL, FALSE, 0, NULL, NULL,
            &startupInfo, &processInfo);
        delete [] lpCmd;

        if (ret) {
            ::CloseHandle(processInfo.hProcess);
            ::CloseHandle(processInfo.hThread);
        }
    } else {
        // If the item to select doesn't exist, try to open its parent
        openPath(path.left(path.lastIndexOf("/")));
    }
#elif defined(Q_OS_UNIX) && !defined(Q_OS_MAC)
    if (QFileInfo(path).exists()) {
        QProcess proc;
        QString output;
        proc.start(
                "xdg-mime",
                QStringList() << "query" << "default" << "inode/directory");
        proc.waitForFinished();
        output = proc.readLine().simplified();
        if (output == "dolphin.desktop" ||
                output == "org.kde.dolphin.desktop") {
            proc.startDetached(
                    "dolphin",
                    QStringList() << "--select"
                    << QDir::toNativeSeparators(path));
        } else if (output == "nautilus.desktop" ||
                    output == "org.gnome.Nautilus.desktop" ||
                    output == "nautilus-folder-handler.desktop") {
            proc.startDetached(
                    "nautilus",
                    QStringList() << "--no-desktop"
                    << QDir::toNativeSeparators(path));
        } else if (output == "caja-folder-handler.desktop") {
            proc.startDetached(
                    "caja",
                    QStringList() << "--no-desktop"
                    << QDir::toNativeSeparators(path));
        } else if (output == "nemo.desktop") {
            proc.startDetached(
                    "nemo",
                    QStringList() << "--no-desktop"
                    << QDir::toNativeSeparators(path));
        } else if (output == "konqueror.desktop" ||
                   output == "kfmclient_dir.desktop") {
            proc.startDetached(
                    "konqueror",
                    QStringList() << "--select"
                    << QDir::toNativeSeparators(path));
        } else {
            openPath(path.left(path.lastIndexOf("/")));
        }
    } else {
        // if the item to select doesn't exist, try to open its parent
        openPath(path.left(path.lastIndexOf("/")));
    }
#else
    openPath(path.left(path.lastIndexOf("/")));
#endif
}

/**
 * Removes a string from the start if it starts with it
 */
QString Utils::Misc::removeIfStartsWith(QString text, QString removeString) {
    if (text.startsWith(removeString)) {
        text.remove(QRegularExpression(
                "^" + QRegularExpression::escape(removeString)));
    }

    return text;
}

/**
 * Removes a string from the end if it ends with it
 */
QString Utils::Misc::removeIfEndsWith(QString text, QString removeString) {
    if (text.endsWith(removeString)) {
        text.remove(QRegularExpression(
                QRegularExpression::escape(removeString) + "$"));
    }

    return text;
}

/**
 * Adds a string to the beginning of a string if it doesn't start with it
 */
QString Utils::Misc::prependIfDoesNotStartWith(
        QString text, QString startString) {
    if (!text.startsWith(startString)) {
        text.prepend(startString);
    }

    return text;
}

/**
 * Adds a string to the end of a string if it doesn't end with it
 */
QString Utils::Misc::appendIfDoesNotEndWith(
        QString text, QString endString) {
    if (!text.endsWith(endString)) {
        text.append(endString);
    }

    return text;
}

/**
 * (thank you to qBittorrent for the inspiration)
 */
QString Utils::Misc::unitString(Utils::Misc::SizeUnit unit)
{
    return QCoreApplication::translate(
            "misc",
            units[static_cast<int>(unit)].source,
            units[static_cast<int>(unit)].comment);
}

/**
 * Returns best user-friendly storage unit (B, KiB, MiB, GiB, TiB, ...)
 *
 * use Binary prefix standards from IEC 60027-2
 * see http://en.wikipedia.org/wiki/Kilobyte
 * value must be given in bytes
 * to send numbers instead of strings with suffixes
 * (thank you to qBittorrent for the inspiration)
 */
bool Utils::Misc::friendlyUnit(
        qint64 sizeInBytes, qreal &val, Utils::Misc::SizeUnit &unit)
{
    if (sizeInBytes < 0) return false;

    int i = 0;
    qreal rawVal = static_cast<qreal>(sizeInBytes);

    while ((rawVal >= 1024.) && (i <= static_cast<int>(SizeUnit::ExbiByte))) {
        rawVal /= 1024.;
        ++i;
    }
    val = rawVal;
    unit = static_cast<SizeUnit>(i);
    return true;
}

/**
 * (thank you to qBittorrent for the inspiration)
 */
QString Utils::Misc::friendlyUnit(qint64 bytesValue, bool isSpeed)
{
    SizeUnit unit;
    qreal friendlyVal;
    if (!friendlyUnit(bytesValue, friendlyVal, unit)) {
        return QCoreApplication::translate("misc", "Unknown", "Unknown (size)");
    }
    QString ret;
    if (unit == SizeUnit::Byte)
        ret = QString::number(bytesValue) + " " + unitString(unit);
    else
        ret = fromDouble(friendlyVal, 1) + " " + unitString(unit);
    if (isSpeed)
        ret += QCoreApplication::translate("misc", "/s", "per second");
    return ret;
}

/**
 * To send numbers instead of strings with suffixes
 * (thank you to qBittorrent for the inspiration)
 */
QString Utils::Misc::fromDouble(double n, int precision)
{
    /* HACK because QString rounds up. Eg QString::number(0.999*100.0, 'f' ,1) == 99.9
    ** but QString::number(0.9999*100.0, 'f' ,1) == 100.0 The problem manifests when
    ** the number has more digits after the decimal than we want AND the digit after
    ** our 'wanted' is >= 5. In this case our last digit gets rounded up. So for each
    ** precision we add an extra 0 behind 1 in the below algorithm. */

    double prec = std::pow(10.0, precision);
    return QLocale::system().toString(
            std::floor(n * prec) / prec, 'f', precision);
}

/**
 * Decompresses gzipped data with the help of miniz
 */
QByteArray Utils::Misc::gUncompress(QByteArray const& data)
{
    mz_uint8 const* inPtr(reinterpret_cast<mz_uint8 const*>(data.data()) + 10);

    tinfl_init(&inflator);

    size_t inAvail(data.size());
    size_t outTotal(0);

    tinfl_status ret;

    do {
        size_t inSize(inAvail);
        size_t outSize(result.size() - outTotal);

        ret = tinfl_decompress(
                &inflator,
                inPtr,
                &inSize,
                reinterpret_cast<mz_uint8*>(result.data()),
                reinterpret_cast<mz_uint8*>(result.data()) + outTotal,
                &outSize,
                0);

        switch (ret) {
            case TINFL_STATUS_HAS_MORE_OUTPUT:
                inAvail -= inSize;
                inPtr += inSize;

                result.resize(2 * result.size());

            case TINFL_STATUS_DONE:
                outTotal += outSize;
                break;

            default:
                throw std::runtime_error("error decompressing gzipped content");
        }
    } while (TINFL_STATUS_DONE != ret);

    return QByteArray::fromRawData(result.data(), outTotal);
}
/**
 * @brief Prepares the debug information to output it as markdown
 * @param headline
 * @param data
 */
QString Utils::Misc::prepareDebugInformationLine(
        const QString &headline, QString data, bool withGitHubLineBreaks,
        QString typeText) {
    // add two spaces if we don't want GitHub line breaks
    QString spaces = withGitHubLineBreaks ? "" : "  ";

    if (data.contains("\n")) {
        data = "\n```\n" + data.trimmed() + "\n```";
    } else {
        data = (data.isEmpty()) ? "*empty*" : "`" + data + "`";
    }

    QString resultText = "**" + headline + "**";

    if (!typeText.isEmpty()) {
        resultText += " (" + typeText + ")";
    }

    resultText += ": " + data + spaces + "\n";
    return resultText;
}

QString Utils::Misc::generateDebugInformation(bool withGitHubLineBreaks) {
    QSettings settings;
    QString output;

    output += "LogAnalyzer Debug Information\n";
    output += "=============================\n";

    QDateTime dateTime = QDateTime::currentDateTime();

    // add information about QOwnNotes
    output += "\n## General Info\n\n";
    output += prepareDebugInformationLine("Current Date", dateTime.toString(),
                                          withGitHubLineBreaks);
    output += prepareDebugInformationLine("Version", QString(VERSION), withGitHubLineBreaks);
    output += prepareDebugInformationLine("Build date", QString(__DATE__), withGitHubLineBreaks);

#if (QT_VERSION >= QT_VERSION_CHECK(5, 4, 0))
    output += prepareDebugInformationLine("Operating System",
                                          QSysInfo::prettyProductName(), withGitHubLineBreaks);
    output += prepareDebugInformationLine("Build architecture",
                                          QSysInfo::buildCpuArchitecture(), withGitHubLineBreaks);
    output += prepareDebugInformationLine("Current architecture",
                                          QSysInfo::currentCpuArchitecture(), withGitHubLineBreaks);
#endif
    output += prepareDebugInformationLine("Qt Version (build)", QT_VERSION_STR, withGitHubLineBreaks);
    output += prepareDebugInformationLine("Qt Version (runtime)", qVersion(), withGitHubLineBreaks);
    output += prepareDebugInformationLine("Settings path / key",
                                          settings.fileName(), withGitHubLineBreaks);
    output += prepareDebugInformationLine("Application arguments",
                                          qApp->property("arguments").toStringList().join("`, `"), withGitHubLineBreaks);

    QString debug = "0";
#ifdef QT_DEBUG
    debug = "1";
#endif

    output += prepareDebugInformationLine("Qt Debug", debug, withGitHubLineBreaks);

    output += prepareDebugInformationLine("Locale (system)",
                                          QLocale::system().name(), withGitHubLineBreaks);
    output += prepareDebugInformationLine("Locale (interface)",
                                          settings.value("interfaceLanguage")
                                                  .toString(), withGitHubLineBreaks);

    output += prepareDebugInformationLine("Icon theme",
                                          QIcon::themeName(), withGitHubLineBreaks);

    // add information about the settings
    output += "\n## Settings\n\n";

    // hide values of these keys
    QStringList keyHiddenList = (QStringList() << "cryptoKey");

    // under OS X we have to ignore some keys
#ifdef Q_OS_MAC
    QStringList keyIgnoreList;
    keyIgnoreList << "AKDeviceUnlockState" << "Apple" << "NS" << "NavPanel"
    << "com/apple";
#endif

    QListIterator<QString> itr(settings.allKeys());
    while (itr.hasNext()) {
        QString key = itr.next();
        QVariant value = settings.value(key);

        // under OS X we have to ignore some keys
#ifdef Q_OS_MAC
        bool ignoreKey = false;

        // ignore values of certain keys
        QListIterator<QString> itr2(keyIgnoreList);
        while (itr2.hasNext()) {
            QString pattern = itr2.next();
            if (key.startsWith(pattern)) {
                ignoreKey = true;
                break;
            }
        }

        // check if key has to be ignored
        if (ignoreKey) {
            continue;
        }
#endif

        // hide values of certain keys
        if (keyHiddenList.contains(key)) {
            output += prepareDebugInformationLine(key, "<hidden>",
                                                  withGitHubLineBreaks, value.typeName());
        } else {
            switch (value.type()) {
                case QVariant::StringList:
                    output += prepareDebugInformationLine(
                            key, value.toStringList().join(", "),
                            withGitHubLineBreaks, value.typeName());
                    break;
                case QVariant::List:
                    output += prepareDebugInformationLine(key,
                                                          QString("<variant list with %1 item(s)>").arg(
                                                                  value.toList().count()),
                                                          withGitHubLineBreaks, value.typeName());
                    break;
                case QVariant::ByteArray:
                case QVariant::UserType:
                    output += prepareDebugInformationLine(key, "<binary data>",
                                                          withGitHubLineBreaks, value.typeName());
                    break;
                default:
                    output += prepareDebugInformationLine(
                            key, value.toString(), withGitHubLineBreaks,
                            value.typeName());
            }
        }
    }

    // add information about the system environment
    output += "\n## System environment\n\n";

    itr = QProcess::systemEnvironment();
    while (itr.hasNext()) {
        QStringList textList = itr.next().split("=");
        QString key = textList.first();
        textList.removeFirst();
        QString value = textList.join("=");
        output += prepareDebugInformationLine(key, value, withGitHubLineBreaks);
    }

    return output;
}
