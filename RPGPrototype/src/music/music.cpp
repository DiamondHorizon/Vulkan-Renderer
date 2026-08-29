#include "music.hpp"

// Libs
#include <SDL/include/SDL_mixer.h>

// std
#include <string>

namespace music {
	Music::Music() {
		Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048);
	}

	Music::~Music() {}

	void Music::play(const char* song) {
		std::string path = "sounds/music/" + std::string(song) + ".wav";
		Mix_Music* bgm = Mix_LoadMUS(path.c_str());
		if (bgm) {
			Mix_PlayMusic(bgm, -1);  // -1 = loop forever
		}
	}
}