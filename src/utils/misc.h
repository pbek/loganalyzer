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

#pragma once

#include <QString>

/*  Miscellaneous functions that can be useful */

namespace Utils
{
    namespace Misc {
        // use binary prefix standards from IEC 60027-2
        // see http://en.wikipedia.org/wiki/Kilobyte
        enum class SizeUnit
        {
            Byte,       // 1024^0,
            KibiByte,   // 1024^1,
            MebiByte,   // 1024^2,
            GibiByte,   // 1024^3,
            TebiByte,   // 1024^4,
            PebiByte,   // 1024^5,
            ExbiByte    // 1024^6,
            // int64 is used for sizes and thus the next units can not be handled
            // ZebiByte,   // 1024^7,
            // YobiByte,   // 1024^8
        };

        void openPath(const QString& absolutePath);
        void openFolderSelect(const QString& absolutePath);
        QString removeIfStartsWith(QString text, QString removeString);
        QString removeIfEndsWith(QString text, QString removeString);
        QString prependIfDoesNotStartWith(QString text, QString startString);
        QString appendIfDoesNotEndWith(QString text, QString endString);

        QString unitString(SizeUnit unit);
        bool friendlyUnit(qint64 sizeInBytes, qreal& val, SizeUnit& unit);
        QString friendlyUnit(qint64 bytesValue, bool isSpeed = false);
        QString fromDouble(double n, int precision);
        QByteArray gUncompress(const QByteArray &data);
    }
}
