/**
* @file console.h
* @author Arves100
* @date 29/09/2026
* @brief Console API
*/
#pragma once

struct ConsoleAPI
{
    ConsoleAPI() : init(false) {}

    void Free(void)
    {
        if (init)
        {
            FreeConsole();
            init = false;
        }
    }

    bool Init(void)
    {
        if (AllocConsole())
        {
            FILE* dummy = nullptr;
            freopen_s(&dummy, "CONOUT$", "w", stdout);
            freopen_s(&dummy, "CONOUT$", "w", stderr);
            init = true;
        }

        return init;
    }

private:
    bool init;
};
