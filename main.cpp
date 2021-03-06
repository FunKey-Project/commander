#include <cstdlib>
#include <cstring>
#include <iostream>
#include <unistd.h>

#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>

#include "error_dialog.h"
#include "commander.h"
#include "def.h"
#include "resourceManager.h"
#include "screen.h"
#include "sdlutils.h"
#include "fk_menu.h"
#include "fk_instant_play.h"

// Globals
const SDL_Color Globals::g_colorTextNormal = {COLOR_TEXT_NORMAL};
const SDL_Color Globals::g_colorTextTitle = {COLOR_TEXT_TITLE};
const SDL_Color Globals::g_colorTextDir = {COLOR_TEXT_DIR};
const SDL_Color Globals::g_colorTextSelected = {COLOR_TEXT_SELECTED};
std::vector<CWindow *> Globals::g_windows;

namespace {

SDL_Surface *SetVideoMode(int width, int height, int bpp, std::uint32_t flags) {
	fprintf(stderr, "Setting video mode %dx%d bpp=%u flags=0x%08X\n", width, height, bpp, flags);
    fflush(stderr);
	auto *result = SDL_SetVideoMode(width, height, bpp, flags);
	const auto &current = *SDL_GetVideoInfo();
	fprintf(stderr, "Video mode is now %dx%d bpp=%u flags=0x%08X\n",
	    current.current_w, current.current_h, current.vfmt->BitsPerPixel, SDL_GetVideoSurface()->flags);
	fflush(stderr);
    return result;
}

} // namespace

int main(int argc, char** argv)
{
    // Avoid crash due to the absence of mouse
    char l_s[]="SDL_NOMOUSE=1";
    putenv(l_s);

    // Init SDL
    SDL_Init(SDL_INIT_VIDEO);
    if (IMG_Init(IMG_INIT_JPG | IMG_INIT_PNG | IMG_INIT_TIF | IMG_INIT_WEBP) == 0) {
        std::cerr << "IMG_Init failed" << std::endl;
    } else {
        // Clear the errors for image libraries that did not initialize.
        SDL_ClearError();
    }

    // Hide cursor before creating the output surface.
    SDL_ShowCursor(SDL_DISABLE);

    if (screen.init() != 0) return 1;

    // Init font
    if (TTF_Init() == -1)
    {
        std::cerr << "TTF_Init failed: " << SDL_GetError() << std::endl;
        return 1;
    }

    // Init menu
    FK_InitMenu();

    // Init Instant Play
    FK_InitInstantPlay();

    // Create instances
    CResourceManager::instance();

    std::string l_path = PATH_DEFAULT;
    std::string r_path = PATH_DEFAULT_RIGHT;
    if (access(l_path.c_str(), F_OK) != 0) l_path = "/";
#ifdef PATH_DEFAULT_RIGHT_FALLBACK
    if (l_path == r_path || access(r_path.c_str(), F_OK) != 0) r_path = PATH_DEFAULT_RIGHT_FALLBACK;
#endif
    if (access(r_path.c_str(), F_OK) != 0) r_path = "/";

    CCommander l_commander(l_path, r_path);

    // exec error from the previous process (when relaunching)
    if (argc >= 3 && std::strcmp(argv[1], "--show_exec_error") == 0)
        ErrorDialog("Exec error", argv[2]);

    // Main loop
    l_commander.execute();

    //Quit
    SDL_utils::hastalavista();

    return 0;
}
