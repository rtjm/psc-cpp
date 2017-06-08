#ifndef _playscenari_h 
#define _playscenari_h
class PlayScenari
{
	private:
		int _scenari, _tickint;
	protected:
	public:
		PlayScenari(int scenari, int tickint)
		{
			cout << "PlayScenari INIT: scenari " << scenari << " - tickint " << tickint << endl;
			_scenari = scenari;
			_tickint = tickint;
		}
		
		void Status()
		{
			cout << "PlayScenari STATUS: scenari " << _scenari << " - tickint " << _tickint << endl;
		}
};
#endif
