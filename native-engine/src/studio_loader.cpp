#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <cstdio>
#include <string>
#include <dlfcn.h>

// Permanent native SDL bootstrap. The real C++ game lives in libeclipse_game.so.
// User-selected revisions live in app-private files and are loaded only on a NEW
// game process. No executable downloaded from the Internet is ever run.
namespace {
std::string storage() {
    const char* dir = SDL_AndroidGetInternalStoragePath();
    return dir ? std::string(dir) : std::string();
}
void status(const std::string& message) {
    const auto dir=storage();
    if (dir.empty()) return;
    const auto path=dir+"/studio_last_launch.txt";
    if (FILE* f=std::fopen(path.c_str(),"wb")) {
        std::fwrite(message.data(),1,message.size(),f);
        std::fclose(f);
    }
    SDL_Log("ECLIPSE Studio: %s",message.c_str());
}
using GameMain=int(*)(int,char**);
}
int main(int argc,char** argv) {
    const std::string dir=storage();
    if(dir.empty()){ SDL_Log("ECLIPSE Studio: missing Android storage path");return 91; }
    const std::string imported=dir+"/game_updates/libeclipse_game.so";
    const std::string packaged="libeclipse_game.so";
    const bool hasImported=[&](){FILE* f=std::fopen(imported.c_str(),"rb");if(!f)return false;std::fclose(f);return true;}();
    const std::string source=hasImported?imported:packaged;
    status("STARTED\n"+source);
    // Packaged library uses the Android linker search path. Imported library
    // must be an Android-compatible arm64 shared object with SDL dependencies.
    void* handle=dlopen(source.c_str(),RTLD_NOW|RTLD_LOCAL);
    if(!handle){status("ERROR: dlopen "+source+"\n"+(dlerror()?dlerror():"unknown linker failure"));return 92;}
    dlerror();
    auto game=reinterpret_cast<GameMain>(dlsym(handle,"SDL_main"));
    const char* err=dlerror();
    if(err||!game){status(std::string("ERROR: SDL_main export missing: ")+(err?err:"unknown"));dlclose(handle);return 93;}
    status("RUNNING\n"+source);
    const int code=game(argc,argv);
    status("EXIT "+std::to_string(code)+"\n"+source);
    dlclose(handle);
    return code;
}
