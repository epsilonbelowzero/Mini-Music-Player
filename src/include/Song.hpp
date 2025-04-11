#pragma once

#include <queue>

#include <cstdio>

#include "Loader.hpp"

//Provides interface to print current song info and to check whether
//a new song is playing

class Song {
	private:
		Loader* load;
		int actSong;
		std::queue<int> play;
		
		void push(int i) {
			play.push(i);
		}
	public:
		
		Song(Loader& load_): load(&load_), actSong(0) {}
		
		void push() { push(load->actSong()); }
		bool change() { return actSong != play.front(); }
		
		Song& updateUser() {
			actSong = play.front();
			load->printBanner(actSong);
			
			return *this;
		}
		void nextBuffer() { play.pop(); }
		
		void updateSongInfo() {
			if(!play.size()) return;
			if( change() ) {
				updateUser();
			}
			nextBuffer();
		}
		
		void debugInfo() {
			printf("\t{% 2li/% 2i/% 2i}", play.size(), play.size() ? play.front(): -1, actSong);
		}
};
