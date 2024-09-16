//#ifndef APOLLO_GUI_H
//#define APOLLO_GUI_H
//
//#include <string>
//#include <utility>
//#include <vector>
//
//#include "general.h"
//
//#include "SDL3_ttf/SDL_ttf.h"
//using namespace std;
//
//typedef struct Timeline {
//
//} Timeline;
//
//typedef struct Panel {
//
//} Panel;
//
//typedef struct Toolbar {
//    SDL_Renderer* renderer;
//    vector<char*> options = {"File", "Edit", "Frame", "Help"};
//    TTF_Font* font;
//    SDL_Color fontColor;
//    SDL_Color toolbarColor;
//    vector<SDL_Texture*> textures;
//    int padding;
//
//    Toolbar(SDL_Renderer* renderer, vector<char*> options, TTF_Font* font, SDL_Color fontColor, SDL_Color toolbarColor, int padding) {
//        this->renderer = renderer;
//        this->options = std::move(options);
//        this->font = font;
//        this->fontColor = fontColor;
//        this->toolbarColor = toolbarColor;
//        this->padding = padding;
//
//        this->createTextures();
//    }
//
//    void createTextures() {
//        for (auto& option : options) {
//            textures.push_back(SDL_CreateTextureFromSurface(renderer, TTF_RenderText_Solid(this->font, option, fontColor)));
//        }
//    }
//
//    // given height so in case window screen is narrow, we can send the rest of the toolbar elements to another row
//    int render(Vector2 currentWindowSize) {
//        int totalLength = 0;
//    }
//
//    void free() {
//        for (auto & option : options) {
//            SDL_free(option);
//        }
//        for (auto& texture : textures) {
//            SDL_DestroyTexture(texture);
//        }
//    }
//
//
//} Toolbar;
//
//// right click actions on panels, default behavior on toolbar
//typedef struct Dropdown {
//
//} Dropdown;
//
//#endif //APOLLO_GUI_H
