/**
* @file fps_cap.cpp
* @brief Throttle the cocos2d render loop to FPS_CAP without desync.
*
* Two hooks work together. Hooking Render alone causes ghosting (the XAML
* page still calls eglSwapBuffers every vblank, presenting stale back-buffer
* content) and input lag (CCEGLView::ProcessEvents only runs inside Render,
* so pointer events queue up across skipped frames). The combined hook fixes
* both:
*
*   1. ?Render@CCEGLView@cocos2d@@QAEXXZ  (libcocos2d_..._Windows_8.1.dll)
*      Replaced with a mimic that always drains the pointer-event queue but
*      gates the game tick (CCDirector::mainLoop, vtable slot 11) by QPC.
*      Sets g_didDraw=1 only when the tick actually ran.
*
*   2. eglSwapBuffers  (libEGL.dll)
*      Returns EGL_TRUE without swapping when g_didDraw is 0 — the back
*      buffer wasn't redrawn this vblank, so swapping it would just present
*      stale or partial content.
*
* Net effect: game logic + draw cadence = FPS_CAP, display update cadence =
* FPS_CAP (no ghost), input handling = display refresh (responsive clicks).
*
* The Render mimic mirrors the original prologue checks at object offsets
* +0x138 and +0x139 (initialization/ready flags). Those offsets are
* hardcoded for libcocos2d_v2.2.5_Windows_8.1.dll and would need re-derivation
* for any other cocos2d build.
*/
#include "pch.h"
#include "fps_cap.h"

#if defined(FPS_CAP) && FPS_CAP > 0

#include <stdlib.h>  // atoi
#include <detours/detours.h>

// FPS_CAP_DIAG is set by serverconfig.h (CMake option OFFLINEMOD_FPS_CAP_DIAG).
// When ON, the hook writes a heartbeat line to %TEMP%\offlinemod_fps.log every
// 600 Render calls and to OutputDebugString. Default OFF for production builds.
#ifndef FPS_CAP_DIAG
#define FPS_CAP_DIAG 0
#endif

