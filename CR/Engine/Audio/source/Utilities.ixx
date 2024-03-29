export module CR.Engine.Audio.Utilities;

import <cmath>;

namespace CR::Engine::Audio {
	export float CalcLogVolume(float a_linearVol) {
		// https://www.dr-lex.be/info-stuff/volumecontrols.html
		// simple logorithmic volume approximation. 60db of range
		float result = a_linearVol * a_linearVol;
		return result * result;
	}
	export float CalcLinearVolume(float a_logVol) {
		return pow(a_logVol, 0.25f);
	}
}    // namespace CR::Engine::Audio