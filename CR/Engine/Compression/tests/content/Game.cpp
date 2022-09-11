#include "Game.h"
#include "../Engines/Graphics/HPTGraphics.h"
#include "../Engines/Input/input_engine.h"
#include "SaveGameManager.h"
#include "Genemies.h"
#include "MessageBroadcaster.h"
//#include <mach/mach.h>
//#include <mach/mach_time.h>
#include "../Engines/Utility/FunctionObjects.h"
#include "FilePaths.h"
#include <sstream>
#include "s3eTimer.h"
#include "Sound.h"
#include "AssetListNew.h"
#include "s3e.h"
#include "TutorialManager.h"
#include "MusicManager.h"
#include "AppConfig.h"
//#include "s3eIOSIAd.h"

using namespace std;
using namespace CR;
using namespace CR::Utility;
using namespace CR::Database;
using namespace CR::Sound;

//using namespace Syntax::Utility;

const float c_healthDropRate = 1;

HPTGraphicsEngine *graphics_engine = 0;
Input_Engine   *input_engine = 0;
//CRSoundPlayer *soundPlayer = 0;
//CRMusicPlayer *musicPlayer = 0;

HPTSprite1* pixel_spr;
HPTSprite1* pixel_red_spr;
HPTSprite1* pixel_yellow_spr;
HPTSprite1* pixel_black_spr;
HPTSprite1* pixel_green_spr;
HPTSprite1* pixel_blue_spr;

HPTFont1* fnt_green;
HPTFont1* fnt_red;
HPTFont1* fnt_yellow;
HPTFont1* fnt_blue;
HPTFont1* gothic10;

extern char path[256];

//const int number_basic_items[15] = {20,11,0,13,22,0,12,20,0,13,16,0,8,18,0};
//const int number_treasure_items[15] = {5,5,0,5,5,0,5,5,0,5,5,0,5,5,0};
const int number_special_items[9] = {3,3,0,4,4,0,4,2,0};

int difficulty_level = 0;

char *hptstrcat(char *x,char *y)
{
	static char *temp_string = 0;
	if(temp_string != 0) delete[] temp_string;
	temp_string = new char[300];
	strcpy(temp_string,x);
	strcat(temp_string,y);
	return temp_string;
}

Game::Game() : ActiveGState(game_start), m_overlay(NULL), m_healthTick(0)
{
	graphics_engine = 0;

/*const int number_basic_items[15] = {20,11,0,13,22,0,12,20,0,13,16,0,8,18,0};
const int number_treasure_items[15] = {5,5,0,5,5,0,5,5,0,5,5,0,5,5,0};
const int number_special_items[15] = {3,3,0,4,4,0,4,3,0,4,3,0,4,2,0};
*/
	difficulty_level = 0;

//	srand(GetTickCount());
	screen_size = NORMAL_SIZE;
	disable_input = false;
	//timerfreq = 1.0f/SysTicksPerSecond();
	
	//mach_timebase_info_data_t time_info;
	//mach_timebase_info(&time_info);
	//timerfreq = time_info.numer/(float)time_info.denom;
	timerfreq = 1/1000.0f;
	
	m_levelEndFSM.AddStateHandler(0,StateHandler(this,(&Game::LevelEndState1)));
	m_levelEndFSM.AddStateHandler(1,StateHandler(this,(&Game::LevelEndState2)));
	m_levelEndFSM.AddStateHandler(2,StateHandler(this,(&Game::LevelEndState3)));
	m_levelEndFSM.AddStateHandler(3,StateHandler(this,(&Game::LevelEndState4)));
	m_levelEndFSM.AddStateHandler(4,StateHandler(this,(&Game::LevelEndState5)));
	
	m_playerDeathFSM.AddStateHandler(0,StateHandler(this,(&Game::PlayerDeathState1)));
	m_playerDeathFSM.AddStateHandler(1,StateHandler(this,(&Game::PlayerDeathState2)));
	m_playerDeathFSM.AddStateHandler(2,StateHandler(this,(&Game::PlayerDeathState3)));
	m_playerDeathFSM.AddStateHandler(3,StateHandler(this,(&Game::PlayerDeathState4)));
	
	int level = 1;
#ifdef BOB_LITE
	for(int i = 1;i <= 1;i++)
#else
	for(int i = 1;i <= 15;i++)
#endif
	{
		if(i == 7)
			i = 13;
		stringstream file;
		file << "ai" << i << ".hlf";
		//NSString * path1 = [[NSBundle mainBundle] pathForResource: [[NSString alloc] initWithCString: file.str().c_str()] ofType: @"hlf"];
		//const char *levelfile1 = [path1 cStringUsingEncoding:1];
		FilePaths::Instance().SetAILevelPath(level,file.str().c_str());
		level++;
	}
	
	/*
	levelAchievmentTimes.push_back(make_pair(2,0));
	levelAchievmentTimes.push_back(make_pair(2,0));
	levelAchievmentTimes.push_back(make_pair(2,0));
	levelAchievmentTimes.push_back(make_pair(2,0));
	levelAchievmentTimes.push_back(make_pair(2,0));
	levelAchievmentTimes.push_back(make_pair(2,0));
	levelAchievmentTimes.push_back(make_pair(2,0));
	levelAchievmentTimes.push_back(make_pair(2,0));
	levelAchievmentTimes.push_back(make_pair(2,0));
	 */
	
	levelAchievementTimes.push_back(60);
	levelAchievementTimes.push_back(90);
	levelAchievementTimes.push_back(20);
	levelAchievementTimes.push_back(90);
	levelAchievementTimes.push_back(90);
	levelAchievementTimes.push_back(45);
	levelAchievementTimes.push_back(200);
	levelAchievementTimes.push_back(140);
	levelAchievementTimes.push_back(75);

	if(AppConfig::Instance().PlatformType() == AppConfigSingleton::IPAD)
		ISound::Instance().SetProcessingFrequency(32000);
	else
		ISound::Instance().SetProcessingFrequency(16000);

	m_database = DatabaseFactory::Instance().CreateDatabase(L"data.edf");
	ISound::Instance().SetDatabase(m_database);

	m_soundBang = ISound::Instance().CreateSoundFX(CR::AssetList::sounds::bang::ID);

	/*if(AppConfig::Instance().IsIAd())
	{
		s3eIAdStart();
		s3eIAdSetInt(S3E_IAD_BANNER_SHOW,0);
		m_bannerShow = 0;
		m_bannerTime = 0;
	}*/
}

Game::~Game()
{
	/*if(AppConfig::Instance().IsIAd())
	{
		s3eIAdStop();
	}*/
	TutorialManager::Instance().Cleanup();
	m_database->Release();
	pixel_spr->Release();
	pixel_red_spr->Release();
	pixel_yellow_spr->Release();
	pixel_black_spr->Release();
	pixel_green_spr->Release();
	pixel_blue_spr->Release();
	delete analogStick;
	delete jumpButton;
	delete attackButton;
	delete specialAttackButton;
	delete grenadeButton;
	delete pauseButton;
	delete weaponSelect;
	delete resumeGameButton;
	delete quitGameButton;
	delete loadScreenButton;
	delete healthMeterBar;
	delete energyMeterBar;
	delete timeDisplay;
	delete deathScreenButton;
	
	m_ProgressMeter = new ProgressMeter("Closing Blade of Betrayal\0", 20);
	m_ProgressMeter->ProgressMeterStart();
	saved_games.SaveToDisk(DEFAULT_SAVE_FILE);

	graphics_engine->SetBackgroundColor(0,0,0);
	graphics_engine->SetBackgroundImage(0);
	graphics_engine->SetTileBackgroundImage(0,0);
	graphics_engine->SetTileBackgroundImage(1,0);

	m_ProgressMeter->Update();
	
	m_ProgressMeter->Update();

	delete menu;
	delete cut_scenes;

	m_ProgressMeter->Update();

	ai_engine->Release();
	delete level_class;
	delete player;

	m_ProgressMeter->Update();

	level_end->Release();
	m_ProgressMeter->Finish();
	m_ProgressMeter->ProgressMeterStop();
	delete m_ProgressMeter;
	graphics_engine->Release();
	
	tallyNumEnemies = 0;
	tallyNumDeadEnemies = 0;
}

void Game::ExecuteGame()
{	
	//graphics_engine->DumpUsage();
	currenttime = s3eTimerGetMs();
	time = (currenttime - starttime)*timerfreq;
	time = min(time, 1.0f/10.0f);
	starttime = currenttime;
	frame_rate_count += time;
	nframes++;
	if(nframes >= 100)
	{
		
		m_gameTime = 1000.0f*(m_gameTimer.GetTotalTime()/nframes);
		m_renderTime = 1000.0f*(m_renderTimer.GetTotalTime()/nframes);
		m_gameTimer.Reset();
		m_renderTimer.Reset();
		if(frame_rate_count == 0) frame_rate_count = 0.00001f;
		frame_rate = 100.0f / frame_rate_count;
		nframes = 0;
		frame_rate_count = 0;
	}

	if(time < 0) time = 0;
	
	if(AppConfig::Instance().IsIAd())
	{
		/*m_bannerTime += time;
		if(m_bannerTime > 60)
		{
			m_bannerShow = (m_bannerShow+1)%2;
			m_bannerTime = 0;
			s3eIAdSetInt(S3E_IAD_BANNER_SHOW,m_bannerShow);
		}*/
		/*if(s3eIAdGetInt(S3E_IAD_BANNER_LOADED) == 1)
		{
			s3eIAdSetInt(S3E_IAD_BANNER_SHOW, 1);
		}
		else	
		{
			s3eIAdSetInt(S3E_IAD_BANNER_SHOW, 0);
		}
		if(s3eIAdGetInt(S3E_IAD_BANNER_FULLSCREEN_AD_IN_PROGRESS ) == 1)
		{
			s3eDeviceYield(15); 
			graphics_engine->BeginFrame();
			graphics_engine->EndFrame();
			return;
		}*/
	}

	//Process New or Current Game State
	switch(ActiveGState)
	{
	case game_end:
		ProcessEnd();
		break;
	case game_movie:
		ProcessMovie();
		break;
	case game_credits:
		ProcessCredits();
		break;
	case game_menu:
		ProcessMenu();
		break;
	case game_ingame:
		ProcessGame();
		break;
	case game_paused:
		ProcessPause();
		break;
	case game_start:
		ProcessStart();
		break;
	case game_death:
		ProcessDeath();
		break;
	case game_level_load:
		ProcessLevelLoad();
		break;
	case game_level_end:
		ProcessEndLevel();
		break;
	case game_tutorial:
		ProcessTutorial();
		break;
	case game_noiad:
		ProcessNoIAD();
		break;
	default:
		break;
	}
}

