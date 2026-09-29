/**
* @file patch_fps.h
* @author Arves100
* @date 29/09/2026
* @brief FPS CAP patching API
*/
#pragma once

struct PatchFPSCAP
{
    bool Attach(void);
    void Detach(void);
};

