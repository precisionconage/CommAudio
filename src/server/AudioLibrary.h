#ifndef AUDIOLIBRARY_H
#define AUDIOLIBRARY_H

#include "audioManagement.h"
#include "SongData.h"

class AudioLibrary
{
public:
	/**
	 * The maximum number of songs held in the library at a time.
	 */
	const static int MAX_SONGS = 10000;

	std::vector<SongData>		songList;
	std::vector<std::string>	supportedTypes;
	int							numsongs;
	int							maxSongs;
	std::string					directory;
	int							playingSong;

	AudioLibrary(std::string directory, std::string supportedTypes, int numTypes, int maxSongs);
	AudioLibrary(std::string directory, std::string supportedTypes, int numTypes);
	std::list<std::shared_ptr<WSABUF>> grabPlaylist();
	SongData grabRandom();
	void resetSongsSent();

private:
	int			sentSongs;

	int			grabSongs(const char * albumDirectory, std::string artDirectory, struct dirent * songEnt);
	bool		checkType(std::string type);
	bool		checkName(const char * name);
	void		retrieveDirectory();
	std::string	getFileType(std::string file);
	std::string	getCoverArt(std::string songDirectory);
};

#endif