void Game::ProcessGame()
{
	weaponSelect->Disabled(false);
	m_gameTimer.StartFrame();
	m_playTime += time;
	IPlayerFlags *ipFlags;
	if(!transition->sActive)
		ai_engine->Start();
	int tempi = enemys.size();
	int x = (int)player->GetXLoc();
	int y = (int)player->GetYLoc();
	int count;
	
	if(current_level == 10)
	{
		m_healthTick += time;
		if(m_healthTick >= c_healthDropRate)
		{
			m_healthTick = 0;
			player->doDamage(1,false,0,true);
		}
	}

	bool boss_level = false;
	if(((current_level % 3) == 0)) boss_level = true;
	s3eDeviceYield(0);
	activeEnemys.clear();
	for(count = 0;count < tempi;count++)
	{
		ipFlags = enemys[count]->GetStateFlags();
		if(ipFlags->S_DEAD)
		{
			//cout << "dead dead dead" << endl;
			ipFlags->S_ACTIVE = false;

			if(ipFlags->S_POINTS)
			{
				//Spawn Point Projectile
				//Projectile *temp;
				//temp = new ProjectilePoints((int)enemys[count]->GetXLoc(),(int) enemys[count]->GetYLoc(), enemys[count]->GetPointValue(), 0);
				
				//projectile_list.push_back(temp);

				//Add Points to Player
				//player->AddScore(enemys[count]->GetPointValue());//enemys[count]->GetPointValue());
				ipFlags->S_POINTS = false;
			}
		}
		else
		{
			int xe = (int)enemys[count]->GetXLoc();
			int ye = (int)enemys[count]->GetYLoc();
			int xex = xe - x;
			int yey = ye - y;
			if(boss_level || (((xex*xex) + (yey*yey)) < 90000+enemys[count]->m_rangeExtra))
			{
				ipFlags->S_ACTIVE = true;
				activeEnemys.push_back(enemys[count]);
			}
			else
			{
				//cout << "in active " << xe << " " << ye << endl;
				ipFlags->S_ACTIVE = false;
			}

		}

		if(ipFlags->S_ACTIVE)
		{
			enemys[count]->setTime(time);
			//if(!transition->sActive)
				enemys[count]->Update();
			level_class->CollideWithLevelE(enemys[count]);
		}
	}
	s3eDeviceYield(0);
	player->GetStateFlags()->S_ACTIVE = true;
/*	tempi = enemys.size();
	for(count = 0;count < tempi;count++)
	{
		//ipFlags = enemys[count]->GetStateFlags();

		if(enemys[count]->GetStateFlags()->S_ACTIVE)
		{
			enemys[count]->setTime(time);
			enemys[count]->Update();
			level_class->CollideWithLevel(enemys[count]);
		}
	}*/

	list<Projectile*>::iterator temppl;
	temppl = projectile_list.begin();
	
	int countmax = projectile_list.size();
	for(count = 0; count < countmax;count++)
//	while(temppl != projectile_list.end())
	{
		(*temppl)->SetTime(time);
		(*temppl)->Update();
		++temppl;
	}
	s3eDeviceYield(0);
	level_class->SetTime(time);
	level_class->CollideWithLevel(player);
	if(new_level)
	{
		new_level = false;
	
		return;
	}
	level_class->WeaponCollideCheck(player);
	level_class->Update();

	float xt,yt;
	//MessageBox(0,("done with loop"),("error"),MB_OK);
	player->UpdateLevel();
	player->getViewTransform(&xt,&yt);
	tempi = activeEnemys.size();
	for(count = 0;count < tempi;count++)
	{
		activeEnemys[count]->setViewTransform((xt),(yt));
	}
	s3eDeviceYield(0);
	
	
	list<Projectile*>::iterator temppl2 = projectile_list.begin();
	countmax = projectile_list.size();
	for(count = 0; count < countmax;count++)
//	while(temppl != projectile_list.end())
	{
		(*temppl2)->setViewTransform(xt,yt);
		++temppl2;
	}
	s3eDeviceYield(0);

//process attacks
	if((boss_level) && ((player->GetCurLife() == 0) || (enemys[0]->GetCurLife() == 0)))
	{

	}
	else
	{
//player to enemy damage
	HPTRect& tweaponrect = player->GetWeaponWorldLoc();
	int damage = player->GetWeaponDamage();
	//IPlayerFlags *tflags = player->GetStateFlags();
	if(player->GetStateFlags()->S_ATTACK)
	{
		int tempi2 = activeEnemys.size();
		for(count = 0;count < tempi2;count++)
		{
			ipFlags = activeEnemys[count]->GetStateFlags();

			if(!ipFlags->S_DEAD)
			{
				HPTRect& teweaponrect = activeEnemys[count]->GetWorldLoc();
				
				bool collide = true;
				bool tempb;
				int tempi;
	
				if((tweaponrect.p1.x == tweaponrect.p2.x) && (tweaponrect.p1.y == tweaponrect.p2.y))
					collide = false;
				else
				{
					if((tweaponrect.p1.x > teweaponrect.p2.x)) collide = false;
					if((tweaponrect.p2.x < teweaponrect.p1.x)) collide = false;
					if((tweaponrect.p1.y > teweaponrect.p2.y)) collide = false;
					if((tweaponrect.p2.y < teweaponrect.p1.y)) collide = false;
					tempi = (int)player->GetXLoc() - (int)activeEnemys[count]->GetXLoc();
					if(tempi < 0) tempb = true;
					else tempb = false;
				}

				if(collide)
				{
					if((ipFlags->S_ACTIVE) && (!ipFlags->S_DAMAGED) && (!ipFlags->S_INVULNERABLE) && (!ipFlags->S_DEAD))
					{
						player_hits++;
						
						/* // No upgrades to knife attack for iPhone version 
						switch(player->current_weapon)
						{
						case 1:
							player->AddEnergy(-1);
							damage = 0;
							break;
						case 2:
							damage = damage>>1;
							player->AddEnergy(damage);
							break;
						case 3:
							damage = damage>>1;
							
							break;
						case 4:
							damage = damage>>1;
							player->AddLife(damage);
							break;
						case 5:
							damage = damage*3;
							break;
						default:
							break;
						}*/
						activeEnemys[count]->doDamage(damage,tempb,player->ipFlags.nCurrAttack);
						//enemys[count]->doDoor(player->current_weapon);
						//enemys[count]->doDamage(damage,tempb,0);
					}
				}
			}
		}
	}
	}
	s3eDeviceYield(0);
//enemy to player damage

	if((boss_level) && ((player->GetCurLife() == 0) || (enemys[0]->GetCurLife() == 0)))
	{

	}
	else
	{

	int tempi2 = activeEnemys.size();
	for(count = 0;count < tempi2;count++)
	{
		HPTRect& tweaponrect = activeEnemys[count]->GetWeaponWorldLoc();
		int damage = activeEnemys[count]->GetWeaponDamage();
		//IPlayerFlags *tflags = enemys[count]->GetStateFlags();
		
		/*float xdistance = player->GetXLoc()-activeEnemys[count]->GetXLoc();
		float ydistance = player->GetYLoc()-activeEnemys[count]->GetYLoc();
		float distance = xdistance*xdistance+ydistance*ydistance;
		if(distance < (5*16*5*16))
		{
			TutorialManager::Instance().Trigger(TutorialManager::TATTACK);
		}*/

		ipFlags = activeEnemys[count]->GetStateFlags();

		//if(!ipFlags->S_DEAD)
		//{
		if((ipFlags->S_ATTACK || ipFlags->S_ALWAYS_ATTACK) && !ipFlags->S_DEAD && ipFlags->S_VISIBLE)
		{
			HPTRect& teweaponrect = player->GetWorldLoc();
		
			bool collide = true;
			bool tempb;
			int tempi;

			if((tweaponrect.p1.x == tweaponrect.p2.x) && (tweaponrect.p1.y == tweaponrect.p2.y))
				collide = false;
			else
			{
				if((tweaponrect.p1.x > teweaponrect.p2.x)) collide = false;
				if((tweaponrect.p2.x < teweaponrect.p1.x)) collide = false;
				if((tweaponrect.p1.y > teweaponrect.p2.y)) collide = false;
				if((tweaponrect.p2.y < teweaponrect.p1.y)) collide = false;
				tempi = (int)activeEnemys[count]->GetXLoc() - (int)player->GetXLoc();
				if(tempi < 0) tempb = true;
				else tempb = false;
			}

			if(collide)
			{
				ipFlags = player->GetStateFlags();

				if((!ipFlags->S_DAMAGED) && (!ipFlags->S_INVULNERABLE) && (!ipFlags->S_DEAD))
				{
					enemy_hits++;
					switch(player->current_weapon)
					{
					case 1:
						break;
					case 2:
						break;
					case 3:
						damage = damage>>1;
						player->AddEnergy(-damage);

						break;
					case 4:
						break;
					case 5:
						break;
					default:
						break;
					}
					player->doDamage(damage,tempb,0);
				}
				
			}
		}
		//}
	}
	}
	s3eDeviceYield(0);
//projectile damage
	if((boss_level) && ((player->GetCurLife() == 0) || (enemys[0]->GetCurLife() == 0)))
	{

	}
	else
	{

	list<Projectile*>::iterator temppl3 = projectile_list.begin();
	countmax = projectile_list.size();
	for(count = 0; count < countmax;count++)
//	while(temppl != projectile_list.end())
	{
		HPTRect& tweaponrect = (*temppl3)->GetWeaponBounds();
		int damage = (*temppl3)->GetWeaponDamage();


//		if(!ipFlags->S_DEAD)
//		{
		if((*temppl3)->GetSource())
		{
			int tempi2 = activeEnemys.size();
			for(int count = 0;count < tempi2;count++)
			{
				ipFlags = activeEnemys[count]->GetStateFlags();
				if(ipFlags->S_DEAD) continue;
				HPTRect& teweaponrect = activeEnemys[count]->GetWorldLoc();
						
				bool collide = true;
//				bool tempb;
//				int tempi;
		
				if((tweaponrect.p1.x == tweaponrect.p2.x) && (tweaponrect.p1.y == tweaponrect.p2.y))
					collide = false;
				else
				{
					collide = true;
					if((tweaponrect.p1.x > teweaponrect.p2.x)) collide = false;
					if((tweaponrect.p2.x < teweaponrect.p1.x)) collide = false;
					if((tweaponrect.p1.y > teweaponrect.p2.y)) collide = false;
					if((tweaponrect.p2.y < teweaponrect.p1.y)) collide = false;
				}

				if(collide)
				{
					if((ipFlags->S_ACTIVE) && (!ipFlags->S_DAMAGED) && (!ipFlags->S_INVULNERABLE) && (!ipFlags->S_DEAD)) player_hits++;
					activeEnemys[count]->doDamage(damage,(*temppl3)->GetDirection(),(*temppl3)->GetType());
					if((boss_level) && ((activeEnemys[0]->GetCurLife() == 0))) break;
					ProjectileFlags& proj_flags = (*temppl3)->GetProjectileFlags();

					if(!proj_flags.S_NON_PROJECTILE)
						(*temppl3)->DeActivate();
				}
			}
		
		}
		else
		{
			HPTRect& teweaponrect = player->GetWorldLoc();
		
			bool collide = true;
//			bool tempb;
//			int tempi;

			if((tweaponrect.p1.x == tweaponrect.p2.x) && (tweaponrect.p1.y == tweaponrect.p2.y))
				collide = false;
			else
			{
				collide = true;
				if((tweaponrect.p1.x > teweaponrect.p2.x)) collide = false;
				if((tweaponrect.p2.x < teweaponrect.p1.x)) collide = false;
				if((tweaponrect.p1.y > teweaponrect.p2.y)) collide = false;
				if((tweaponrect.p2.y < teweaponrect.p1.y)) collide = false;
			}
			
			if(collide)
			{
				ipFlags = player->GetStateFlags();
				if((!ipFlags->S_DAMAGED) && (!ipFlags->S_INVULNERABLE) && (!ipFlags->S_DEAD)) enemy_hits++;
				player->doDamage(damage,(*temppl3)->GetDirection(),0);
				if((boss_level) && ((player->GetCurLife() == 0))) break;

				ProjectileFlags& proj_flags = (*temppl3)->GetProjectileFlags();
				if(!proj_flags.S_NON_PROJECTILE)(*temppl3)->DeActivate();
			}
		}
//		}
		++temppl3;
	}
	}
	s3eDeviceYield(0);
//collide projectiles with level
	temppl = projectile_list.begin();
	//while(temppl != projectile_list.end())
	countmax = projectile_list.size();
	for(count = 0; count < countmax;count++)
	{
		//ProjectileFlags& proj_flags = (*temppl)->GetProjectileFlags();

		HPTRect& tweaponrect = (*temppl)->GetWeaponBounds();

		if(!((tweaponrect.p1.x == tweaponrect.p2.x) && (tweaponrect.p1.y == tweaponrect.p2.y)))
		//if(!proj_flags.S_NON_PROJECTILE)
			level_class->ProjectileCollideCheck(*temppl);
		
		++temppl;
	}
	s3eDeviceYield(0);

//remove dead projectiles
	temppl = projectile_list.begin();
	while(temppl != projectile_list.end())
	{
		if((*temppl)->IsActive())	
			++temppl;
		else
		{
			delete (*temppl);
			temppl = projectile_list.erase(temppl);
		}
	}
	s3eDeviceYield(0);
      
//Respawn Dead Enemies
	/*tempi = enemys.size();
	int xTileLoc = player->GetXTilePos();
	int yTileLoc = player->GetYTilePos();
	for(count = 0;count < tempi;count++)
	{
		if(!enemys[count]->GetStateFlags()->S_ACTIVE)
		{
			if((enemys[count]->GetXTilePos() < (xTileLoc - 30)) || (enemys[count]->GetXTilePos() > (xTileLoc + 30)) && 
   			   (enemys[count]->GetYTilePos() < (yTileLoc - 30)) || (enemys[count]->GetYTilePos() > (yTileLoc + 30)))
			{
				enemys[count]->Reset();
				enemys[count]->ReSpawn();
			}
		}
	}*/

	//overlay
	if(m_overlay && m_overlayoffTime >= 0 && m_overlayonTime >= 0)
	{
		m_TimeTillOverlayFlip -= time;
		if(m_TimeTillOverlayFlip <= 0)
		{
			if(m_overlayOn)
			{
				m_TimeTillOverlayFlip += m_overlayoffTime;
			}
			else
			{
				m_TimeTillOverlayFlip += m_overlayonTime;
			}
			m_overlayOn = !m_overlayOn;
		}
	}
   
	//Draw Game After Processing
	
	m_gameTimer.Update();
	Render();
	if((current_level % 3) == 0/* == 3)*/)
	{
		if(enemys[0]->GetStateFlags()->S_DEAD)
			//enemys[0]->GetCurLife() <= 0)
		{
			//hack, we beat the game!
			if(current_level == 9)
				saved_games.BeatGame();
			EndLevel();
		}
	}

#ifdef _DEBUG
	char temp[256];
	sprintf(temp, "[Player] (xtile:%d) (ytile:%d)", player->GetXTilePos(), player->GetYTilePos() );
//	OutputDebugString(temp);
#endif
	TutorialManager::Instance().CheckForTutorial(this);
}

void Game::ProcessMenu()
{
	menu->Update(time);
	graphics_engine->BeginFrame();
	menu->Render();
	graphics_engine->EndFrame();
}

void Game::ProcessPause()
{
	graphics_engine->BeginFrame();
	graphics_engine->DrawTiles1();
	graphics_engine->DrawTiles2();
	pauseBackground->Render();
	resumeGameButton->Render();
	quitGameButton->Render();	
	graphics_engine->EndFrame();
}

void Game::ProcessTutorial()
{
	graphics_engine->BeginFrame();
	graphics_engine->DrawTiles1();
	graphics_engine->DrawTiles2();
	graphics_engine->EnableAlphaTest();
	level_class->Render();
	player->Render();
	int tempi = activeEnemys.size();
	for(int count = 0;count < tempi;count++)
	{
		activeEnemys[count]->Render();
	}
	list<Projectile*>::iterator temppl = projectile_list.begin();
	list<Projectile*>::iterator projEnd = projectile_list.end();
	while(temppl != projEnd)
	{
		(*temppl)->Render();
		++temppl;
	}
	graphics_engine->DisableAlphaTest();
	if(m_overlay && m_overlayOn)
		m_overlay->Render();
	RenderHUD();
	TutorialManager::Instance().Tick();
	graphics_engine->EndFrame();
}

void Game::ProcessMovie()
{
	if(transition->sActive)
		transition->Update();
	
	if(m_overlay)
		m_overlay->Release();
	m_overlay = NULL;
	
	for(int i = 0;i < m_rain.size();++i)
	{
		m_rain[i].m_sprites[0]->Release();
		m_rain[i].m_sprites[1]->Release();
		m_rain[i].m_sprites[2]->Release();
	}
	m_rain.clear();

	if(!cut_scenes->IsDone())
	{
		cut_scenes->Update(time);
		graphics_engine->BeginFrame();
		cut_scenes->Render();
		
		if(transition->sActive)
			transition->Render();	
		
		graphics_engine->EndFrame();
	}
	else
	{
		input_engine->RegisterInputController(this);
		//music->Stop();
		//musicPlayer->Stop();
		if(RequestGState == game_menu)
		{
			menu->RestartMenu(this,gameStateManager.GetGameStateInfo().canResume,&saved_games);
			ChangeState(RequestGState);
			cut_scenes->FreeAssets();
			//music->ChangeSong(0);
			//music->Play();
		}
		else
		{
			ChangeState(game_level_load);
			
			//graphics_engine->SetBackgroundImage(0);
			//level_class->EnableLevel();
				SetSpawnPoints();
				switch(current_level)
				{
				case 1:
					LoadLevel1();
					break;
				case 2:
					LoadLevel2();
					break;
				case 3:
					LoadLevel3();
					break;
				case 4:
					LoadLevel4();
					break;
				case 5:
					LoadLevel5();
					break;
				case 6:
					LoadLevel6();
					break;
				case 7:
					LoadLevel7();
					break;
				case 8:
					LoadLevel8();
					break;
				case 9:
					LoadLevel9();
					break;
				case 10:
					LoadLevel10();
					break;
				case 11:
					LoadLevel11();
					break;
				case 12:
					LoadLevel12();
					break;
				default:
					LoadLevel1();
					break;
				};
		}
	}
}

void Game::ProcessStart()
{
	/*if(AppConfig::Instance().IsIAd())
	{
		if (!s3eExtIAdAvailable())
		{			
			ChangeState(game_noiad);
			RequestGState = game_noiad;
			return;
		}
	}*/
	ChangeState(game_movie);
	RequestGState = game_menu;
	cut_scenes->StartMovie(0);
}

void Game::ProcessEnd()
{
	//music->Stop();
	ChangeState(game_movie);
	RequestGState = game_menu;
	//music->ChangeSong(1);
	//music->Play();
	cut_scenes->StartMovie(16);
}

void Game::ProcessCredits()
{
}

void Game::ProcessLevelLoad()
{
	graphics_engine->BeginFrame();
	switch(m_levelLoadState)
	{
		case 0:
			LoadLevelPhase1();
			m_levelLoadState++;
			break;
		case 1:
			LoadLevelPhase2();
			if(!transition->sActive)
				m_levelLoadState++;
			break;
			break;
		case 2:
			LoadLevelPhase3();
			m_levelLoadState++;			
			break;
		case 3:
			m_levelLoadState++;
			LoadLevelPhase4();
			break;
		case 4:
			LoadLevelPhase5();
			break;	
		case 5:
			//LoadLevelPhase5();
			
			//if(!transition->sActive)
			//{
			transition->Render();
				ChangeState(RequestGState);				
				transition->Begin(false);
				//transition->Update();
				level_class->EnableLevel();
				//static_background->SetImage(m_levelStaticBackground);
				graphics_engine->SetBackgroundImage(static_background);
				m_playTime = 0;
				
				// set the previous spawn position for when spencer dies
				if (resumeGame && !gameStateManager.GetGameStateInfo().inTheater)
				{
					GameStateInfo gsInfo = gameStateManager.GetGameStateInfo();
					
					// Enemy dead states
					Enemy* tenemy;
					for(int count = 0;count < enemys.size();count++)
					{
						tenemy = enemys[count];
						if (gsInfo.enemyStates[count].isDead == true)
								tenemy->forceDeath();
						else
								tenemy->SetCurLife(gsInfo.enemyStates[count].life);
					}
					
					// object states
					list<Object*> object_list = level_class->GetObjectList();		
					int i = 0;
					for (list<Object*>::iterator tobject = object_list.begin(); tobject != object_list.end(); tobject++)
					{
						(*tobject)->SetState(gsInfo.objectStates[i]);
						i++;
					}
					
					//player->ipFlags = gsInfo.playerInfo.ipFlags;
					player->ipFlags.S_ON_GROUND = gsInfo.playerInfo.S_ON_GROUND;
					player->ipFlags.S_NEXT_TO_LADDER = gsInfo.playerInfo.S_NEXT_TO_LADDER;
					player->ipFlags.S_ON_LADDER = gsInfo.playerInfo.S_ON_LADDER;
					player->ipFlags.S_LADDER_TYPE = gsInfo.playerInfo.S_LADDER_TYPE;
					player->ipFlags.S_CROUCH = gsInfo.playerInfo.S_CROUCH;
					player->ipFlags.S_CROUCH_TOGGLE = gsInfo.playerInfo.S_CROUCH_TOGGLE;
					player->ipFlags.S_CRAWL = gsInfo.playerInfo.S_CRAWL;
					
					player->bossUsedOnlyBasicAttack = gsInfo.playerInfo.bossUsedOnlyBasicAttack;
					player->bossNoDamageTaken = gsInfo.playerInfo.bossNoDamageTaken;
					
					
					if (gsInfo.playerInfo.S_ON_LADDER)
						player->LadderDownGrab();
					
					//player->moveStop();
					//player->player_spr->SetFrameSet(gsInfo.playerInfo.frameSet);
					player->SetSpawnPoint(gsInfo.playerInfo.lastSpawnPoint.x, gsInfo.playerInfo.lastSpawnPoint.y);		
					m_playTime = gsInfo.playTime;
					this->m_SpawnPointInfo = gsInfo.spawnPointInfo;
				}
			else
			{
				CaptureSpawnPointInfo();
			}
				
				InitializeLevelState();
				resumeGame = false;
//			musicPlayer->Play();
				ai_engine->Resume();
			loadingButton->Release();
			loadingButton = NULL;
			loadingLevelTags->Release();
			loadingLevelTags = NULL;
			loadingObjectiveTags->Release();
			loadingObjectiveTags = NULL;
			awardInfo->Release();
			awardInfo = NULL;
			delete endTimeDisplay;
			endTimeDisplay = NULL;
			//}
		
			break;			
	};
	graphics_engine->EndFrame();
}

void Game::ProcessNoIAD()
{
	graphics_engine->SetBackgroundColor(255,0,0);
	graphics_engine->BeginFrame();
	graphics_engine->EndFrame();
}