namespace {

// __thiscall on MSVC x86: this in ECX, no other args. __fastcall on MSVC x86
// also puts arg0 in ECX (and arg1 in EDX). Declaring a hook __fastcall with
// a dummy EDX is the idiomatic way to intercept __thiscall under Detours.
typedef void  (__fastcall* Render_t)(void* self, void* edx);
typedef void  (__fastcall* ProcessEvents_t)(void* self, void* edx);
typedef void* (__cdecl*    sharedDirector_t)(void);
typedef void  (__fastcall* mainLoop_t)(void* self, void* edx);

// KHRONOS_APIENTRY on Windows = __stdcall.
typedef int   (__stdcall*  eglSwapBuffers_t)(void* dpy, void* surface);

Render_t          g_trueRender       = nullptr;
ProcessEvents_t   g_processEvents    = nullptr;
sharedDirector_t  g_sharedDirector   = nullptr;
eglSwapBuffers_t  g_trueSwapBuffers  = nullptr;

LARGE_INTEGER     g_qpcFrequency = {};
LONGLONG          g_lastDraw     = 0;
LONGLONG          g_minFrameTicks = 0;

// Set to 1 each time we actually let the tick + draw run. eglSwapBuffers
// reads-and-clears this to decide whether to swap or short-circuit. Volatile
// because both writers and readers run on the UI thread but the compiler
// shouldn't elide the load/store across our function-call boundaries.
volatile LONG     g_didDraw = 0;

// Subtract a small slop so a 60 Hz display reliably hits the 60 FPS cap
// instead of dropping to 30 due to vblank jitter against an exact 16.666 ms
// threshold.
const double kSlopSeconds = 0.0005;

const char* const kCocosDll     = "libcocos2d_v2.2.5_Windows_8.1.dll";
const char* const kEglDll       = "libEGL.dll";
const char* const kRenderExport = "?Render@CCEGLView@cocos2d@@QAEXXZ";
const char* const kProcessExport = "?ProcessEvents@CCEGLView@cocos2d@@AAEXXZ";
const char* const kSharedDirector = "?sharedDirector@CCDirector@cocos2d@@SAPAV12@XZ";
const char* const kEglSwap      = "eglSwapBuffers";

// Endpoint exposed by the offline server (gimuserver's OfflineModController).
// Returns the desired client FPS cap as a bare integer in the response body.
// The InternetConnect/HttpOpenRequest hooks in main.cpp redirect this to
// SERVICE_IP:SERVICE_PORT regardless of hostname, so the literal "127.0.0.1"
// + 9960 we pass here are nominal — the actual destination is the offline
// server the user configured.
const char* const kFpsHost      = "127.0.0.1";
const INTERNET_PORT kFpsPort    = 9960;
const char* const kFpsPath      = "/offline_mod/fps_cap";
const DWORD       kFpsTimeoutMs = 1000;

// One-shot resolver state. 0 = not yet attempted, 1 = in flight, 2 = done.
// Probed exactly once on the first valid Render call (rather than at attach
// time) because the offline server may still be starting up when DllMain runs.
volatile LONG g_fpsResolveState = 0;

// Forward decls.
int  FetchFpsCapFromServer();
void ApplyFpsCap(int cap);

// Offsets into the CCEGLView instance that the original Render checks before
// doing anything. Derived from the disassembly of the exported Render in
// libcocos2d_v2.2.5_Windows_8.1.dll (RVA 0x139240).
const size_t kReadyFlag1Offset = 0x138;
const size_t kReadyFlag2Offset = 0x139;

// CCDirector vtable slot for mainLoop. The original Render does
// `jmp [edx+0x2c]` after putting the director in ecx; 0x2c / sizeof(void*) = 11.
const size_t kMainLoopVtableSlot = 11;

#if FPS_CAP_DIAG
LONGLONG g_renderCalls = 0;
LONGLONG g_renderSkips = 0;
LONGLONG g_swapCalls   = 0;
LONGLONG g_swapSkips   = 0;
LONGLONG g_diagLastQpc = 0;

void DiagLogImpl(const char* fmt, ...)
{
    char msg[512];
    va_list ap;
    va_start(ap, fmt);
    int n = vsnprintf(msg, sizeof(msg) - 2, fmt, ap);
    va_end(ap);
    if (n < 0) return;
    if (n > (int)sizeof(msg) - 2) n = (int)sizeof(msg) - 2;
    msg[n] = '\n';
    msg[n + 1] = '\0';

    OutputDebugStringA(msg);

    char tempDir[MAX_PATH];
    DWORD tlen = GetTempPathA(MAX_PATH, tempDir);
    if (tlen == 0 || tlen > MAX_PATH) return;
    char path[MAX_PATH];
    if ((int)tlen + 20 >= MAX_PATH) return;
    lstrcpyA(path, tempDir);
    lstrcatA(path, "offlinemod_fps.log");

    HANDLE h = CreateFileA(path, FILE_APPEND_DATA, FILE_SHARE_READ | FILE_SHARE_WRITE,
                           nullptr, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (h == INVALID_HANDLE_VALUE) return;
    DWORD written = 0;
    WriteFile(h, msg, (DWORD)(n + 1), &written, nullptr);
    CloseHandle(h);
}
#define DiagLog(...) DiagLogImpl(__VA_ARGS__)
#else
// Variadic macro that compiles to nothing — neither the call nor the format
// strings survive into the binary.
#define DiagLog(...) ((void)0)
#endif

void __fastcall MyRender(void* self, void* edx)
{
    // Mirror the original Render's two ready-flag checks. If the view isn't
    // ready, the original would bail without touching the director — do the
    // same so we don't drive cocos2d before EGL initialization completes.
    unsigned char* obj = (unsigned char*)self;
    if (obj[kReadyFlag1Offset] == 0) return;
    if (obj[kReadyFlag2Offset] == 0) return;

    // Always drain queued pointer / window events. This keeps clicks
    // responsive even on vblanks where we skip the game tick.
    if (g_processEvents)
        g_processEvents(self, edx);

    // One-shot: probe the offline server for the configured FPS cap. We
    // do this on the first valid Render (rather than at attach time) so
    // the offline server has had time to come up. CompareExchange ensures
    // exactly one thread does the fetch even if Render fires reentrantly.
    if (g_fpsResolveState == 0
        && InterlockedCompareExchange(&g_fpsResolveState, 1, 0) == 0)
    {
        int fetched = FetchFpsCapFromServer();
        if (fetched >= 0)
            ApplyFpsCap(fetched);
        InterlockedExchange(&g_fpsResolveState, 2);
    }

    LARGE_INTEGER now;
    QueryPerformanceCounter(&now);

#if FPS_CAP_DIAG
    g_renderCalls++;
    if ((g_renderCalls % 600) == 0) {
        const double sec = g_qpcFrequency.QuadPart
            ? (double)(now.QuadPart - g_diagLastQpc) / (double)g_qpcFrequency.QuadPart
            : 0.0;
        DiagLog("[fps-cap] heartbeat render_calls=%lld render_skips=%lld swap_calls=%lld swap_skips=%lld interval=%.3fs call_hz=%.1f",
                g_renderCalls, g_renderSkips, g_swapCalls, g_swapSkips,
                sec, sec > 0 ? 600.0 / sec : 0.0);
        g_diagLastQpc = now.QuadPart;
    }
#endif

    if ((now.QuadPart - g_lastDraw) < g_minFrameTicks) {
#if FPS_CAP_DIAG
        g_renderSkips++;
#endif
        return; // gated: events drained, but no tick / draw / swap this vblank
    }

    g_lastDraw = now.QuadPart;

    // Tail-call CCDirector::mainLoop via vtable slot 11. We re-resolve the
    // director each call rather than caching at attach time because the
    // director instance may not yet exist when our DllMain runs.
    if (!g_sharedDirector) return;
    void* director = g_sharedDirector();
    if (!director) return;
    void** vtable = *(void***)director;
    mainLoop_t mainLoop = (mainLoop_t)vtable[kMainLoopVtableSlot];
    mainLoop(director, nullptr);

    // Mark that the back buffer was redrawn this vblank, so the upcoming
    // eglSwapBuffers is allowed through.
    InterlockedExchange(&g_didDraw, 1);
}

int __stdcall MySwapBuffers(void* dpy, void* surface)
{
#if FPS_CAP_DIAG
    g_swapCalls++;
#endif
    LONG didDraw = InterlockedExchange(&g_didDraw, 0);
    if (!didDraw) {
#if FPS_CAP_DIAG
        g_swapSkips++;
#endif
        return 1; // EGL_TRUE — pretend we swapped so the caller continues normally
    }
    return g_trueSwapBuffers(dpy, surface);
}

// Synchronous WinINet GET against the offline server. Returns the parsed
// integer in [0, 1000] on success, -1 on any failure (server down, timeout,
// unparseable body, etc.) so the caller can keep using the compile-time
// default. Bounded by kFpsTimeoutMs (1 sec) so a missing server only stalls
// the very first frame briefly.
int FetchFpsCapFromServer()
{
    HINTERNET hSession = InternetOpenA("OfflineMod-FpsCap",
                                       INTERNET_OPEN_TYPE_DIRECT,
                                       nullptr, nullptr, 0);
    if (!hSession) return -1;

    DWORD timeout = kFpsTimeoutMs;
    InternetSetOptionA(hSession, INTERNET_OPTION_CONNECT_TIMEOUT, &timeout, sizeof(timeout));
    InternetSetOptionA(hSession, INTERNET_OPTION_RECEIVE_TIMEOUT, &timeout, sizeof(timeout));
    InternetSetOptionA(hSession, INTERNET_OPTION_SEND_TIMEOUT,    &timeout, sizeof(timeout));

    int result = -1;
    HINTERNET hConn = InternetConnectA(hSession, kFpsHost, kFpsPort,
                                       nullptr, nullptr, INTERNET_SERVICE_HTTP, 0, 0);
    if (hConn) {
        HINTERNET hReq = HttpOpenRequestA(hConn, "GET", kFpsPath, nullptr,
                                          nullptr, nullptr,
                                          INTERNET_FLAG_NO_CACHE_WRITE
                                            | INTERNET_FLAG_NO_COOKIES
                                            | INTERNET_FLAG_RELOAD,
                                          0);
        if (hReq && HttpSendRequestA(hReq, nullptr, 0, nullptr, 0)) {
            char buf[16] = {0};
            DWORD bytesRead = 0;
            if (InternetReadFile(hReq, buf, (DWORD)sizeof(buf) - 1, &bytesRead)
                && bytesRead > 0)
            {
                buf[bytesRead] = 0;
                int parsed = atoi(buf);
                if (parsed >= 0 && parsed <= 1000)
                    result = parsed;
            }
        }
        if (hReq) InternetCloseHandle(hReq);
        InternetCloseHandle(hConn);
    }
    InternetCloseHandle(hSession);
    return result;
}

// Recomputes g_minFrameTicks for the new cap. cap==0 disables gating by
// setting the threshold to 0 (every Render passes; the swap hook also
// becomes a passthrough because g_didDraw is always set).
void ApplyFpsCap(int cap)
{
    if (cap <= 0) {
        g_minFrameTicks = 0;
        DiagLog("[fps-cap] server returned cap=0; disabling gate (every frame passes).");
        return;
    }
    const double frameSeconds = (1.0 / (double)cap) - kSlopSeconds;
    LONGLONG ticks = (LONGLONG)((double)g_qpcFrequency.QuadPart * frameSeconds);
    if (ticks < 1) ticks = 1;
    g_minFrameTicks = ticks;
    DiagLog("[fps-cap] server cap applied: cap=%dfps min_ticks=%lld", cap, ticks);
}

} // namespace

void FpsCap_Attach(void)
{
    HMODULE hCocos = GetModuleHandleA(kCocosDll);
    if (!hCocos) {
        DiagLog("[fps-cap] FAIL: GetModuleHandleA(\"%s\") returned NULL (err=%lu).",
                kCocosDll, GetLastError());
        return;
    }

    PVOID renderAddr  = (PVOID)GetProcAddress(hCocos, kRenderExport);
    PVOID processAddr = (PVOID)GetProcAddress(hCocos, kProcessExport);
    PVOID directorAddr = (PVOID)GetProcAddress(hCocos, kSharedDirector);
    if (!renderAddr || !processAddr || !directorAddr) {
        DiagLog("[fps-cap] FAIL: missing cocos2d export(s): render=%p process=%p sharedDirector=%p (err=%lu).",
                renderAddr, processAddr, directorAddr, GetLastError());
        return;
    }

    HMODULE hEgl = GetModuleHandleA(kEglDll);
    if (!hEgl) {
        DiagLog("[fps-cap] FAIL: GetModuleHandleA(\"%s\") returned NULL (err=%lu).",
                kEglDll, GetLastError());
        return;
    }
    PVOID swapAddr = (PVOID)GetProcAddress(hEgl, kEglSwap);
    if (!swapAddr) {
        DiagLog("[fps-cap] FAIL: GetProcAddress(\"%s\") returned NULL (err=%lu).",
                kEglSwap, GetLastError());
        return;
    }

    if (!QueryPerformanceFrequency(&g_qpcFrequency) || g_qpcFrequency.QuadPart == 0) {
        DiagLog("[fps-cap] FAIL: QueryPerformanceFrequency failed (err=%lu).", GetLastError());
        return;
    }

    // Seed g_minFrameTicks with the compile-time default so we have a sane
    // cap from the very first frame. The first MyRender call probes the
    // offline server (GET /offline_mod/fps_cap) and ApplyFpsCap overrides
    // this with the server-configured value. If the probe fails (server
    // down, unreachable) the compile-time default remains in effect.
    const double frameSeconds = (1.0 / (double)FPS_CAP) - kSlopSeconds;
    g_minFrameTicks = (LONGLONG)((double)g_qpcFrequency.QuadPart * frameSeconds);
    if (g_minFrameTicks < 1) g_minFrameTicks = 1;

    LARGE_INTEGER now;
    QueryPerformanceCounter(&now);
    // Bias the last-draw timestamp into the past so the first Render call
    // is not gated and the game gets to render at least one frame promptly.
    g_lastDraw = now.QuadPart - g_minFrameTicks;
#if FPS_CAP_DIAG
    g_diagLastQpc = now.QuadPart;
#endif

    g_processEvents   = (ProcessEvents_t)processAddr;
    g_sharedDirector  = (sharedDirector_t)directorAddr;
    g_trueRender      = (Render_t)renderAddr;
    g_trueSwapBuffers = (eglSwapBuffers_t)swapAddr;

    LONG err1 = DetourAttach(&(PVOID&)g_trueRender,      (PVOID)MyRender);
    LONG err2 = DetourAttach(&(PVOID&)g_trueSwapBuffers, (PVOID)MySwapBuffers);
    if (err1 != NO_ERROR || err2 != NO_ERROR) {
        DiagLog("[fps-cap] FAIL: DetourAttach render=%ld swap=%ld.", err1, err2);
        // Best-effort detach of any partial attachment.
        if (err1 == NO_ERROR) DetourDetach(&(PVOID&)g_trueRender,      (PVOID)MyRender);
        if (err2 == NO_ERROR) DetourDetach(&(PVOID&)g_trueSwapBuffers, (PVOID)MySwapBuffers);
        g_trueRender = nullptr;
        g_trueSwapBuffers = nullptr;
        return;
    }
    DiagLog("[fps-cap] OK: render hook at %p, swap hook at %p, compile_cap=%dfps slop=%.4fs min_ticks=%lld qpc_hz=%lld (server probe deferred to first Render)",
            renderAddr, swapAddr, (int)FPS_CAP,
            kSlopSeconds, g_minFrameTicks, g_qpcFrequency.QuadPart);
}

void FpsCap_Detach(void)
{
    if (g_trueRender) {
        DetourDetach(&(PVOID&)g_trueRender, (PVOID)MyRender);
        g_trueRender = nullptr;
    }
    if (g_trueSwapBuffers) {
        DetourDetach(&(PVOID&)g_trueSwapBuffers, (PVOID)MySwapBuffers);
        g_trueSwapBuffers = nullptr;
    }
#if FPS_CAP_DIAG
    DiagLog("[fps-cap] detached. render_calls=%lld render_skips=%lld swap_calls=%lld swap_skips=%lld",
            g_renderCalls, g_renderSkips, g_swapCalls, g_swapSkips);
#endif
}

#endif // FPS_CAP > 0
