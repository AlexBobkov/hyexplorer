#pragma once

#include <QString>

namespace portal
{
    QString genetrateRandomName(std::size_t size = 10);

    void openExplorer(const QString& path);
}