void Game::InitializeLevelState()
{
	LevelStateInfo levelStateInfo;
	levelStateInfo = levelStateManager.GetLevelStateInfo(current_level);
	
	// object states
	list<Object*> object_list = level_class->GetObjectList();		
	int iHealth = 0;
	int iEnergy = 0;
	int iAttack = 0;
	int iWeapon = 0;
	
	for (list<Object*>::iterator tobject = object_list.begin(); tobject != object_list.end(); tobject++)
	{
		if ((*tobject)->GetUpgradeType() == LEVELSTATETYPE_NOTUPGRADE)
			continue;
		// Attempt to set health states
		else if (iHealth < levelStateInfo.numHealthUpgrades && (*tobject)->GetUpgradeType() == LEVELSTATETYPE_HEALTH)
		{
			(*tobject)->SetUpgradeState(levelStateInfo.hasHealthUpgrade[iHealth]);
			iHealth++;
			continue;
		}
		// Attempt to set energy states
		else if (iEnergy < levelStateInfo.numEnergyUpgrades && (*tobject)->GetUpgradeType() == LEVELSTATETYPE_ENERGY)
		{
			(*tobject)->SetUpgradeState(levelStateInfo.hasEnergyUpgrade[iEnergy]);
			iEnergy++;
			continue;
		}
		// Attempt to set attack upgrade states
		else if (iAttack < levelStateInfo.numAttackUpgrades && (*tobject)->GetUpgradeType() == LEVELSTATETYPE_ATTACK)
		{
			(*tobject)->SetUpgradeState(levelStateInfo.hasAttackUpgrade[iAttack]);
			iAttack++;
			continue;
		}
		// Attempt to set weapon upgrade states
		else if (iWeapon < levelStateInfo.numWeaponUpgrades && (*tobject)->GetUpgradeType() == LEVELSTATETYPE_WEAPON)
		{
			(*tobject)->SetUpgradeState(levelStateInfo.hasWeaponUpgrade[iWeapon]);
			iWeapon++;
			continue;
		}
	}
}

void Game::ChangeState(GAME_STATE newState)
{
	ActiveGState = newState;
}

void Game::InputChanged()
{	
	if (ActiveGState != game_ingame) return;
	
	float aX = analogStick->AnalogX();
	float aY = analogStick->AnalogY();
	if((s3eKeyboardGetState(s3eKeyAbsLeft) & S3E_KEY_STATE_DOWN))
		aX = -1.0f;
	if((s3eKeyboardGetState(s3eKeyAbsRight) & S3E_KEY_STATE_DOWN))
		aX = 1.0f;
	if((s3eKeyboardGetState(s3eKeyAbsDown) & S3E_KEY_STATE_DOWN))
		aY = 1.0f;
	if((s3eKeyboardGetState(s3eKeyAbsUp) & S3E_KEY_STATE_DOWN))
		aY = -1.0f;

	float absX = fabs(aX);
	float absY = fabs(aY); 
	bool upPressed = false;
	
	//player->SetAnalogModY(absY);
	//if (absY > 0)
	//	player->SetAnalogModY(1);
	//if (absX > 0)
	//	player->SetAnalogX(1);
	
	//if (absY > .3)
	player->SetAnalogModY(1);
	//else
	//	player->SetAnalogModY(absY * 3.33);
	
	//if (absX > .3)
	player->SetAnalogX(1);
	//else
	//	player->SetAnalogX(absX * 3.33);
	
	if(player->ipFlags.S_NEXT_TO_LADDER)
	{
		if (absX == 0 && absY == 0)
		{
			player->moveStop();
		}
		else if ((absX/0.7f) < (absY/0.5f))
		{			
			if (aY < 0) // up
			{
				player->moveUp();
				upPressed = true;
			}
			else // down
			{
				player->moveDown();
			}
		}
		else /*if (absX > absY)*/
		{
			if (aX < 0) // left
			{
				player->moveLeft();
			}
			else // right
			{
				player->moveRight();
			}
		
		}
	}
	else
	{
		if (absX == 0 && absY == 0)
		{
			player->moveStop();
		}
		else if (absX > absY)
		{
			if (aX < 0) // left
			{
				player->moveLeft();
			}
			else // right
			{
				player->moveRight();
			}
		
		}
		else /*if (absX < absY)*/
		{			
			if (aY < 0) // up
			{
				player->moveUp();
				upPressed = true;
			}
			else // down
			{
				player->moveDown();
			}
		}
	}
	
	if (jumpButton->IsActing() || (s3eKeyboardGetState(s3eKeyAbsGameB) & S3E_KEY_STATE_DOWN) ||
		(s3eKeyboardGetState(s3eKeyS) & S3E_KEY_STATE_DOWN))
	{		
		player->moveButton3();
	}
	
	if (attackButton->IsActing() || (s3eKeyboardGetState(s3eKeyAbsGameA) & S3E_KEY_STATE_DOWN) ||
		(s3eKeyboardGetState(s3eKeyA) & S3E_KEY_STATE_DOWN))
	{
		player->moveButton4();
	}
	
	if (specialAttackButton->IsActing() || (s3eKeyboardGetState(s3eKeyAbsGameC) & S3E_KEY_STATE_DOWN) ||
		(s3eKeyboardGetState(s3eKeyQ) & S3E_KEY_STATE_DOWN))
	{
		player->moveButton5();
	}

	if (grenadeButton->IsActing() || (s3eKeyboardGetState(s3eKeyAbsGameD) & S3E_KEY_STATE_DOWN) ||
		(s3eKeyboardGetState(s3eKeyW) & S3E_KEY_STATE_DOWN))
	{
		player->ThrowGrenade();
	}
}

void Game::LoadLevel(int arg)
{	
	// hack to skip levels 7 - 12 for the iPhone version
	if (arg > 6)
		arg += 6;
	
	player->ResetIPlayerFlags();
	player->moveStop();
	player->SetLevel(level_class);
	player->setGravityConstant(500.0f);
	player->SetSprite(0,CR::AssetList::Player_Spencer);
	
	aiobject.level_interface = level_class;
	aiobject.player_input = player;
	aiobject.level = arg;
	ai_engine->RegisterAIObject(&aiobject);
	ai_engine->RemoveAll();
	list<Projectile*>::iterator temppl = projectile_list.begin();
	while(temppl != projectile_list.end())
	{
		delete (*temppl);
		temppl = projectile_list.erase(temppl);
	}
	Enemy* tenemy;
	for(int count = 0;count < enemys.size();count++)
	{
		tenemy = enemys[count];
		delete tenemy;
	}
	enemys.clear();
	player->ReSpawn();
	m_healthTick = 0;
	tallyNumBeacons = 0;
}

void Game::LevelTransition(bool arg)
{
	//Screen Transition
	if(transition->sActive)
		transition->Update();
}

void Game::LoadLevelPhase1()
{
	loadScreenButton->Disabled(true);
	//music->Stop();
	transition->Begin(true);
	cut_scenes->Render();
	transition->Update();
	weaponSelect->Disabled(true);
	//m_ProgressMeter = new ProgressMeter(m_levelLoadingText.c_str(), 10);
	//m_ProgressMeter->ProgressMeterStart();
	
	/*graphics_engine->SetBackgroundColor(0,0,0);
	graphics_engine->SetBackgroundImage(0);
	graphics_engine->SetTileBackgroundImage(0,0);
	graphics_engine->SetTileBackgroundImage(1,0);
	
	level_class->UnloadLevel();	
	m_ProgressMeter->Update();*/
}

void Game::LoadLevelPhase2()
{
	cut_scenes->Render();
	transition->Update();
	
	/*graphics_engine->SetBackgroundColor(0,0,0);
	level_load->SetImage(m_levelLoadImage);
	graphics_engine->SetTileBackgroundImage(0,0);
	graphics_engine->SetTileBackgroundImage(1,0);
	LoadLevel(current_level);
	m_ProgressMeter->Update();*/
}

void Game::LoadLevelPhase3()
{	
	graphics_engine->SetBackgroundImage(0);
	loadingButton = graphics_engine->CreateSprite1();
	loadingButton->SetImage(AssetList::Transition_Banner_Bottom);
	loadingButton->SetFrameSet(0);
	loadingButton->SetPositionAbsalute(285, 280);
	if(AppConfig::Instance().IsIpad())
		loadingButton->SetDesignSize(444,65);

	loadingLevelTags = graphics_engine->CreateSprite1();
	loadingLevelTags->SetImage(AssetList::Level_Load_Tags);
	if(current_level == 10)
		loadingLevelTags->SetFrameSet(10);
	else
		loadingLevelTags->SetFrameSet(current_level-1);
	loadingLevelTags->SetPositionAbsalute(180, 34);
	if(AppConfig::Instance().IsIpad())
		loadingLevelTags->SetDesignSize(334,18);
	
	loadingObjectiveTags = graphics_engine->CreateSprite1();
	loadingObjectiveTags->SetImage(AssetList::Level_Objective_Tags);
	loadingObjectiveTags->SetFrameSet(current_level-1);
	loadingObjectiveTags->SetPositionAbsalute(130, 80);
	if(AppConfig::Instance().IsIpad())
	{
		loadingObjectiveTags->SetPositionAbsalute(130, 84);
		loadingObjectiveTags->SetDesignSize(232,44);
	}
	
	//tallyFont = graphics_engine->CreateSprite1();
	//tallyFont->SetImage(AssetList::Tally_Font);
	
	if(current_level == 10)
		endTimeDisplay = NULL;
	else
	{
		endTimeDisplay = new TimeDisplay();
		endTimeDisplay->SetFont(bankGothic);
		endTimeDisplay->SetShowFrontZero(false);
		if(AppConfig::Instance().IsIpad())
			endTimeDisplay->SetPosition(425, 216);
		else
			endTimeDisplay->SetPosition(425, 206);
		int lev = current_level;
		if(current_level >= 13)
			lev -= 6;
		endTimeDisplay->SetTime(levelAchievementTimes[lev-1]);
	}
	
	awardInfo = graphics_engine->CreateSprite1();
	awardInfo->SetImage(AssetList::Award_Info);
	if(current_level == 10)
		awardInfo->SetFrameSet(2);
	else
	{
		if(AppConfig::Instance().IsIpad())
			awardInfo->SetFrameSet(current_level%3==0?0:1);
		else
			awardInfo->SetFrameSet(current_level%3==0?1:0);
	}
	
	if(AppConfig::Instance().IsIpad())
	{
		if(current_level == 10)
			awardInfo->SetPositionAbsalute(260, 170);
		else
			awardInfo->SetPositionAbsalute(310, 170);
	}
	else
	{
		if(current_level == 10)
			awardInfo->SetPositionAbsalute(260, 160);
		else
			awardInfo->SetPositionAbsalute(310, 160);
	}

	if(AppConfig::Instance().IsIpad())
		awardInfo->SetDesignSize(325,119);
	
	cut_scenes->FreeAssets();
	transition->Render();
	loadingButton->Render();
	loadingLevelTags->Render();
	loadingObjectiveTags->Render();
	awardInfo->Render();
	if(endTimeDisplay)
		endTimeDisplay->Render();
	/*int lev = current_level;
	if(current_level >= 13)
		lev -= 6;
	tallyFont->SetPositionAbsalute(450,200);
	tallyFont->SetFrameSet(levelAchievmentTimes[lev].first);
	tallyFont->Render();*/
	//tallyFont->SetFrameSet(10);
	//tallyFont->Render();

	
	//-- Start of enemy's	
	//level_class->LoadLevel(current_level);
	//level_class->DisableLevel();
	//m_ProgressMeter->Update();
	//graphics_engine->SetBackgroundImage(level_load);
	//loadScreenButton->Disabled(false);
	//m_ProgressMeter->ProgressMeterStop();
	//m_ProgressMeter->Finish();
	//delete m_ProgressMeter;
}

void Game::LoadLevelPhase4()
{			
	transition->Render();
	loadingButton->Render();
	loadingLevelTags->Render();
	loadingObjectiveTags->Render();
	awardInfo->Render();
	if(endTimeDisplay)
		endTimeDisplay->Render();
	
	/*int lev = current_level;
	if(current_level >= 13)
		lev -= 6;
	tallyFont->SetPositionAbsalute(450,200);
	tallyFont->SetFrameSet(levelAchievmentTimes[lev].first);
	tallyFont->Render();*/
	
	
	loadScreenButton->Disabled(false);
	graphics_engine->SetBackgroundColor(0,0,0);
	 graphics_engine->SetBackgroundImage(0);
	 graphics_engine->SetTileBackgroundImage(0,0);
	 graphics_engine->SetTileBackgroundImage(1,0);
	 
	 level_class->UnloadLevel();	
	graphics_engine->SetBackgroundColor(0,0,0);
	 //level_load->SetImage(m_levelLoadImage);
	 graphics_engine->SetTileBackgroundImage(0,0);
	 graphics_engine->SetTileBackgroundImage(1,0);
	 LoadLevel(current_level);
	level_class->LoadLevel(current_level);
	level_class->DisableLevel();
	static_background->SetImage(m_levelStaticBackground);
	//m_ProgressMeter->Update();
	//graphics_engine->SetBackgroundImage(level_load);
	
	//Screen Transition
	//transition->Begin(true);
	
	//level_class->EnableLevel();
	//static_background->SetImage(m_levelStaticBackground);
	//graphics_engine->SetBackgroundImage(static_background);
	//	QueryPerformanceCounter(&starttime);
	//	QueryPerformanceCounter(&currenttime);
	player->SetMaxAttacks(2);
	//music->ChangeSong(2);
	//music->Play();
}

void Game::LoadLevelPhase5()
{		
	loadingButton->SetFrameSet(1);
	transition->Render();
	loadingButton->Render();
	loadingLevelTags->Render();
	loadingObjectiveTags->Render();
	awardInfo->Render();
	if(endTimeDisplay)
		endTimeDisplay->Render();	

	//tallyFont->SetPositionAbsalute(435,220);
	//tallyFont->SetFrameSet(levelAchievmentTimes[lev].first);
	//tallyFont->Render();

	

	//Screen Transition
	//LevelTransition();	

}

void Game::LoadLevel1()
{
	m_levelLoadingText = "Loading Chapter 1 - Level 1";
	m_levelLoadImage = AssetList::chpater_1_a_intro_screen;
	if(saved_games.IsQualityMode())
		m_levelStaticBackground = AssetList::chapter_1_a_static_background;
	else
		m_levelStaticBackground = AssetList::BackedBG1;
	m_levelLoadState = 0;
	MusicManager::Instance().PlaySong(MusicManager::Level1A);
//musicPlayer->ChangeSong(MUSIC_1A);
	player->InBossBattle(false);

	if(saved_games.IsQualityMode())
	{
		m_overlay = graphics_engine->CreateSprite1();
		m_overlay->SetImage(CR::AssetList::Overlay1A);
		m_overlay->SetPositionAbsalute(240,160);
		m_overlayonTime = -1.0f;
		m_overlayoffTime = -1.0f;
		m_overlayOn = true;
	}
}

void Game::LoadLevel2()
{
	m_levelLoadingText = "Loading Chapter 1 - Level 2";
	m_levelLoadImage = AssetList::chapter_1b_intro_screen;
	if(saved_games.IsQualityMode())
		m_levelStaticBackground = AssetList::chapter_1_a_static_background;
	else
		m_levelStaticBackground = AssetList::BackedBG1;
	m_levelLoadState = 0;
	MusicManager::Instance().PlaySong(MusicManager::Level1B);
//musicPlayer->ChangeSong(MUSIC_1B);
	player->InBossBattle(false);

	if(saved_games.IsQualityMode())
	{
		m_overlay = graphics_engine->CreateSprite1();
		m_overlay->SetImage(CR::AssetList::Overlay1B);
		m_overlay->SetPositionAbsalute(240,160);
		m_overlayonTime = -1.0f;
		m_overlayoffTime = -1.0f;
		m_overlayOn = true;
	}
}

void Game::LoadLevel3()
{
	m_levelLoadingText = "Loading Chapter 1 - Level 3";
	m_levelLoadImage = AssetList::chpater_1_level_3_intro_scene;
	if(saved_games.IsQualityMode())
		m_levelStaticBackground = AssetList::chapter_1_a_static_background;
	else
		m_levelStaticBackground = AssetList::BackedBG1;
	m_levelLoadState = 0;
	MusicManager::Instance().PlaySong(MusicManager::BossBattle);
//musicPlayer->ChangeSong(MUSIC_BOSS);
	
	player->InBossBattle(true);

	if(saved_games.IsQualityMode())
	{
		m_overlay = graphics_engine->CreateSprite1();
		m_overlay->SetImage(CR::AssetList::Overlay1C);
		m_overlay->SetPositionAbsalute(240,160);
		m_overlayonTime = -1.0f;
		m_overlayoffTime = -1.0f;
		m_overlayOn = true;
	}
}

void Game::LoadLevel4()
{
	m_levelLoadingText = "Loading Chapter 2 - Level 1";
	m_levelLoadImage = AssetList::chapter_2_level_1_intro_screen;
	if(saved_games.IsQualityMode())
		m_levelStaticBackground = AssetList::chapter_2_level_1_static_background;
	else
		m_levelStaticBackground = AssetList::BackedBG2A;
	m_levelLoadState = 0;	
	MusicManager::Instance().PlaySong(MusicManager::Level2A);
//musicPlayer->ChangeSong(MUSIC_2A);
	player->InBossBattle(false);

	if(saved_games.IsQualityMode())
	{
		m_overlay = graphics_engine->CreateSprite1();
		m_overlay->SetImage(CR::AssetList::PrisonOverlay);
		m_overlay->SetPositionAbsalute(240,160);
		m_overlayonTime = -1.0f;
		m_overlayoffTime = -1.0f;
		m_overlayOn = true;
	}
}

