export module CR.Engine.Audio.Utilities;

import std;

export namespace CR::Engine::Audio {
	inline float CalcLogVolume(float a_linearVol) {
		// https://www.dr-lex.be/info-stuff/volumecontrols.html
		// simple logarithmic volume approximation. 60db of range
		float result = a_linearVol * a_linearVol;
		return result * result;
	}
	inline float CalcLinearVolume(float a_logVol) {
		return pow(a_logVol, 0.25f);
	}
}    // namespace CR::Engine::Audio