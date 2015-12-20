/*
UserTracks.h
Function to read and write to San Andreas User Tracks
*/

#include <string>
#include <vector>
#include <cerrno>

int UserTracksSave(std::vector<const char*> fileList);
int UserTracksSave(std::vector<std::string> fileList);
std::vector<std::string> UserTracksLoad();