void Game::LoadLevel5()
{
	m_levelLoadingText = "Loading Chapter 2 - Level 2";
	m_levelLoadImage = AssetList::Chapter_2_level_2_intro_screen;
	if(saved_games.IsQualityMode())
		m_levelStaticBackground = AssetList::chpater_2_level_2_static_background;
	else
		m_levelStaticBackground = AssetList::BackedBG2B;
	m_levelLoadState = 0;	
	MusicManager::Instance().PlaySong(MusicManager::Level2B);
//musicPlayer->ChangeSong(MUSIC_2B);
	player->InBossBattle(false);

	if(saved_games.IsQualityMode())
	{
		m_overlay = graphics_engine->CreateSprite1();
		m_overlay->SetImage(CR::AssetList::LabOverlay);
		m_overlay->SetPositionAbsalute(240,160);
		m_overlayonTime = 1.0f;
		m_overlayoffTime = 1.0f;
		m_TimeTillOverlayFlip = m_overlayoffTime;
		m_overlayOn = false;
	}
}

void Game::LoadLevel6()
{
	m_levelLoadingText = "Loading Chapter 2 - Level 3";
	m_levelLoadImage = AssetList::Chapter_2_level_3_intro_screen;
	if(saved_games.IsQualityMode())
		m_levelStaticBackground = AssetList::Chapter_2_level_3_static_background;
	else
		m_levelStaticBackground = AssetList::BackedBG2C;
	m_levelLoadState = 0;
	MusicManager::Instance().PlaySong(MusicManager::BossBattle);
//musicPlayer->ChangeSong(MUSIC_BOSS);
	
	player->InBossBattle(true);

	if(saved_games.IsQualityMode())
	{
		m_overlay = graphics_engine->CreateSprite1();
		m_overlay->SetImage(CR::AssetList::Overlay2C);
		m_overlay->SetPositionAbsalute(240,160);
		m_overlayonTime = -1.0f;
		m_overlayoffTime = -1.0f;
		m_overlayOn = true;
	}
}

void Game::LoadLevel7()
{
	m_levelLoadingText = "Loading Chapter 3 - Level 1";
	m_levelLoadImage = AssetList::Chpater_5_level_a_intro;
	if(saved_games.IsQualityMode())
		m_levelStaticBackground = AssetList::Chapter_5_level_a_static;
	else
		m_levelStaticBackground = AssetList::BackedBG3A;
	m_levelLoadState = 0;
	MusicManager::Instance().PlaySong(MusicManager::Level3A);
//musicPlayer->ChangeSong(MUSIC_3A);
	player->InBossBattle(false);

	if(saved_games.IsQualityMode())
	{
		m_overlay = graphics_engine->CreateSprite1();
		m_overlay->SetImage(CR::AssetList::Overlay_3A);
		m_overlay->SetPositionAbsalute(240,160);
		m_overlayonTime = -1.0f;
		m_overlayoffTime = -1.0f;
		m_overlayOn = true;
	}
}

void Game::LoadLevel8()
{
	m_levelLoadingText = "Loading Chapter 3 - Level 2";
	m_levelLoadImage = AssetList::Chapter_5_level_b_intro;
	if(saved_games.IsQualityMode())
		m_levelStaticBackground = AssetList::Chapter_5_level_b_static;
	else
		m_levelStaticBackground = AssetList::BackedBG3B;
	m_levelLoadState = 0;	
	MusicManager::Instance().PlaySong(MusicManager::Level3B);
//musicPlayer->ChangeSong(MUSIC_3B);
	player->InBossBattle(false);

	if(saved_games.IsQualityMode())
	{
		m_overlay = graphics_engine->CreateSprite1();
		m_overlay->SetImage(CR::AssetList::Overlay3B);
		m_overlay->SetPositionAbsalute(240,160);
		m_overlayonTime = -1.0f;
		m_overlayoffTime = -1.0f;
		m_overlayOn = true;
	}

	
	for(int i = 0;i < 20;++i)
	{
		Rain2 temp;
		temp.m_sprites.push_back(graphics_engine->CreateSprite1(true));
		temp.m_sprites.back()->SetImage(AssetList::rain3b1);
		temp.m_sprites.back()->SetFrameRate(15);
		temp.m_sprites.push_back(graphics_engine->CreateSprite1(true));
		temp.m_sprites.back()->SetImage(AssetList::rain3b2);
		temp.m_sprites.back()->SetFrameRate(15);		
		temp.m_sprites.push_back(graphics_engine->CreateSprite1(true));
		temp.m_sprites.back()->SetImage(AssetList::rain3b3);
		temp.m_sprites.back()->SetFrameRate(15);
		
		temp.m_delay = (rand()&100)/100.0f;
		temp.m_currentSprite = -1;
		
		/*if(i < 5)
		{
			temp.m_y = 264;
		}
		else if(i >= 15)
		{
			temp.m_y = 216;
		}
		else*/
		{
			temp.m_y = 160;//408;
		}
		m_rain.push_back(temp);
	}
}


void Game::LoadLevel9()
{
	m_levelLoadingText = "Loading Chapter 3 - Level 3";
	m_levelLoadImage = AssetList::Chapter_5_level_c_intro;
	if(saved_games.IsQualityMode())
		m_levelStaticBackground = AssetList::Chapter_5_level_c_static;
	else
		m_levelStaticBackground = AssetList::BackedBG3C;
	m_levelLoadState = 0;	
	MusicManager::Instance().PlaySong(MusicManager::BossBattle);
//musicPlayer->ChangeSong(MUSIC_BOSS);
	
	player->InBossBattle(true);

	if(saved_games.IsQualityMode())
	{
		m_overlay = graphics_engine->CreateSprite1();
		m_overlay->SetImage(CR::AssetList::Overlay3C);
		m_overlay->SetPositionAbsalute(240,160);
		m_overlayonTime = -1.0f;
		m_overlayoffTime = -1.0f;
		m_overlayOn = true;
	}
}

void Game::LoadLevel10()
{
	m_levelLoadingText = "Loading Bonus Level - Level 1";
	m_levelLoadImage = AssetList::Chapter_5_level_b_intro;
	if(saved_games.IsQualityMode())
		m_levelStaticBackground = AssetList::BonusStaticBG;
	else
		m_levelStaticBackground = AssetList::BackedBGB1;
	m_levelLoadState = 0;	
	MusicManager::Instance().PlaySong(MusicManager::Bonus1);
//musicPlayer->ChangeSong(MUSIC_3B);
	player->InBossBattle(false);
	player->SetEnergyLevel(player->GetMaxEnergyLevel());
	player->SetCurLife(5*5);

	if(saved_games.IsQualityMode())
	{
		m_overlay = graphics_engine->CreateSprite1();
		m_overlay->SetImage(CR::AssetList::OverlayBonus);
		m_overlay->SetPositionAbsalute(240,160);
		m_overlayonTime = -1.0f;
		m_overlayoffTime = -1.0f;
		m_overlayOn = true;
	}
}

void Game::LoadLevel11()
{
	// TODO: Add functionality
}

void Game::LoadLevel12()
{
	// TODO: Add functionality
	// NOTE: There is a hack in the AI loading code that will need to be fixed if this level needs AI!!!
}

/* Steph
void Game::LoadLevel16()
{
//	sound_engine->LoadHSF(hptstrcat(path, ("sounds//0.hsf")), 1);
	//music->Stop();
	m_ProgressMeter = new ProgressMeter("Loading Chapter 6 - Level 1\0", 10);
	NEW(m_ProgressMeter);

	m_ProgressMeter->ProgressMeterStart();

	graphics_engine->SetBackgroundColor(0,0,0);
	graphics_engine->SetBackgroundImage(0);
//	graphics_engine->SetClipper(0);
	graphics_engine->SetTileBackgroundImage(0,0);
	graphics_engine->SetTileBackgroundImage(1,0);
//	graphics_engine->Position(25,140);
//	(*graphics_engine) << blue12;
//	graphics_engine->BeginFrame();
//	(*graphics_engine) << "Loading Chapter 1 - Level 1";
	level_class->UnloadLevel();

//	graphics_engine->EndFrame();
  	if(!mute_music)
	{
//		sound_engine->LoadHSF(hptstrcat(path, ("sounds//sounds.hsf")), 0);
	//	sound_engine->LoadHSF(hptstrcat(path, ("sounds//sounds.hsf")), 1);
//		sound_engine->LoadHSF(hptstrcat(path, ("sounds//level1a-m.hsf")), 1);
//		music = sound_engine->CreateSoundObject(0, 1);

	}
 
	//graphics_engine->LoadHGF(0,hptstrcat(path,TEXT("graphics//main.hgf")));
//	graphics_engine->LoadHGF(1, hptstrcat(path,("graphics//level1a.hgf")));
 	m_ProgressMeter->Update();

//	ai_engine->Pause();
	graphics_engine->SetBackgroundColor(0,0,0);
	level_load->SetImage(76);
//	graphics_engine->SetClipper(0);
	graphics_engine->SetTileBackgroundImage(0,0);
	graphics_engine->SetTileBackgroundImage(1,0);
	//player->SetWorldLocAbsolute(300,70);
//	player->SetWorldLocAbsoluteT(5,10);
 	
	LoadLevel(1);

	m_ProgressMeter->Update();
//MessageBox(0,("done with loop"),("error"),MB_OK);
//-- Start of enemy's	
	level_class->LoadLevel(16);
	level_class->DisableLevel();

	m_ProgressMeter->Update();
	m_ProgressMeter->ProgressMeterStop();
	m_ProgressMeter->Finish();
	delete m_ProgressMeter;


//	LoadEnemies1();
//	m_ProgressMeter->Update();
 
//	m_ProgressMeter->Update();

	disable_input = true;
	//input_engine->PollInput();
	disable_input = false;

	graphics_engine->SetBackgroundImage(level_load);
	
	graphics_engine->BeginFrame();
	graphics_engine->EndFrame();
 

//	MessageBox(0,("done with loop"),("error"),MB_OK);

	key_pressed = false;
	while(!key_pressed)
	{
//		MessageBox(0,("done with loop"),("error"),MB_OK);
		//input_engine->PollInput();
	}
	
	//Screen Transition
	LevelTransition();

	level_class->EnableLevel();
	static_background->SetImage(75);
	graphics_engine->SetBackgroundImage(static_background);

//	MessageBox(0,("done with loop"),("error"),MB_OK);

//	ai_engine->Resume();
	if(!mute_music)
	{
//		if((music != 0) && (mute_music != true)) music->HPTPlaySound(true);
	}
//	QueryPerformanceCounter(&starttime);
//	QueryPerformanceCounter(&currenttime);
	player->SetMaxAttacks(2);
	//music->ChangeSong(2);
	//music->Play();
}*/

void Game::StartGame(int nLevel, bool skipMovie)
{
//musicPlayer->Stop();
//	ai_engine->Pause();
//	ai_engine->RemoveAll();
	
	// setup button positions
	UpdateHUD();
	
	// unload all projectiles
	list<Projectile*>::iterator temppl = projectile_list.begin();
	while(temppl != projectile_list.end())
	{
		delete (*temppl);
		temppl = projectile_list.erase(temppl);
	}
	
	// unload all enemies
	Enemy* tenemy;
	for(int count = 0;count < enemys.size();count++)
	{
		tenemy = enemys[count];
		delete tenemy;
	}
	enemys.clear();
	
	// unload current level
	level_class->DisableLevel();
	level_class->UnloadLevel();

	input_engine->RegisterInputController(this);
	ChangeState(game_ingame);

	const SaveGameInfo saveInfo = saved_games.GetSaveGameInfo();
	new_level = true;
	current_level = nLevel;
	player->SetCurLife(saveInfo.health);
	//player->SetCurLife(50);
	player->SetNumLifeBars(saveInfo.health_bars);
	player->SetBasicAttackDamage(saveInfo.basic_weapon);
	player->SetMaxEnergyLevel(saveInfo.energy_max);
	player->SetEnergyLevel(saveInfo.energy);
	//player->SetMaxAttacks(tempp[pnumb].games[arg-1].weapon);
	//player->SetScore(saveInfo.score);
	player->SetExtraLives(saveInfo.nlives);
	player->ResetItems();
	player->ResetWeapons();
	if(saveInfo.weapons[0]) player->AddWeapon(0);
	if(saveInfo.weapons[1]) player->AddWeapon(1);
	if(saveInfo.weapons[2]) player->AddWeapon(2);
	if(saveInfo.weapons[3]) player->AddWeapon(3);
	if(saveInfo.weapons[4]) player->AddWeapon(4);
	player->SetGremades(saveInfo.grenades);
	player->SetAttack(0);
	
	for (int i = 0; i < 12; i++)
	{
		player->levelAwards[i].bronzeAchievement = saveInfo.levelInfo[i].bronzeAchievement;
		player->levelAwards[i].silverAchievement = saveInfo.levelInfo[i].silverAchievement;
		player->levelAwards[i].goldAchievement = saveInfo.levelInfo[i].goldAchievement;
	}
	
	// Difficulty level is always Normal (iPhone)
	difficulty_level = 1; // 1 = Normal
	player->SetDifficultyLevel(difficulty_level);
	ProjectilePoints::SetModifier(player->GetModifier());
	player_hits = 0;
	enemy_hits = 0;
	
	RequestGState = game_ingame;
	
	if (!skipMovie)
	{
		ChangeState(game_movie);
		level_class->DisableLevel();
		cut_scenes->StartMovie(current_level >= 10 ? current_level + 1 : current_level);
	}
	else
	{
		ChangeState(game_level_load);
		
		graphics_engine->SetBackgroundImage(0);
		//level_class->EnableLevel();
		SetSpawnPoints();
		switch(current_level)
		{
			case 1:
				LoadLevel1();
				break;
			case 2:
				LoadLevel2();
				break;
			case 3:
				LoadLevel3();
				break;
			case 4:
				LoadLevel4();
				break;
			case 5:
				LoadLevel5();
				break;
			case 6:
				LoadLevel6();
				break;
			case 7:
				LoadLevel7();
				break;
			case 8:
				LoadLevel8();
				break;
			case 9:
				LoadLevel9();
				break;
			case 10:
				LoadLevel10();
				break;
			case 11:
				LoadLevel11();
				break;
			case 12:
				LoadLevel12();
				break;
			default:
				LoadLevel1();
				break;
		};		
	}
}

void Game::SetVideoQuality(int arg)
{
	video_quality = arg;
	level_class->SetQuality(video_quality);
}

int Game::GetVideoQuality()
{
	return video_quality;
}

