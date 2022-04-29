export module CR.Engine.Audio.Utilities;

namespace CR::Engine::Audio {
	export float CalcVolume(float a_linearVol) {
		// https://www.dr-lex.be/info-stuff/volumecontrols.html
		// simple logorithmic volume approximation. 60db of range
		float result = a_linearVol * a_linearVol;
		return result * result;
	}
}    // namespace CR::Engine::Audio