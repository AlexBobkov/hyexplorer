#pragma once

namespace portal
{
    /**
    Описании версии геопортала
    */
    struct Version
    {
        static unsigned int majorVersion;
        static unsigned int minorVersion;
        static unsigned int patchVersion;

        static const char* str;
    };

}