void Game::Initialize()
{
	graphics_engine = GetHPTGraphicsEngine();
	
	resumeGame = false;
	transition = 0;
	video_quality = 0;
	level_class = new Level();
	graphics_engine->SetBackgroundColor(0,0,0);
	//blue12 = graphics_engine->CreateFont1();
	//blue12->LoadHFFFont(hptstrcat(path,("fonts//loading screen.hff")));

	/*NSString * path1 = [[NSBundle mainBundle] pathForResource:  @"gothic10" ofType: @"HFF"];
	const char *font1 = [path1 cStringUsingEncoding:1];
	path1 = [[NSBundle mainBundle] pathForResource:  @"gothic10r" ofType: @"HFF"];
	const char *font2 = [path1 cStringUsingEncoding:1];
	path1 = [[NSBundle mainBundle] pathForResource:  @"gothic12" ofType: @"HFF"];
	const char *font3 = [path1 cStringUsingEncoding:1];
	path1 = [[NSBundle mainBundle] pathForResource:  @"pGreen" ofType: @"HFF"];
	const char *font4 = [path1 cStringUsingEncoding:1];
	path1 = [[NSBundle mainBundle] pathForResource:  @"pRed" ofType: @"HFF"];
	const char *font5 = [path1 cStringUsingEncoding:1];
	path1 = [[NSBundle mainBundle] pathForResource:  @"pYellow" ofType: @"HFF"];
	const char *font6 = [path1 cStringUsingEncoding:1];
	path1 = [[NSBundle mainBundle] pathForResource:  @"pBlue" ofType: @"HFF"];
	const char *font7 = [path1 cStringUsingEncoding:1];
	path1 = [[NSBundle mainBundle] pathForResource:  @"bankgothic" ofType: @"HFF"];
	const char *font8 = [path1 cStringUsingEncoding:1];*/
	
	gothic10 = graphics_engine->CreateFont1();
	gothic10->LoadHFFFont("gothic10.HFF");
	gothic10r = graphics_engine->CreateFont1();
	gothic10r->LoadHFFFont("gothic10r.HFF");
	gothic12 = graphics_engine->CreateFont1();
	gothic12->LoadHFFFont("gothic12.HFF");
	fnt_green = graphics_engine->CreateFont1();
	fnt_green->LoadHFFFont("pGreen.HFF");
	fnt_red = graphics_engine->CreateFont1();
	fnt_red->LoadHFFFont("pRed.HFF");
	fnt_yellow = graphics_engine->CreateFont1();
	fnt_yellow->LoadHFFFont("pYellow.HFF");
	fnt_blue = graphics_engine->CreateFont1();
	fnt_blue->LoadHFFFont("pBlue.HFF");
	bankGothic = graphics_engine->CreateFont1();
	bankGothic->LoadHFFFont("bankgothic.HFF");
	graphics_engine->SetBackgroundImage(0);
	graphics_engine->SetTileBackgroundImage(0,0);
	graphics_engine->SetTileBackgroundImage(1,0);

	m_ProgressMeter = new ProgressMeter("Loading Blade of Betrayal\0", 18,50);
	m_ProgressMeter->ProgressMeterStart();
	m_ProgressMeter->Update();

#ifdef BOB_LITE
	//path1 = [[NSBundle mainBundle] pathForResource:  @"boblite" ofType: @"hgf"];
	char mainhgf[] = "boblite.hgf";
#else
	//path1 = [[NSBundle mainBundle] pathForResource:  @"main" ofType: @"HGF"];
	char mainhgf[] = "main.hgf";
#endif
	//const char *mainhgf = [path1 cStringUsingEncoding:1];
	graphics_engine->LoadHGF(mainhgf);
	
	// Initialize the menu
	menu = new Menu(&saved_games);
	
	current_level = 1;

	m_ProgressMeter->Update();

	ai_engine = GetAIEngine();
	ai_engine->SetPath(path);
	static_background = graphics_engine->CreateHPTBackground();
	level_end = graphics_engine->CreateHPTBackground();
	level_end->SetImage(CR::AssetList::spawn_flag_explosion);
	level_load = graphics_engine->CreateHPTBackground();
	
	pauseBackground = graphics_engine->CreateSprite1();
	pauseBackground->SetImage(CR::AssetList::pause_menu_base);
	pauseBackground->SetPositionAbsalute(240, 160);
	if(AppConfig::Instance().IsIpad())
		pauseBackground->SetDesignSize(480,320);

	cut_scenes = new Theater();
	cut_scenes->SetGame(this);

	m_ProgressMeter->Update();
	level_class->SetQuality(video_quality);
	key_pressed = true;
	new_level = false;
	saved_games.LoadFromDisk(DEFAULT_SAVE_FILE);
	gameStateManager.LoadFromDisk(DEFAULT_GAMESTATE_FILE);
	levelStateManager.LoadFromDisk(DEFAULT_LEVELSTATE_FILE);

	player = new Player();
	m_ProgressMeter->Update();
	player->SetRenderOffset(10);
	player->SetProjectileList(&projectile_list);
	level_class->SetPlayer(player);
	level_class->SetProjectileList(&projectile_list);

	//Draw Transition
	transition = new TransitionScreen(false);

	//transition->Begin(false);

	basic_bonus_points = 0;
	treasure_bonus_points = 0;
	special_bonus_points = 0;

	pixel_yellow_spr = graphics_engine->CreateSprite1();
	pixel_yellow_spr->SetImage(CR::AssetList::Yellow_Pixel);
	pixel_black_spr = graphics_engine->CreateSprite1();
	pixel_black_spr->SetImage(CR::AssetList::Black_Pixel);
	pixel_red_spr = graphics_engine->CreateSprite1();
	pixel_red_spr->SetImage(CR::AssetList::Red_Pixel);
	pixel_spr = graphics_engine->CreateSprite1();	
	//pixel_spr->SetImage(CR::AssetList::Pixel);	
	pixel_spr->SetImage(CR::AssetList::Bubbles);
	pixel_green_spr = graphics_engine->CreateSprite1();
	pixel_green_spr->SetImage(CR::AssetList::Green_Pixel);
	pixel_blue_spr = graphics_engine->CreateSprite1();
	pixel_blue_spr->SetImage(CR::AssetList::Blue_Pixel);
	
	nframes = 0;
	frame_rate_count = 0;
	frame_rate = 0;
	currenttime = s3eTimerGetMs();
	starttime = s3eTimerGetMs();
	m_ProgressMeter->Finish();
	m_ProgressMeter->ProgressMeterStop();
	delete m_ProgressMeter;
	
	// Input Controls
	m_analogAdjust = 94*graphics_engine->GetAspectRatioMod() - 94;
	analogStick = new Input_Analog(CR::AssetList::thumb_active_area);
	analogStick->SetPosition(75, 245-m_analogAdjust);
	if(AppConfig::Instance().IsIpad())
		analogStick->DesignSize(105,94+m_analogAdjust);
	analogStick->SetRadius(100+m_analogAdjust, 0, 75); // was 69, 10
	input_objects.push_back(analogStick);
	
	m_attackButtonAdjust = 49*graphics_engine->GetAspectRatioMod() - 49;
	if(AppConfig::Instance().IsIpad())
		attackButton = new Input_Button(true);
	else
		attackButton = new Input_Button();
	attackButton->SetSpriteAndBounds(320, 243-m_attackButtonAdjust, CR::AssetList::button_attack);
	if(AppConfig::Instance().IsIpad())
		attackButton->DesignSize(55,49+m_attackButtonAdjust);
	input_objects.push_back(attackButton);
	
	m_specialAttackButtonAdjust = 20*graphics_engine->GetAspectRatioMod() - 20;
	if(AppConfig::Instance().IsIpad())
		specialAttackButton = new Input_Button(true);
	else
		specialAttackButton = new Input_Button();
	specialAttackButton->SetSpriteAndBounds(320, 163-m_specialAttackButtonAdjust-m_attackButtonAdjust,
		CR::AssetList::SpecialAttackButton);
	if(AppConfig::Instance().IsIpad())
		specialAttackButton->DesignSize(51,20+m_specialAttackButtonAdjust);
	//else
	//	specialAttackButton->SetButtonBounds(317,161,57,24);
	input_objects.push_back(specialAttackButton);

	if(AppConfig::Instance().IsIpad())
		grenadeButton = new Input_Button(true);
	else
		grenadeButton = new Input_Button();
	grenadeButton->SetSpriteAndBounds(400, 163-m_attackButtonAdjust-m_specialAttackButtonAdjust, 
		CR::AssetList::button_grenade);
	if(AppConfig::Instance().IsIpad())
		grenadeButton->DesignSize(51,20+m_specialAttackButtonAdjust);
	input_objects.push_back(grenadeButton);
	
	if(AppConfig::Instance().IsIpad())
		jumpButton = new Input_Button(true);
	else
		jumpButton = new Input_Button();
	jumpButton->SetSpriteAndBounds(400, 243-m_attackButtonAdjust, CR::AssetList::button_jump);
	if(AppConfig::Instance().IsIpad())
		jumpButton->DesignSize(55,49+m_attackButtonAdjust);
	input_objects.push_back(jumpButton);
	
	if(AppConfig::Instance().IsIpad())
		pauseButton = new Input_Button(true);
	else
		pauseButton = new Input_Button();
	if(AppConfig::Instance().IsIpad())
		pauseButton->SetSpriteAndBounds(439, 3, CR::AssetList::button_pause);
	else
		pauseButton->SetSpriteAndBounds(3, 30, CR::AssetList::button_pause);
	pauseButton->OnClicked += Delegate(this, &Game::OnClickedPauseButton);
	if(AppConfig::Instance().IsIpad())
		pauseButton->DesignSize(39,30);
	input_objects.push_back(pauseButton);
	
	weaponSelect = new Input_WeaponSelect(player);
	weaponSelect->SetPosition(445, 30, 20);
	input_objects.push_back(weaponSelect);
	
	resumeGameButton = new Input_Button();
	
	if(AppConfig::Instance().IsIpad())
	{
		resumeGameButton->SetSpriteAndBounds(132, 115, CR::AssetList::ButtonResumePause);
		resumeGameButton->DesignSize(217,50);
	}
	else
		resumeGameButton->SetSpriteAndBounds(132, 115, CR::AssetList::ButtonResumePause);
	resumeGameButton->OnClicked += Delegate(this, &Game::OnClickedResumeGameButton);
	input_objects.push_back(resumeGameButton);
	resumeGameButton->Disabled(true);
	resumeGameButton->SetSoundOn(true);
	
	quitGameButton = new Input_Button();
	if(AppConfig::Instance().IsIpad())
	{
		quitGameButton->SetSpriteAndBounds(132, 182, CR::AssetList::button_main_menu);
		quitGameButton->DesignSize(217,50);
	}
	else
		quitGameButton->SetSpriteAndBounds(132, 182, CR::AssetList::button_main_menu);
	quitGameButton->OnClicked += Delegate(this, &Game::OnClickedQuitGameButton);
	input_objects.push_back(quitGameButton);
	quitGameButton->Disabled(true);
	quitGameButton->SetSoundOn(true);
	
	loadScreenButton = new Input_Button();
	loadScreenButton->SetButtonBounds(0, 0, 480, 320);
	loadScreenButton->OnClicked += Delegate(this,&Game::OnClickedLevelLoadScreen);
	loadScreenButton->Disabled(true);
	input_objects.push_back(loadScreenButton);		
	
	deathScreenButton = new Input_Button();
	deathScreenButton->SetButtonBounds(0, 0, 480, 320);
	deathScreenButton->OnClicked += Delegate(this,&Game::OnClickedDeathScreen);
	deathScreenButton->Disabled(true);
	input_objects.push_back(deathScreenButton);
	
	endScreenButton = new Input_Button();
	endScreenButton->SetButtonBounds(0, 0, 480, 320);
	endScreenButton->OnClicked += Delegate(this,&Game::OnEndScreenClick);
	endScreenButton->Disabled(true);
	input_objects.push_back(endScreenButton);	
	
	healthMeterBar = new MeterBar(Red);
	if(AppConfig::Instance().IsIpad())		
		healthMeterBar->SetPosition(7, 7);
	else
		healthMeterBar->SetPosition(6, 10);
	
	energyMeterBar = new MeterBar(Green);
	if(AppConfig::Instance().IsIpad())		
		energyMeterBar->SetPosition(7, 18);
	else
		energyMeterBar->SetPosition(6, 22);
	
	timeDisplay = new TimeDisplay();
	timeDisplay->SetFont(gothic12);
	if(AppConfig::Instance().IsIpad())		
		timeDisplay->SetPosition(16, 31);
	else
		timeDisplay->SetPosition(47, 37);
	
//soundPlayer->Mute(!saved_games.GetSettingsInfo().soundOn);
	ISound::Instance().MuteMusic(!saved_games.GetSettingsInfo().musicOn);
//	musicPlayer->Mute(!saved_games.GetSettingsInfo().musicOn);
}

void Game::Resume()
{
	resumeGame = true;
	StartGame(gameStateManager.GetGameStateInfo().level, !gameStateManager.GetGameStateInfo().inTheater);
}

void Game::EndLevel()
{	
//musicPlayer->Stop();
	
	SaveLevelState();
	RequestGState = game_ingame;
	ChangeState(game_level_end);
	m_levelEndFSM.Reset();
}

void Game::PlayerDeath()
{
	SaveLevelState();
	
	ai_engine->Pause();
	ai_engine->RemoveAll();
	list<Projectile*>::iterator temppl = projectile_list.begin();
	while(temppl != projectile_list.end())
	{
		delete (*temppl);
		temppl = projectile_list.erase(temppl);
	}
	
	Enemy* tenemy;
	//bool deadEnemies[enemys.size()];
	//int lifeEnemies[enemys.size()];
	for(int count = 0;count < enemys.size();count++)
	{
		tenemy = enemys[count];
		//deadEnemies[count] = tenemy->GetDead();
		//lifeEnemies[count] = tenemy->GetCurLife();
		delete tenemy;
	}
	enemys.clear();
	
	player->ResetIPlayer();
	player->moveStop();
	//MessageBox(0,("done with loop"),("error"),MB_OK);

	//musicPlayer->Stop();
	/*player->SubtractLife();
	if(player->GetNumLives() < 0)
	{
//		if((music != 0) && (mute_music != true)) music->HPTStopSound();
		graphics_engine->SetBackgroundImage(0);
		ChangeState(game_death);
//		ai_engine->Pause();
		level_class->DisableLevel();
		disable_input = true;
		//input_engine->PollInput();
		disable_input = false;

		LevelTransition(false);

		return;
	}*/
	new_level = true;
	//current_level++;
	/*SaveGame temps;
	temps.energy = player->GetEnergyLevel();
	temps.energy_max = player->GetMaxEnergyLevel();
	temps.health = player->GetCurLife();
	temps.health_bars = player->GetNumLifeBars();
	temps.level = current_level;
	temps.weapon = player->GetAttack();
	GetLocalTime(&(temps.saved_time));
	saved_games.SaveaGame(current_level,&temps);
	*/
//	ai_engine->Pause();
	
	
	player->moveStop();
	player->SetLevel(level_class);
	player->setGravityConstant(500.0f);
	player->SetSprite(0,CR::AssetList::Player_Spencer);
	player->ReSpawn();
	level_class->LoadObjects(true);
	LevelTransition(false);
		
	// restore enemy states (only if the player is not in a boss battle)
	// disabled - restore this code if there is time to save dead enemies based on checkpoints
	/*
	if (!player->inBossBattle)
	{
		for(int count = 0;count < enemys.size();count++)
		{
			tenemy = enemys[count];
			
			if (deadEnemies[count] == true)
				tenemy->forceDeath();
			else
				tenemy->SetCurLife(lifeEnemies[count]);
		}
	}
	*/
	
	// Enemy dead states
	//Enemy* tenemy;
	level_class->ResetToSpawnPointData(m_SpawnPointInfo);
	
	for(int count = 0;count < enemys.size();count++)
	{
		tenemy = enemys[count];
		if (m_SpawnPointInfo.enemyDeadStates[count] == true)
			tenemy->forceDeath();
	}
	
	//InitializeLevelState();
		
	ai_engine->Resume();

	tallyNumBeacons = 0;	
}


void Game::ProcessDeath()
{
	graphics_engine->BeginFrame();
	m_playerDeathFSM();
	graphics_engine->EndFrame();
	return;
	
	HPTSprite1* game_over;
	game_over = graphics_engine->CreateSprite1();
	game_over->SetImage(CR::AssetList::Game_Over_Screen);
	game_over->SetPositionAbsalute(120, 160);
//	graphics_engine->SetBackgroundColor(255,0,0);
	graphics_engine->BeginFrame();
	game_over->Render();
	transition->Update();

//	graphics_engine->Position(50,100);
//	(*graphics_engine) << blue12 << "Your Dead, \nSucks to be you";
//	(*graphics_engine) << blue12 << "\n\nPress any key to continue";
	graphics_engine->EndFrame();
	game_over->Release();
}

void Game::PlayerDeathState1()
{	
	PlayerDeath();
	
	transition->Begin(true);
	m_playerDeathFSM++;
	weaponSelect->Disabled(true);
}

void Game::PlayerDeathState2()
{
	transition->Update();
	transition->Render();
	
	if(!transition->sActive) 
	{
		loadingButton = graphics_engine->CreateSprite1();
		loadingButton->SetImage(AssetList::Transition_Banner_Bottom);
		loadingButton->SetFrameSet(1);
		loadingButton->SetPositionAbsalute(285, 280);
		if(AppConfig::Instance().IsIpad())
			loadingButton->SetDesignSize(444,65);
		
		loadingLevelTags = graphics_engine->CreateSprite1();
		loadingLevelTags->SetImage(AssetList::Level_Load_Tags);
		if(current_level == 10)
			loadingLevelTags->SetFrameSet(10);
		else
			loadingLevelTags->SetFrameSet(current_level-1);
		loadingLevelTags->SetPositionAbsalute(180, 34);
		if(AppConfig::Instance().IsIpad())
			loadingLevelTags->SetDesignSize(334,18);
		
		loadingObjectiveTags = graphics_engine->CreateSprite1();
		loadingObjectiveTags->SetImage(AssetList::Level_Objective_Tags);
		loadingObjectiveTags->SetFrameSet(current_level-1);
		loadingObjectiveTags->SetPositionAbsalute(130, 80);
		if(AppConfig::Instance().IsIpad())
		{
			loadingObjectiveTags->SetPositionAbsalute(130, 84);
			loadingObjectiveTags->SetDesignSize(232,44);
		}
		
		//tallyFont = graphics_engine->CreateSprite1();
		//tallyFont->SetImage(AssetList::Tally_Font);
		
		
		if(current_level == 10)
			endTimeDisplay = NULL;
		else
		{
			endTimeDisplay = new TimeDisplay();
			endTimeDisplay->SetFont(bankGothic);
			endTimeDisplay->SetShowFrontZero(false);
			if(AppConfig::Instance().IsIpad())
				endTimeDisplay->SetPosition(425, 216);
			else
				endTimeDisplay->SetPosition(425, 206);
			int lev = current_level;
			if(current_level >= 13)
				lev -= 6;
			endTimeDisplay->SetTime(levelAchievementTimes[lev-1]);
		}
		
		awardInfo = graphics_engine->CreateSprite1();
		awardInfo->SetImage(AssetList::Award_Info);
		if(current_level == 10)
			awardInfo->SetFrameSet(2);
		else
		{
			if(AppConfig::Instance().IsIpad())
				awardInfo->SetFrameSet(current_level%3==0?0:1);
			else
				awardInfo->SetFrameSet(current_level%3==0?1:0);
		}
		if(AppConfig::Instance().IsIpad())
		{
			if(current_level == 10)
				awardInfo->SetPositionAbsalute(260, 170);
			else
				awardInfo->SetPositionAbsalute(310, 170);
		}
		else
		{
			if(current_level == 10)
				awardInfo->SetPositionAbsalute(260, 160);
			else
				awardInfo->SetPositionAbsalute(310, 160);
		}
		if(AppConfig::Instance().IsIpad())
			awardInfo->SetDesignSize(325,119);
		
		deathScreenButton->Disabled(false);
		m_playerDeathFSM++;
	}
}

