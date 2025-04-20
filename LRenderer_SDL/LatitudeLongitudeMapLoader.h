#pragma once
class LatitudeLongitudeMap;

class LatitudeLongitudeMapLoader
{
public:
	LatitudeLongitudeMapLoader() = delete;
	static void SaveLatitudeLongitudeEXR(const char *path, const LatitudeLongitudeMap *latitudeLongitudeMap);
	static LatitudeLongitudeMap *LoadLatitudeLongitudeMapEXR(const char *path);
};