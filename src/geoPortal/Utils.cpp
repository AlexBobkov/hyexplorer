/* HyExplorer - hyperspectral images management system
* Copyright (c) 2015-2016 HyExplorer team
* http://virtualglobe.ru/hyexplorer/
*
* Permission is hereby granted, free of charge, to any person obtaining
* a copy of this software and associated documentation files (the
* "Software"), to deal in the Software without restriction, including
* without limitation the rights to use, copy, modify, merge, publish,
* distribute, sublicense, and/or sell copies of the Software, and to
* permit persons to whom the Software is furnished to do so, subject to
* the following conditions:
*
* The above copyright notice and this permission notice shall be
* included in all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
* NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
* LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
* OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
* WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include "Utils.hpp"

#include <QFileInfo>
#include <QUrl>
#include <QDir>
#include <QDesktopServices>
#include <QProcess>
#include <QDebug>

using namespace portal;

namespace
{
    std::string alphabet = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890";
}

QString portal::genetrateRandomName(std::size_t size)
{
    QString name;    
    name.resize(size);

    for (int i = 0; i < name.size(); i++)
    {
        name[i] = alphabet[rand() % alphabet.size()];
    }

    return name;
}

void portal::openExplorer(const QString& path)
{
    if (QFileInfo(path).isDir())
    {
        QDesktopServices::openUrl(QUrl(QString("file:///") + path));
    }
    else
    {
        QProcess::startDetached(QString("explorer.exe /select,\"%0\"").arg(QDir::toNativeSeparators(path)));
    }
}