void Game::PlayerDeathState3()
{	
	transition->Render();
	
	loadingButton->Render();
	loadingLevelTags->Render();
	loadingObjectiveTags->Render();
	awardInfo->Render();
	if(endTimeDisplay)
		endTimeDisplay->Render();
}

void Game::PlayerDeathState4()
{
	loadingButton->Release();
	loadingButton = NULL;
	loadingLevelTags->Release();
	loadingLevelTags = NULL;
	loadingObjectiveTags->Release();
	loadingObjectiveTags = NULL;
	awardInfo->Release();
	awardInfo = NULL;
	delete endTimeDisplay;
	endTimeDisplay = NULL;

	deathScreenButton->Disabled(true);
	transition->Render();
	transition->Begin(false);
	m_playerDeathFSM.Reset();
	ChangeState(game_ingame);
}

void Game::ProcessEndLevel()
{	
	graphics_engine->BeginFrame();
	graphics_engine->DrawTiles1();
	graphics_engine->DrawTiles2();
	m_levelEndFSM();
	graphics_engine->EndFrame();
}

void Game::LevelEndState1()
{		
	transition->Begin(true);

	//player->moveStop();
	player->player_spr->SetAutoAnimate(false);
	
	m_levelEndFSM++;
	weaponSelect->Disabled(true);
}

void Game::LevelEndState2()
{	
	//level_class->SetTime(time);

	level_class->Render();
	player->Render();	
	
	int tempi = enemys.size();
	for(int count = 0;count < tempi;count++)
	{		
		if(enemys[count]->GetStateFlags()->S_ACTIVE)
		{
			enemys[count]->moveStop();
			enemys[count]->Render();
		}
	}	
	
	RenderHUD();
	
	transition->Update();
	if(!transition->sActive)
		m_levelEndFSM++;
		
}

void Game::LevelEndState3()
{	
	list<Projectile*>::iterator temppl = projectile_list.begin();
	while(temppl != projectile_list.end())
	{
		delete (*temppl);
		temppl = projectile_list.erase(temppl);
	}
	
	tallyNumEnemies = 0;
	tallyNumDeadEnemies = 0;
	Enemy* tenemy;
	for(int count = 0;count < enemys.size();count++)
	{
		tenemy = enemys[count];
		
		// count the tally
		if (tenemy->IsEnemy())
		{
			tallyNumEnemies++;
			if (tenemy->GetDead())
				tallyNumDeadEnemies++;
		}
		
		delete tenemy;
	}
	enemys.clear();
	
	tallyNumObjects = 0;
	tallyNumGotObjects = 0;
	list<Object*> objects = level_class->GetObjectList();
	for (list<Object*>::iterator i = objects.begin(); i != objects.end(); i++)
	{
		if ((*i)->IsObject())
		{
			tallyNumObjects++;
			if ((*i)->GotObject())
				tallyNumGotObjects++;
		}
	}
	
	level_class->DisableLevel();
	level_class->UnloadLevel();
	
	if((current_level % 3) == 0)
	{
		//basic_bonus_points = 0;
		//treasure_bonus_points = 0;
		//special_bonus_points = 0;
		player->SetEnergyLevel(player->GetMaxEnergyLevel());
		player->SetCurLife(player->GetNumLifeBars()*5);
		
	}
	
	
	//player->moveStop();
	
	new_level = true;
	current_level++;
	
	/*
	SaveGameInfo temps;
	temps.energy = player->GetEnergyLevel();
	temps.energy_max = player->GetMaxEnergyLevel();
	temps.health = player->GetCurLife();
	temps.health_bars = player->GetNumLifeBars();
	temps.level = current_level;
	//temps.score = player->GetScore();
	//temps.nlives = player->GetNumLives();
	//		temps.items = player->GetTreasureItems();
	temps.basic_weapon = player->GetBasicAttackDamage();
	
	temps.weapons[0] = player->IsWeaponAvailable(0);
	temps.weapons[1] = player->IsWeaponAvailable(1);
	temps.weapons[2] = player->IsWeaponAvailable(2);
	temps.weapons[3] = player->IsWeaponAvailable(3);
	temps.weapons[4] = player->IsWeaponAvailable(4);
	temps.weapons[5] = player->IsWeaponAvailable(5);
	temps.grenades = player->GetGrenades();
	
	//	GetLocalTime(&(temps.saved_time));
	saved_games.SaveGame(&temps);
	saved_games.SaveToDisk(DEFAULT_SAVE_FILE);
	*/
	
	// determine achievements
	if (GotBronzeAchievement())
		player->levelAwards[current_level-2].bronzeAchievement = true;
	
	if (GotSilverAchievement())
		player->levelAwards[current_level-2].silverAchievement = true;
	
	if (GotGoldAchievement())
		player->levelAwards[current_level-2].goldAchievement = true;

	// HACK to unlock bonus content based on achievment count
	int totalAchievements = 0;
	for (int i = 0; i < 12; i++)
	{
		if (player->levelAwards[i].bronzeAchievement)
			totalAchievements++;
		if (player->levelAwards[i].silverAchievement)
			totalAchievements++;
		if (player->levelAwards[i].goldAchievement)
			totalAchievements++;
	}
	
	// TODO: Add some kind of bonus notification, so the player knows they got a bonus
	if (totalAchievements >= 5)
		player->bonusAwards.BonusLevel1 = true;
	if (totalAchievements >= 15)
		player->bonusAwards.BonusLevel2 = true;
	if (totalAchievements >= 20)
		player->bonusAwards.BonusLevel3 = true;

	

	SaveGame();
	
	level_class->DisableLevel();
	//level_end->SetImage(54);
	graphics_engine->SetBackgroundImage(0);
	
	transition->Render();
	//if(!transition->sActive)
	
	m_levelEndFSM++;
	endScreenButton->Disabled(false);
	
	loadingButton = graphics_engine->CreateSprite1();
	loadingButton->SetImage(AssetList::Transition_Banner_Bottom);
	loadingButton->SetFrameSet(2);
	loadingButton->SetPositionAbsalute(285, 280);
	loadingButton->Render();	
	if(AppConfig::Instance().IsIpad())
		loadingButton->SetDesignSize(444,65);
	
	loadingLevelTags = graphics_engine->CreateSprite1();
	loadingLevelTags->SetImage(AssetList::Level_Load_Tags);
	loadingLevelTags->SetFrameSet(9);
	loadingLevelTags->SetPositionAbsalute(180, 34);
	loadingLevelTags->Render();	
	if(AppConfig::Instance().IsIpad())
		loadingLevelTags->SetDesignSize(334,18);
	
	endTally = graphics_engine->CreateSprite1();
	if(current_level == 11)
	{
		endTally->SetImage(AssetList::End_Level_Tally_03);
		endTally->SetPositionAbsalute(150, 150);
		if(AppConfig::Instance().IsIpad())
			endTally->SetDesignSize(180,102);
	}
	else if(!player->inBossBattle)
	{
		endTally->SetImage(AssetList::End_Level_Tally_01);
		endTally->SetPositionAbsalute(150, 150);
		if(AppConfig::Instance().IsIpad())
			endTally->SetDesignSize(160,106);
	}
	else
	{
		endTally->SetImage(AssetList::End_Level_Tally_02);
		endTally->SetPositionAbsalute(150, 150);
		if(AppConfig::Instance().IsIpad())
			endTally->SetDesignSize(181,103);
	}
	
	endTally->SetFrameSet(0);
	endTally->Render();
	
	endTimeDisplay = new TimeDisplay();
	endTimeDisplay->SetFont(bankGothic);
	endTimeDisplay->SetShowFrontZero(false);
	endTimeDisplay->SetPosition(280, 198);
	endTimeDisplay->BeginCountUpAnimation(this->m_playTime);
	//endTimeDisplay->SetTime(this->m_playTime);
	//endTimeDisplay->Render();
	
	medalsSprite = graphics_engine->CreateSprite1();
	medalsSprite->SetImage(CR::AssetList::Level_Tally_Medal_Icons);
	if(AppConfig::Instance().IsIpad())
		medalsSprite->SetDesignSize(26,26);
	bronzeAwardReceived = false;
	silverAwardReceived = false;
	goldAwardReceived = false;
		
	itemsTallyDisplay = NULL;
	if(current_level != 11)
	{
		itemsTallyDisplay = new TallyDisplay();
		itemsTallyDisplay->SetFont(bankGothic);
		itemsTallyDisplay->SetPosition(280, 109);
		itemsTallyDisplay->SetTally(tallyNumGotObjects, tallyNumObjects);
		
		if (!player->inBossBattle)
			itemsTallyDisplay->BeginCountUpAnimation();
	}
	
	if(current_level == 11)
	{
		enemiesTallyDisplay = new TallyDisplay();
		enemiesTallyDisplay->SetFont(bankGothic);
		enemiesTallyDisplay->SetPosition(280, 150);
		enemiesTallyDisplay->SetTally(tallyNumBeacons, 10);
	}
	else
	{
		enemiesTallyDisplay = new TallyDisplay();
		enemiesTallyDisplay->SetFont(bankGothic);
		enemiesTallyDisplay->SetPosition(280, 154);
		enemiesTallyDisplay->SetTally(tallyNumDeadEnemies, tallyNumEnemies);
	}
	
	if (!player->inBossBattle)
		enemiesTallyDisplay->BeginCountUpAnimation();
}

void Game::LevelEndState4()
{	
	transition->Render();
	loadingButton->Render();
	loadingLevelTags->Render();
	endTally->Render();

	// items
	if (!player->inBossBattle)
	{
		if(itemsTallyDisplay)
		{
			itemsTallyDisplay->Update(time);
			itemsTallyDisplay->Render();
			
			if (itemsTallyDisplay->DoneAnimating())
			{
				enemiesTallyDisplay->Update(time);
				
				if (!goldAwardReceived &&
					GotGoldAchievement())
				{
					goldAwardReceived = true;
					m_soundBang->Play();
	//			soundPlayer->PlaySound(bang00);
				}
				else if (goldAwardReceived)
				{
					medalsSprite->SetFrameSet(0);
					medalsSprite->SetPositionAbsalute(345, 109);
					medalsSprite->Render();
				}
			}
		}
		else
			enemiesTallyDisplay->Update(time);
	
		// enemies
		enemiesTallyDisplay->Render();
	
		if(current_level == 11)
		{
			if(enemiesTallyDisplay->CurrentTally() >= 10)
				medalsSprite->SetFrameSet(2);
			else if(enemiesTallyDisplay->CurrentTally() >= 6)
				medalsSprite->SetFrameSet(1);
			else 
				medalsSprite->SetFrameSet(0);

			if(enemiesTallyDisplay->CurrentTally() >= 3)
			{
				medalsSprite->SetPositionAbsalute(345, 154);
				medalsSprite->Render();
			}
		}
		if (enemiesTallyDisplay->DoneAnimating() && (!itemsTallyDisplay || itemsTallyDisplay->DoneAnimating()))
		{
			if(endTimeDisplay)
				endTimeDisplay->Update(time);
			
			if(current_level == 11)
			{
				if (!silverAwardReceived)
				{
					silverAwardReceived = true;
					m_soundBang->Play();
	//			soundPlayer->PlaySound(bang00);
				}
				else if (silverAwardReceived)
				{
					if(GotGoldAchievement())
						medalsSprite->SetFrameSet(2);
					else if(GotSilverAchievement())
						medalsSprite->SetFrameSet(1);
					else 
						medalsSprite->SetFrameSet(0);

					if(GotBronzeAchievement())
					{
						medalsSprite->SetPositionAbsalute(345, 154);
						medalsSprite->Render();
					}
				}
			}
			else
			{
				if (!silverAwardReceived &&
					GotSilverAchievement())
				{
					silverAwardReceived = true;
					m_soundBang->Play();
	//			soundPlayer->PlaySound(bang00);
				}
				else if (silverAwardReceived)
				{
					medalsSprite->SetFrameSet(1);
					medalsSprite->SetPositionAbsalute(345, 154);
					medalsSprite->Render();
				}
			}
		}
	}
	else
	{
		if (GotGoldAchievement())
		{
			graphics_engine->Position(280, 109);
			(*graphics_engine) << bankGothic << "PASS";
			medalsSprite->SetFrameSet(0);
			medalsSprite->SetPositionAbsalute(345, 109);
			medalsSprite->Render();
		}
		else
		{
			graphics_engine->Position(280, 109);
			(*graphics_engine) << bankGothic << "FAIL";
		}
		if (GotSilverAchievement())
		{
			graphics_engine->Position(280, 154);
			(*graphics_engine) << bankGothic << "PASS";
			
			medalsSprite->SetFrameSet(1);
			medalsSprite->SetPositionAbsalute(345, 154);
			medalsSprite->Render();
		}
		else
		{
			graphics_engine->Position(280, 154);
			(*graphics_engine) << bankGothic << "FAIL";
		}
		
		if(endTimeDisplay)
			endTimeDisplay->Update(time);
	}
	
	// level time
	if(endTimeDisplay)
		endTimeDisplay->Render();
	
	if (current_level != 11 && endTimeDisplay && endTimeDisplay->DoneAnimating() && 
		enemiesTallyDisplay->DoneAnimating() && (!itemsTallyDisplay || itemsTallyDisplay->DoneAnimating()))
	{
		if (!bronzeAwardReceived &&
			GotBronzeAchievement())
		{
			bronzeAwardReceived = true;
			m_soundBang->Play();
//		soundPlayer->PlaySound(bang00);
		}
		else if (bronzeAwardReceived && current_level <= 10)
		{
			medalsSprite->SetFrameSet(2);
			medalsSprite->SetPositionAbsalute(345, 198);
			medalsSprite->Render();
		}
	}
	
	// Enemies
	//endTallyDisplay->SetPosition(380, 140);
	//endTallyDisplay->SetTally(10, 48);
	//endTallyDisplay->Render();
	
	//graphics_engine->Position(50,10);
	//(*graphics_engine) << gothic12 << "Weapon Upgrade:\n";

	/*graphics_engine->Position(140,10);
	(*graphics_engine) << gothic10 << "Level " << current_level-1 << ": " << gothic10r << "Mission Success!";
	graphics_engine->Position(140,300);
	(*graphics_engine) << gothic10r << "Click Screen to continue";*/
	
	//transition->Update();
}

void Game::LevelEndState5()
{	
	endScreenButton->Disabled(true);
	loadingButton->Release();
	loadingButton = NULL;
	loadingLevelTags->Release();
	loadingLevelTags = NULL;
	endTally->Release();
	endTally = NULL;

	medalsSprite->Release();
	medalsSprite = NULL;
	
	delete endTimeDisplay;
	endTimeDisplay = NULL;
	
	delete itemsTallyDisplay;
	itemsTallyDisplay = NULL;
	
	delete enemiesTallyDisplay;
	enemiesTallyDisplay = NULL;
	
	graphics_engine->SetBackgroundImage(0);
	transition->Begin(false);	
	transition->Update();
	
	player->ResetItems();
#ifdef BOB_LITE
	if (current_level >= 2)
	{
		RequestGState = game_menu;
		
		// save the game, player beat the game
		//SaveGame();
		SaveGameState(false);
		//SaveLevelState();
	}
	else
		RequestGState = game_ingame;
#else
	if (current_level >= 10)
	{
		RequestGState = game_menu;
		
		// save the game, player beat the game
		//SaveGame();
		SaveGameState(false);
		//SaveLevelState();
	}
	else
		RequestGState = game_ingame;
#endif
	
	if(current_level == 11)
	{
		menu->RestartMenu(this,gameStateManager.GetGameStateInfo().canResume,&saved_games);
		menu->ChangeState(menu_mission_select);
		ChangeState(game_menu);
	}
	else
		ChangeState(game_movie);
	level_class->DisableLevel();
	//music->Stop();
	//music->ChangeSong(1);
	//music->Play();
	if(current_level != 11)
		cut_scenes->StartMovie(current_level);
}

void Game::EndLevelScreen()
{
	HPTSprite1* endlevel = graphics_engine->CreateSprite1();
//	graphics_engine->LoadHGF(1,hptstrcat(path,("graphics//endlevel.hgf")));
	endlevel->SetImage(CR::AssetList::Door_dimmed);
//	endlevel->AutoStopAnimate();
	endlevel->SetPositionAbsalute(240, 160);
//	level_end->SetImage(1,0);
//	level_end->SetImage(0,52);
	level_class->DisableLevel();
	graphics_engine->SetBackgroundImage(0);
//	graphics_engine->SetBackgroundImage(level_end);
	graphics_engine->SetClearScreen(false);
	transition->Begin(true);

	while(transition->sActive)
	{
		graphics_engine->BeginFrame();
		transition->Update();
		graphics_engine->EndFrame();
	}
	
	key_pressed = false;
//	bool bonus_calc = false;
	/*if(((current_level-1) % 3) == 0)
	{
//		level_class->DisableLevel();
//		graphics_engine->SetBackgroundImage(new_weapon_back);
		graphics_engine->SetClipper(0);
	
		graphics_engine->BeginFrame();
		endlevel->Render();
		if(current_level == 4)
			new_weapon_icon->SetImage(65);
		if(current_level == 7)
			new_weapon_icon->SetImage(66);
		if(current_level == 10)
			new_weapon_icon->SetImage(67);
		if(current_level == 13)
			new_weapon_icon->SetImage(69);
		new_weapon_icon->SetPositionAbsalute(120,120);
		new_weapon_icon->Render();
		graphics_engine->Position(50,10);
		(*graphics_engine) << gothic12 << "Weapon Upgrade:\n";
		graphics_engine->Position(60,50);
		graphics_engine->SetClipper(0);

		if(current_level == 4)
		{
			(*graphics_engine) << gothic12 << "    Flame Arch";
			graphics_engine->Position(30,165);
			(*graphics_engine) << gothic10
				<< "Spencer can now lash out\n"
				   "a fire whip from his sword\n"
				   "that throws balls of fire\n"
				   "in all directions.\n\n";
				   
		
		}
		if(current_level == 7)
		{
			graphics_engine->Position(48,50);
			(*graphics_engine) << gothic12 << "    Flak Grenade";
			graphics_engine->Position(30,165);
			(*graphics_engine) << gothic10
				<<  "Spencer can now throw\n"
				    "grenades at his enemies.\n"
					"The flak produced by the\n"
					"explosion will damage any\n"
					"enemies within range.\n\n";
	
//					"Description: Spencer can\n"
//				   "  now lash out a fire whip\n"
//				   "  from his sword that throws\n"
//				   "  flaming projectiles in\n"
//				   "  all directions.\n\n";
				   
		
		}
		if(current_level == 10)
		{
			graphics_engine->Position(60,50);
			(*graphics_engine) << gothic12 << "   Plasma Wave";
			graphics_engine->Position(30,165);
			(*graphics_engine) << gothic10
				<< "Spencer emits a wave of\n"
				   "energy from his blade that\n"
				   "has a devistating effect on\n"
				   "anything in its path.\n\n";
		
//				"Description: Spencer can\n"
//				   "  now unleash a devastating\n"
//				   "  wave of energy from his\n"
//				   "  sword that moves through\n"
//				   "  everything in it's path.\n\n";
				  
		
		}
		if(current_level == 13)
		{
			graphics_engine->Position(60,50);
			(*graphics_engine) << gothic12 << "  The Red Quake";
			graphics_engine->Position(30,165);
			(*graphics_engine) << gothic10
				<< "Spencer can now create a\n"
				   "devastating localized\n"
				   "earthquake, which will\n"
				   "damage all enemies within\n"
				   "an area around Spencer for\n"
				   "a short period of time.\n\n";
				   
		
		}
		graphics_engine->Position(40,300);
		(*graphics_engine) << gothic10 << "Press attack to continue";
	
		graphics_engine->EndFrame();
		key_pressed = false;
		while(!key_pressed)
		{
	//		MessageBox(0,("done with loop"),("error"),MB_OK);
			input_engine->PollInput();
		}
		if(current_level == 10)
		{
			graphics_engine->BeginFrame();
			endlevel->Render();
			new_weapon_icon->SetImage(68);
			new_weapon_icon->SetPositionAbsalute(120,120);
			new_weapon_icon->Render();
			graphics_engine->Position(50,10);
			(*graphics_engine) << gothic12 << "Special Upgrade:\n";
			graphics_engine->Position(60,50);
			graphics_engine->SetClipper(0);

			graphics_engine->Position(65,50);
			(*graphics_engine) << gothic12 << "    Respirator";
			graphics_engine->Position(30,165);
			(*graphics_engine) << gothic10
				<< "Spencer can now survive for\n"
				   "an extended time while\n"
				   "under water or in hazardous\n"
				   "chemical areas.\n\n";
			graphics_engine->Position(40,300);
			(*graphics_engine) << gothic10 << "Press attack to continue";
		
			graphics_engine->EndFrame();
			key_pressed = false;
			while(!key_pressed)
			{
				input_engine->PollInput();
			}

		}
	}
*/
	//key_pressed = false;
	key_pressed = true;
	while(!key_pressed)
	{
	graphics_engine->BeginFrame();
	endlevel->Render();
	graphics_engine->Position(140,10);
	(*graphics_engine) << gothic10 << "Level " << current_level-1 << ": " << gothic10r << "Mission Success!";

	//graphics_engine->Position(140,80);
	//(*graphics_engine) << gothic10 << "\nCurrent Score: " << gothic10r<< level_end_score;
	/*if(((current_level-1) % 3) != 0)
	{
		(*graphics_engine) << gothic10 << "\n\nBasic items:              " << gothic10r << player->GetBasicItems() << gothic10 << "/" << gothic10r << number_basic_items[current_level-2];
		(*graphics_engine) << gothic10 << "\n   Bonus: " << gothic10r << (int)(basic_bonus_points * player->GetModifier());//"5000";
		//(*graphics_engine) << gothic10 << "\nNagolaCorp Docs:     " << gothic10r << player->GetTreasureItems() << gothic10 << "/"  << gothic10r << number_treasure_items[current_level-2];
		//(*graphics_engine) << gothic10 << "\n   Bonus: " << gothic10r << (int)(treasure_bonus_points * player->GetModifier());// "10000";
		(*graphics_engine) << gothic10 << "\nSpecial items:           " << gothic10r << player->GetSpecialItems() << gothic10 << "/" << gothic10r << number_special_items[current_level-2];
		(*graphics_engine) << gothic10 << "\n   Bonus: " << gothic10r << (int)(special_bonus_points * player->GetModifier());//"15000";
	}
	else
		(*graphics_engine) << "\n\n\n\n\n\n\n";
*/
	//(*graphics_engine) << gothic10 << "\nCombat Efficiency:    " << gothic10r << (int)level_end_efficiency << "%"; 
	//(*graphics_engine) << gothic10 << "\n   Bonus: " << gothic10r << (int)(combat_bonus * player->GetModifier());
	//(*graphics_engine) << gothic10 << "\nTotal bonus:              " << gothic10r << (int)(bonus_points * player->GetModifier());
	//(*graphics_engine) << gothic10 << "\nNew Score: " << gothic10r << player->GetScore();
	
	graphics_engine->Position(140,300);
	(*graphics_engine) << gothic10r << "Click Screen to continue";
	graphics_engine->EndFrame();

//		MessageBox(0,("done with loop"),("error"),MB_OK);
		//input_engine->PollInput();
		//Sleep(50);
	}

	graphics_engine->SetClearScreen(true);

	level_class->EnableLevel();
}

bool Game::GetEnableStatus()
{
	return true;
//	return enable_status;
}

void Game::SetEnableStatus(bool arg)
{
	enable_status = arg;
}

void Game::Render()
{	
	m_renderTimer.StartFrame();
	graphics_engine->BeginFrame();
	graphics_engine->DrawTiles1();
	for(int i = 0;i < m_rain.size();++i)
	{
		if(m_rain[i].m_currentSprite == -1)
		{
			m_rain[i].m_delay -= time;
			if(m_rain[i].m_delay <= 0)
			{
				m_rain[i].m_x = (i*3*16+(rand()%(3*16)));
				m_rain[i].m_currentSprite = rand()%3;
				m_rain[i].m_sprites[m_rain[i].m_currentSprite]->SetFrame(0);
				m_rain[i].m_sprites[m_rain[i].m_currentSprite]->SetAutoAnimate(true);
				m_rain[i].m_sprites[m_rain[i].m_currentSprite]->AutoStopAnimate();
				m_rain[i].m_sprites[m_rain[i].m_currentSprite]->SetPositionAbsalute(m_rain[i].m_x,m_rain[i].m_y);
				m_rain[i].m_sprites[m_rain[i].m_currentSprite]->Render();

			}
		}
		else
		{
			m_rain[i].m_sprites[m_rain[i].m_currentSprite]->SetPositionAbsalute(m_rain[i].m_x,m_rain[i].m_y);
			m_rain[i].m_sprites[m_rain[i].m_currentSprite]->Render();

			if(!m_rain[i].m_sprites[m_rain[i].m_currentSprite]->IsAnimating())
			{
				m_rain[i].m_currentSprite = -1;
				if(i < 5 || i >= 15)
					m_rain[i].m_delay = (rand()&100)/100.0f;
				else
					m_rain[i].m_delay = (rand()&100)*0.001f;
			}
		}
		
	}
	graphics_engine->DrawTiles2();
	
	player->setTime(time);
	player->Update();

	graphics_engine->EnableAlphaTest();
	level_class->Render();
	player->Render();

	int tempi = activeEnemys.size();
	for(int count = 0;count < tempi;count++)
	{
		activeEnemys[count]->Render();
	}
	list<Projectile*>::iterator temppl = projectile_list.begin();
	list<Projectile*>::iterator projEnd = projectile_list.end();
	while(temppl != projEnd)
	{
		(*temppl)->Render();
		++temppl;
	}
	
	if(m_overlay && m_overlayOn)
	{
		graphics_engine->DisableAlphaTest();
		m_overlay->Render();
		graphics_engine->EnableAlphaTest();
	}

//#if defined PERFORMANCE
	//graphics_engine->Position(100,50);
	//(*graphics_engine) << gothic10r << "FPS: " << static_cast<int>(frame_rate) << " Game " <<
	//	m_gameTime << " Render " << m_renderTime;
	//graphics_engine->Position(150,70);
	//(*graphics_engine) << gothic10r << "Video Memory left = " << (8*1024) - (graphics_engine->GetAvailableMemory()/1024) << "kb";
//#endif
	RenderHUD();
	transition->Update();

	if(!transition->sActive && current_level == 1)
		TutorialManager::Instance().Trigger(TutorialManager::TBASIC);
	
	graphics_engine->EndFrame();
	m_renderTimer.Update();
}

void Game::RenderHUD()
{
	if(player->GetStateFlags()->S_DEAD)
	{
		//PlayerDeath();
		ChangeState(game_death);
	}
	
	// Input Controls
	
	graphics_engine->DisableAlphaTest();

	attackButton->Render();
	specialAttackButton->Render();
	grenadeButton->Render();
	jumpButton->Render();
	pauseButton->Render();
	analogStick->Render();
	
	 weaponSelect->Update(time);
	weaponSelect->Render();
	
	healthMeterBar->SetTotalBars((float)player->GetNumLifeBars() * 3.0f);
	healthMeterBar->SetPercentFull((float)player->GetCurLife()/((float)player->GetNumLifeBars() *5.0f));
	healthMeterBar->Render();
	
	energyMeterBar->SetTotalBars((float)player->GetMaxEnergyLevel() * 3.0f/5.0f);
	energyMeterBar->SetPercentFull((float)player->GetEnergyLevel()/(float)player->GetMaxEnergyLevel());
	energyMeterBar->Render();
	
	bool boss_level = false;
	if(((current_level % 3) == 0)) boss_level = true;

	if(boss_level)
		activeEnemys[0]->RenderHealthBar();

	timeDisplay->SetTime(m_playTime);
	timeDisplay->Render();
	 
}

void Game::SetSpawnPoints()
{
	/*if(current_level <= 15) player->setSteph(false);
	else*/ //player->setSteph(true);
	
	if (resumeGame && !gameStateManager.GetGameStateInfo().inTheater)
	{
		//player->ipFlags = gameStateManager.GetGameStateInfo().playerInfo.ipFlags;
		player->SetSpawnPoint(gameStateManager.GetGameStateInfo().playerInfo.saveSpawnPoint.x, gameStateManager.GetGameStateInfo().playerInfo.saveSpawnPoint.y);
	}
	else
	{
		switch(current_level)
		{
		case 1:
				player->SetSpawnPoint(32*16,27*16);
				//player->SetSpawnPoint(35*16,9*16);
				//player->SetSpawnPoint(275*16,9*16);
			break;
		case 2:
			player->SetSpawnPoint(28*16,32*16);
			//player->SetSpawnPoint(59*16,32*16);
			//player->SetSpawnPoint(115*16,16*16);
			break;
		case 3:
			player->SetSpawnPoint(31*16,35*16);
			break;
		case 4:
			player->SetSpawnPoint(78*16,19*16);
	//		player->SetSpawnPoint(1250,2050);  //close to end of level
			break;
		case 5:
			player->SetSpawnPoint(60*16,28*16);
			break;
		case 6:
			player->SetSpawnPoint(41*16,31*16);
			break;	
		case 7:
			player->SetSpawnPoint(169*16,117*16);
		//	player->SetSpawnPoint(130*16,74*16); spawn point 2
			break;
		case 8:
			player->SetSpawnPoint(35*16,212*16);
			break;
		case 9:
			player->SetSpawnPoint(25*16,33*16);
			break;
		case 10: // TODO: Bonus Level 1
			player->SetSpawnPoint(50*16,67*16);
			break;
		case 11: // TODO: Bonus Level 2
			break;
		case 12: // TODO: Bonus Level 3
			break;
		default:
			player->SetSpawnPoint(5*16,10*16);

			break;
		};
	}
}

void Game::ReplayIntro()
{
	ChangeState(game_movie);
	RequestGState = game_menu;
	
	cut_scenes->StartMovie(0);

}

void Game::OnClickedLevelLoadScreen()
{
	m_levelLoadState=5;
}

void Game::OnClickedPauseButton()
{
	if (ActiveGState != game_ingame)
		return;

	attackButton->Disabled(true);
	specialAttackButton->Disabled(true);
	grenadeButton->Disabled(true);
	jumpButton->Disabled(true);
	analogStick->Disabled(true);
	weaponSelect->Disabled(true);
	
	resumeGameButton->Disabled(false);
	quitGameButton->Disabled(false);
//musicPlayer->Stop();
	ChangeState(game_paused);
}

void Game::OnClickedResumeGameButton()
{
	ChangeState(game_ingame);
	
	attackButton->Disabled(false);
	specialAttackButton->Disabled(false);
	grenadeButton->Disabled(false);
	jumpButton->Disabled(false);
	analogStick->Disabled(false);
	weaponSelect->Disabled(false);
	
	resumeGameButton->Disabled(true);
	quitGameButton->Disabled(true);
//musicPlayer->Play();
}

void Game::OnClickedQuitGameButton()
{
//musicPlayer->Stop();
	SaveGame();
	SaveGameState(true);
	SaveLevelState();
	
	// unload level
	list<Projectile*>::iterator temppl = projectile_list.begin();
	while(temppl != projectile_list.end())
	{
		delete (*temppl);
		temppl = projectile_list.erase(temppl);
	}
	Enemy* tenemy;
	for(int count = 0;count < enemys.size();count++)
	{
		tenemy = enemys[count];
		delete tenemy;
	}
	enemys.clear();
	level_class->DisableLevel();
	level_class->UnloadLevel();

	// change game state
	menu->RestartMenu(this, gameStateManager.GetGameStateInfo().canResume, &saved_games);
	ChangeState(game_menu);	
	
	attackButton->Disabled(false);
	specialAttackButton->Disabled(false);
	grenadeButton->Disabled(false);
	jumpButton->Disabled(false);
	analogStick->Disabled(false);
	weaponSelect->Disabled(false);
	
	resumeGameButton->Disabled(true);
	quitGameButton->Disabled(true);
}

void Game::UpdateHUD()
{	
	if (saved_games.GetSettingsInfo().analogFlip)
	{
		if(AppConfig::Instance().IsIpad())
			analogStick->SetPosition(422, 271-m_analogAdjust);
		else
			analogStick->SetPosition(405, 245-m_analogAdjust);
		
		if (saved_games.GetSettingsInfo().buttonFlip)
		{
			if(AppConfig::Instance().IsIpad())
				attackButton->SetPosition(70,263-m_attackButtonAdjust);
			else
			{
				attackButton->SetPosition(95, 243-m_attackButtonAdjust);
				attackButton->SetButtonBounds(88,236-m_attackButtonAdjust,82,82+m_attackButtonAdjust);
			}

			if(AppConfig::Instance().IsIpad())
				specialAttackButton->SetPosition(72, 240-m_attackButtonAdjust-m_specialAttackButtonAdjust);
			else
			{
				specialAttackButton->SetPosition(97, 200-m_attackButtonAdjust-m_specialAttackButtonAdjust);
				specialAttackButton->SetButtonBounds(88,153-m_attackButtonAdjust-m_specialAttackButtonAdjust,
					82,82+m_attackButtonAdjust+m_specialAttackButtonAdjust);
			}

			if(AppConfig::Instance().IsIpad())
				grenadeButton->SetPosition(14, 240-m_attackButtonAdjust-m_specialAttackButtonAdjust);
			else
			{
				grenadeButton->SetPosition(14, 200-m_attackButtonAdjust-m_specialAttackButtonAdjust);
				grenadeButton->SetButtonBounds(4,153-m_attackButtonAdjust-m_specialAttackButtonAdjust,
					82,82+m_attackButtonAdjust+m_specialAttackButtonAdjust);
			}

			if(AppConfig::Instance().IsIpad())
				jumpButton->SetPosition(12, 263-m_attackButtonAdjust);
			else
			{
				jumpButton->SetPosition(12, 243-m_attackButtonAdjust);
				jumpButton->SetButtonBounds(5,236-m_attackButtonAdjust,82,82+m_attackButtonAdjust);
			}
		}
		else
		{
			if(AppConfig::Instance().IsIpad())
				attackButton->SetPosition(12,263-m_attackButtonAdjust);
			else
			{
				attackButton->SetPosition(12, 243-m_attackButtonAdjust);
				attackButton->SetButtonBounds(5,236-m_attackButtonAdjust,82,82+m_attackButtonAdjust);
			}

			if(AppConfig::Instance().IsIpad())
				specialAttackButton->SetPosition(14, 240-m_attackButtonAdjust-m_specialAttackButtonAdjust);
			else
			{
				specialAttackButton->SetPosition(14, 200-m_attackButtonAdjust-m_specialAttackButtonAdjust);
				specialAttackButton->SetButtonBounds(4,153-m_attackButtonAdjust-m_specialAttackButtonAdjust,
					82,82+m_attackButtonAdjust+m_specialAttackButtonAdjust);
			}

			if(AppConfig::Instance().IsIpad())
				grenadeButton->SetPosition(72, 240-m_attackButtonAdjust-m_specialAttackButtonAdjust);
			else
			{
				grenadeButton->SetPosition(97, 200-m_attackButtonAdjust-m_specialAttackButtonAdjust);
				grenadeButton->SetButtonBounds(88,153-m_attackButtonAdjust-m_specialAttackButtonAdjust,
					82,82+m_attackButtonAdjust+m_specialAttackButtonAdjust);
			}

			if(AppConfig::Instance().IsIpad())
				jumpButton->SetPosition(70, 263-m_attackButtonAdjust);
			else
			{
				jumpButton->SetPosition(95, 243-m_attackButtonAdjust);
				jumpButton->SetButtonBounds(88,236-m_attackButtonAdjust,82,82+m_attackButtonAdjust);
			}
		}
	}
	else
	{
		if(AppConfig::Instance().IsIpad())
			analogStick->SetPosition(58, 271-m_analogAdjust);
		else
			analogStick->SetPosition(75, 245-m_analogAdjust);
		
		if (saved_games.GetSettingsInfo().buttonFlip)
		{
			if(AppConfig::Instance().IsIpad())
				attackButton->SetPosition(413,263-m_attackButtonAdjust);
			else
			{
				attackButton->SetPosition(400, 243-m_attackButtonAdjust);
				attackButton->SetButtonBounds(393,236-m_attackButtonAdjust,82,82+m_attackButtonAdjust);
			}

			if(AppConfig::Instance().IsIpad())
				specialAttackButton->SetPosition(415, 240-m_attackButtonAdjust-m_specialAttackButtonAdjust);
			else
			{
				specialAttackButton->SetPosition(403, 200-m_attackButtonAdjust-m_specialAttackButtonAdjust);
				specialAttackButton->SetButtonBounds(393,153-m_attackButtonAdjust-m_specialAttackButtonAdjust,
					82,82+m_attackButtonAdjust+m_specialAttackButtonAdjust);
			}

			if(AppConfig::Instance().IsIpad())
				grenadeButton->SetPosition(357, 240-m_attackButtonAdjust-m_specialAttackButtonAdjust);
			else
			{
				grenadeButton->SetPosition(319, 200-m_attackButtonAdjust-m_specialAttackButtonAdjust);
				grenadeButton->SetButtonBounds(310,153-m_attackButtonAdjust-m_specialAttackButtonAdjust,
					82,82+m_attackButtonAdjust+m_specialAttackButtonAdjust);
			}

			if(AppConfig::Instance().IsIpad())
				jumpButton->SetPosition(355, 263-m_attackButtonAdjust);
			else
			{
				jumpButton->SetPosition(317, 243-m_attackButtonAdjust);
				jumpButton->SetButtonBounds(310,236-m_attackButtonAdjust,82,82+m_attackButtonAdjust);
			}
		}
		else
		{
			if(AppConfig::Instance().IsIpad())
				attackButton->SetPosition(355,263-m_attackButtonAdjust);
			else
			{
				attackButton->SetPosition(317, 243-m_attackButtonAdjust);
				attackButton->SetButtonBounds(310,236-m_attackButtonAdjust,82,82+m_attackButtonAdjust);
			}

			if(AppConfig::Instance().IsIpad())
				specialAttackButton->SetPosition(357, 240-m_attackButtonAdjust-m_specialAttackButtonAdjust);
			else
			{
				specialAttackButton->SetPosition(319, 200-m_attackButtonAdjust-m_specialAttackButtonAdjust);
				specialAttackButton->SetButtonBounds(310,153-m_attackButtonAdjust-m_specialAttackButtonAdjust,
					82,82+m_attackButtonAdjust+m_specialAttackButtonAdjust);
			}

			if(AppConfig::Instance().IsIpad())
				grenadeButton->SetPosition(415, 240-m_attackButtonAdjust-m_specialAttackButtonAdjust);
			else
			{
				grenadeButton->SetPosition(403, 200);
				grenadeButton->SetButtonBounds(393,153-m_attackButtonAdjust-m_specialAttackButtonAdjust,
					82,82+m_attackButtonAdjust+m_specialAttackButtonAdjust);
			}

			if(AppConfig::Instance().IsIpad())
				jumpButton->SetPosition(413, 263-m_attackButtonAdjust);
			else
			{
				jumpButton->SetPosition(400, 243-m_attackButtonAdjust);
				jumpButton->SetButtonBounds(393,236-m_attackButtonAdjust,82,82+m_attackButtonAdjust);
			}
		}
	}
}

void Game::CaptureSpawnPointInfo()
{
	list<Object*> object_list = level_class->GetObjectList();
	
	m_SpawnPointInfo.numEnemies = enemys.size();
	m_SpawnPointInfo.enemyDeadStates = new bool[m_SpawnPointInfo.numEnemies];
	Enemy* tenemy;
	for(int count = 0;count < enemys.size();count++)
	{
		tenemy = enemys[count];
		if (tenemy->GetDead() || tenemy->GetCurLife() <= 0)
			m_SpawnPointInfo.enemyDeadStates[count] = true;
		else
			m_SpawnPointInfo.enemyDeadStates[count] = false;
	}
	
	m_SpawnPointInfo.numObjects = object_list.size();
	m_SpawnPointInfo.objectStates = new int [m_SpawnPointInfo.numObjects];
	int i = 0;
	for (list<Object*>::iterator tobject = object_list.begin(); tobject != object_list.end(); tobject++)
	{
		m_SpawnPointInfo.objectStates[i] = (*tobject)->GetState();
		i++;
	}
	
	//SaveLevelState();
}

bool Game::GotGoldAchievement()
{
	if(current_level == 11)
		return tallyNumBeacons >= 10;
	else
	{
		if (player->inBossBattle) // boss level
		{
			if (player->bossNoDamageTaken)
				return true;
			else
				return false;
		}
		else
		{
			if (tallyNumGotObjects >= tallyNumObjects)
				return true;
			else
				return false;
		}
	}
}

bool Game::GotSilverAchievement()
{	
	if(current_level == 11)
		return tallyNumBeacons >= 6;
	else
	{
		if (player->inBossBattle) // boss level
		{
			if (player->bossUsedOnlyBasicAttack)
				return true;
			else
				return false;
		}
		else
		{
			if (tallyNumDeadEnemies >= tallyNumEnemies)
				return true;
			else
				return false;
		}
	}
}

bool Game::GotBronzeAchievement()
{
	if(current_level == 11)
		return tallyNumBeacons >= 3;
	else
		return m_playTime <= levelAchievementTimes[current_level-2];
}

int Game::GetCompletionPercent()
{	
	float completionPercent = (((float)GetAchievementsCount())/30.0f) * 100.0f;
	
	return (int)completionPercent;
}

int Game::GetAchievementsCount()
{
	int totalAwards = 0;
	
	for (int i = 0; i < 12; i++)
	{		
		if (saved_games.GetSaveGameInfo().levelInfo[i].goldAchievement)
			totalAwards++;
		if (saved_games.GetSaveGameInfo().levelInfo[i].silverAchievement)
			totalAwards++;
		if (saved_games.GetSaveGameInfo().levelInfo[i].bronzeAchievement)
			totalAwards++;
	}
	
	return totalAwards;
}

void Game::SaveGame()
{
	SaveGameInfo temps;
	temps.energy = player->GetEnergyLevel();
	temps.energy_max = player->GetMaxEnergyLevel();
	temps.health = player->GetCurLife();
	
	// prevent player from being saved in a dead state
	if (temps.health <= 0)
		temps.health = 1;
	
	temps.health_bars = player->GetNumLifeBars();
	
	if (saved_games.GetSaveGameInfo().level < current_level
		&& current_level <= 9) // HACK: don't update this when on bonus levels!
		temps.level = current_level;
	else
		temps.level = saved_games.GetSaveGameInfo().level;
	
	//temps.score = player->GetScore();
	//temps.nlives = player->GetNumLives();
	//		temps.items = player->GetTreasureItems();
	temps.basic_weapon = player->GetBasicAttackDamage();
	
	temps.weapons[0] = player->IsWeaponAvailable(0);
	temps.weapons[1] = player->IsWeaponAvailable(1);
	temps.weapons[2] = player->IsWeaponAvailable(2);
	temps.weapons[3] = player->IsWeaponAvailable(3);
	temps.weapons[4] = player->IsWeaponAvailable(4);
	temps.weapons[5] = player->IsWeaponAvailable(5);
	temps.grenades = player->GetGrenades();
	
	for (int i = 0; i < 12; i++)
	{
		//temps.levelInfo[i] = player->levelAwards[i];
		temps.levelInfo[i].bronzeAchievement = player->levelAwards[i].bronzeAchievement;
		temps.levelInfo[i].silverAchievement = player->levelAwards[i].silverAchievement;
		temps.levelInfo[i].goldAchievement = player->levelAwards[i].goldAchievement;
	}
	
	// Bonuses
	temps.bonusInfo.BonusLevel1 = player->bonusAwards.BonusLevel1;
	temps.bonusInfo.BonusLevel2 = player->bonusAwards.BonusLevel2;
	temps.bonusInfo.BonusLevel3 = player->bonusAwards.BonusLevel3;

	//	GetLocalTime(&(temps.saved_time));
	saved_games.SaveGame(&temps);
	saved_games.SaveToDisk(DEFAULT_SAVE_FILE);
}

void Game::SaveGameState(bool canResume)
{
	// Save GameState information
	GameStateInfo gameStateInfo;
	gameStateInfo.canResume = canResume;
	gameStateInfo.level = current_level;
	
	if (ActiveGState == game_ingame || ActiveGState == game_paused || ActiveGState == game_death)
		gameStateInfo.inTheater = false;
	else
		gameStateInfo.inTheater = true;
	
	gameStateInfo.playTime = m_playTime;
	gameStateInfo.playerInfo.saveSpawnPoint.x = player->GetXLoc();
	gameStateInfo.playerInfo.saveSpawnPoint.y = player->GetYLoc();
	gameStateInfo.playerInfo.lastSpawnPoint.x = player->GetXSpawn();
	gameStateInfo.playerInfo.lastSpawnPoint.y = player->GetYSpawn();
	//gameStateInfo.playerInfo.ipFlags = player->ipFlags;
	
	gameStateInfo.playerInfo.S_ON_GROUND = player->ipFlags.S_ON_GROUND;
	gameStateInfo.playerInfo.S_NEXT_TO_LADDER = player->ipFlags.S_NEXT_TO_LADDER;
	gameStateInfo.playerInfo.S_ON_LADDER = player->ipFlags.S_ON_LADDER;
	gameStateInfo.playerInfo.S_LADDER_TYPE = player->ipFlags.S_LADDER_TYPE;
	gameStateInfo.playerInfo.S_CROUCH = player->ipFlags.S_CROUCH;
	gameStateInfo.playerInfo.S_CROUCH_TOGGLE = player->ipFlags.S_CROUCH_TOGGLE;
	gameStateInfo.playerInfo.S_CRAWL = player->ipFlags.S_CRAWL;
	gameStateInfo.playerInfo.bossUsedOnlyBasicAttack = player->bossUsedOnlyBasicAttack;
	gameStateInfo.playerInfo.bossNoDamageTaken = player->bossNoDamageTaken;
	
	gameStateInfo.playerInfo.frameSet = player->player_spr->GetFrameSet();
	gameStateInfo.numEnemies = enemys.size();
	gameStateInfo.enemyStates = new GameStateEnemyInfo[enemys.size()];
	Enemy* tenemy;
	for(int count = 0;count < enemys.size();count++)
	{
		tenemy = enemys[count];
		//gameStateInfo.enemyDead.push_back(tenemy->GetDead());
		gameStateInfo.enemyStates[count].isDead = tenemy->GetDead();
		gameStateInfo.enemyStates[count].life = tenemy->GetCurLife();
	}
	
	list<Object*> object_list = level_class->GetObjectList();
	gameStateInfo.numObjects = object_list.size();
	gameStateInfo.objectStates = new int[gameStateInfo.numObjects];
	
	int i = 0;
	for (list<Object*>::iterator tobject = object_list.begin(); tobject != object_list.end(); tobject++)
	{
		gameStateInfo.objectStates[i] = (*tobject)->GetState();
		i++;
	}
	
	gameStateInfo.spawnPointInfo = this->m_SpawnPointInfo;
	
	gameStateManager.SaveGameState(gameStateInfo);
	gameStateManager.SaveToDisk(DEFAULT_GAMESTATE_FILE);
}

void Game::SaveLevelState()
{	
	list<Object*> object_list = level_class->GetObjectList();
	vector<bool> hasHealthUpgrade;
	vector<bool> hasEnergyUpgrade;
	vector<bool> hasAttackUpgrade;
	vector<bool> hasWeaponUpgrade;
	
	for (list<Object*>::iterator tobject = object_list.begin(); tobject != object_list.end(); tobject++)
	{
		if ((*tobject)->GetUpgradeType() == LEVELSTATETYPE_NOTUPGRADE)
			continue;
		// Attempt to set health states
		else if ((*tobject)->GetUpgradeType() == LEVELSTATETYPE_HEALTH)
		{
			hasHealthUpgrade.push_back((*tobject)->GetUpgradeState());
			continue;
		}
		// Attempt to set energy states
		else if ((*tobject)->GetUpgradeType() == LEVELSTATETYPE_ENERGY)
		{
			hasEnergyUpgrade.push_back((*tobject)->GetUpgradeState());
			continue;
		}
		// Attempt to set attack upgrade states
		else if ((*tobject)->GetUpgradeType() == LEVELSTATETYPE_ATTACK)
		{
			hasAttackUpgrade.push_back((*tobject)->GetUpgradeState());
			continue;
		}
		// Attempt to set weapon upgrade states
		else if ((*tobject)->GetUpgradeType() == LEVELSTATETYPE_WEAPON)
		{
			hasWeaponUpgrade.push_back((*tobject)->GetUpgradeState());
			continue;
		}
	}	

	LevelStateInfo levelStateInfo;
	levelStateInfo.level = current_level;
	levelStateInfo.numHealthUpgrades = hasHealthUpgrade.size();
	levelStateInfo.hasHealthUpgrade = new bool[hasHealthUpgrade.size()];
	levelStateInfo.numEnergyUpgrades = hasEnergyUpgrade.size();
	levelStateInfo.hasEnergyUpgrade = new bool[hasEnergyUpgrade.size()];
	levelStateInfo.numAttackUpgrades = hasAttackUpgrade.size();
	levelStateInfo.hasAttackUpgrade = new bool[hasAttackUpgrade.size()];
	levelStateInfo.numWeaponUpgrades = hasWeaponUpgrade.size();
	levelStateInfo.hasWeaponUpgrade = new bool[hasWeaponUpgrade.size()];
	
	for (int i = 0; i < hasHealthUpgrade.size(); i++)
		levelStateInfo.hasHealthUpgrade[i] = hasHealthUpgrade[i];

	for (int i = 0; i < hasEnergyUpgrade.size(); i++)
		levelStateInfo.hasEnergyUpgrade[i] = hasEnergyUpgrade[i];
	
	for (int i = 0; i < hasAttackUpgrade.size(); i++)
		levelStateInfo.hasAttackUpgrade[i] = hasAttackUpgrade[i];
	
	for (int i = 0; i < hasWeaponUpgrade.size(); i++)
		levelStateInfo.hasWeaponUpgrade[i] = hasWeaponUpgrade[i];
	
	levelStateManager.SaveLevelState(levelStateInfo);
	levelStateManager.SaveToDisk(DEFAULT_LEVELSTATE_FILE);
}

void Game::ApplicationTerminated()
{
	//if (ActiveGState == game_ingame || ActiveGState == game_movie)
	if (ActiveGState != game_menu && RequestGState != game_menu)
	{
		SaveGame();
		SaveGameState(true);
		
		if (ActiveGState != game_movie && ActiveGState != game_level_load && ActiveGState != game_level_end && ActiveGState != game_death)
			SaveLevelState();
	}
}
